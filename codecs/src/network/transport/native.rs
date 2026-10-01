use super::*;
use bytes::BytesMut;
use futures_util::{SinkExt, StreamExt};
use std::future::Future;
use std::thread::JoinHandle;
use std::time::Duration;
use tokio::sync::{mpsc, oneshot};
use tokio_tungstenite::tungstenite::{protocol::WebSocketConfig, Message};
use webrtc::data_channel::{DataChannel, DataChannelEvent, RTCDataChannelInit};
use webrtc::peer_connection::{
    PeerConnection, PeerConnectionBuilder, PeerConnectionEventHandler, RTCConfigurationBuilder,
    RTCIceCandidateInit, RTCIceServer, RTCPeerConnectionIceEvent, RTCPeerConnectionState,
    RTCSessionDescription, SettingEngineBuilder,
};

type Result<T> = std::result::Result<T, String>;
fn failure(error: impl std::fmt::Display) -> String {
    error.to_string()
}
struct Outbound {
    peer: u8,
    lane: u8,
    generation: u32,
    bytes: Vec<u8>,
}
// Each channel drains independently: SCTP backpressure from one guest must not
// hold the signaling loop or the other guests' data channels.
async fn send_packets<F, Fut>(shared: &Shared, mut queue: mpsc::Receiver<Outbound>, mut send: F)
where
    F: FnMut(u8, Vec<u8>) -> Fut,
    Fut: Future<Output = Result<()>>,
{
    while let Some(packet) = queue.recv().await {
        if shared.send_generation(packet.peer, packet.lane, packet.bytes.len())
            != Some(packet.generation)
        {
            break;
        }
        let timeout = if packet.lane == 0 {
            Duration::from_secs(2)
        } else {
            Duration::from_millis(100)
        };
        let result = tokio::time::timeout(timeout, send(packet.lane, packet.bytes)).await;
        if packet.lane == 0 && !matches!(result, Ok(Ok(()))) {
            shared.disconnected(packet.peer, packet.generation);
            break;
        }
    }
}
enum Internal {
    Signal(u8, u32, Signal),
    PeerClosed,
}
fn reject_peer(
    shared: &Shared,
    events: &mpsc::Sender<Internal>,
    slot: u8,
    generation: u32,
    reason: &str,
) {
    if reason.is_empty() {
        shared.disconnected(slot, generation);
    } else {
        shared.reject_peer(slot, generation, reason);
    }
    // If the queue is full, draining an existing event also runs peer cleanup.
    let _ = events.try_send(Internal::PeerClosed);
}
struct Handler {
    peer: u8,
    generation: u32,
    shared: Arc<Shared>,
    events: mpsc::Sender<Internal>,
}
#[async_trait::async_trait]
impl PeerConnectionEventHandler for Handler {
    async fn on_ice_candidate(&self, event: RTCPeerConnectionIceEvent) {
        if !self.shared.live(self.peer, self.generation) {
            return;
        }
        let candidate = match event.candidate.to_json() {
            Ok(value) => value,
            Err(_) => {
                reject_peer(
                    &self.shared,
                    &self.events,
                    self.peer,
                    self.generation,
                    "Cannot encode ICE candidate",
                );
                return;
            }
        };
        if self
            .events
            .try_send(Internal::Signal(
                self.peer,
                self.generation,
                Signal::Candidate {
                    candidate: candidate.candidate,
                    mid: candidate.sdp_mid.unwrap_or_else(|| "0".into()),
                },
            ))
            .is_err()
        {
            reject_peer(
                &self.shared,
                &self.events,
                self.peer,
                self.generation,
                "Signaling queue exceeded its limit",
            );
        }
    }
    async fn on_connection_state_change(&self, state: RTCPeerConnectionState) {
        if matches!(
            state,
            RTCPeerConnectionState::Disconnected
                | RTCPeerConnectionState::Failed
                | RTCPeerConnectionState::Closed
        ) {
            reject_peer(&self.shared, &self.events, self.peer, self.generation, "");
        }
    }
    async fn on_data_channel(&self, _channel: Arc<dyn DataChannel>) {
        if !self.shared.live(self.peer, self.generation) {
            return;
        }
        reject_peer(
            &self.shared,
            &self.events,
            self.peer,
            self.generation,
            "Unexpected data channel",
        );
    }
}
struct Peer {
    connection: Arc<dyn PeerConnection>,
    channels: [Option<Arc<dyn DataChannel>>; 2],
    outgoing: [Option<mpsc::Sender<Outbound>>; 2],
    tasks: Vec<tokio::task::JoinHandle<()>>,
    generation: u32,
    described: bool,
    candidates: Vec<RTCIceCandidateInit>,
    candidate_count: u8,
}
impl Drop for Peer {
    fn drop(&mut self) {
        for task in &self.tasks {
            task.abort();
        }
    }
}
fn retire(peer: Peer) {
    let connection = peer.connection.clone();
    drop(peer);
    // A peer's close handshake must not hold the shared signaling loop.
    tokio::spawn(async move {
        let _ = tokio::time::timeout(Duration::from_secs(1), connection.close()).await;
    });
}

async fn attach(
    peer: &mut Peer,
    slot: u8,
    channel: Arc<dyn DataChannel>,
    shared: &Arc<Shared>,
    events: &mpsc::Sender<Internal>,
) -> Result<()> {
    let lane = match channel.label().await.map_err(failure)?.as_str() {
        "actions" => 0,
        "state" => 1,
        _ => return Err("Unexpected data channel".into()),
    };
    if peer.channels[lane].is_some()
        || channel.ordered().await.map_err(failure)? != (lane == 0)
        || channel.max_retransmits().await.map_err(failure)?
            != if lane == 0 { None } else { Some(0) }
        || channel
            .max_packet_life_time()
            .await
            .map_err(failure)?
            .is_some()
        || !channel.negotiated().await.map_err(failure)?
        || !channel.protocol().await.map_err(failure)?.is_empty()
    {
        return Err("Invalid data channel reliability".into());
    }
    peer.channels[lane] = Some(channel.clone());
    let (sender, receiver) = mpsc::channel(if lane == 0 { 32 } else { 2 });
    peer.outgoing[lane] = Some(sender);
    let output = channel.clone();
    let connection = peer.connection.clone();
    let state = shared.clone();
    let generation = peer.generation;
    peer.tasks.push(tokio::spawn(async move {
        send_packets(&state, receiver, move |lane, bytes| {
            let output = output.clone();
            async move {
                let bytes = BytesMut::from(bytes.as_slice());
                if lane == 0 {
                    output.send(bytes).await.map_err(failure)
                } else {
                    output.try_send(bytes).await.map_err(failure)
                }
            }
        })
        .await;
        if state.current(slot, generation) {
            let _ = tokio::time::timeout(Duration::from_secs(1), connection.close()).await;
        }
    }));
    let shared = shared.clone();
    let events = events.clone();
    peer.tasks.push(tokio::spawn(async move {
        while let Some(event) = channel.poll().await {
            if !shared.live(slot, generation) {
                break;
            }
            match event {
                DataChannelEvent::OnOpen => shared.connected(slot, lane as u8, generation),
                DataChannelEvent::OnClose | DataChannelEvent::OnError => {
                    reject_peer(&shared, &events, slot, generation, "");
                    break;
                }
                DataChannelEvent::OnMessage(message) => {
                    if message.is_string
                        || message.data.is_empty()
                        || message.data.len() > PACKET_LIMIT
                    {
                        reject_peer(
                            &shared,
                            &events,
                            slot,
                            generation,
                            "Invalid data channel packet",
                        );
                        break;
                    }
                    shared.push(Event {
                        kind: PACKET,
                        identity: [0; 32],
                        peer: slot,
                        lane: lane as u8,
                        generation,
                        data: message.data.to_vec(),
                    });
                    if !shared.live(slot, generation) {
                        let _ = events.try_send(Internal::PeerClosed);
                        break;
                    }
                }
                _ => {}
            }
        }
    }));
    Ok(())
}

async fn create_peer(
    session: &Session,
    shared: &Arc<Shared>,
    events: &mpsc::Sender<Internal>,
    slot: u8,
) -> Result<Peer> {
    let generation = shared.begin_peer(slot);
    let ice = session
        .ice
        .iter()
        .map(|s| RTCIceServer {
            urls: vec![s.urls.clone()],
            username: s.username.clone(),
            credential: s.credential.clone(),
        })
        .collect();
    let connection: Arc<dyn PeerConnection> = Arc::new(
        PeerConnectionBuilder::new()
            .with_configuration(RTCConfigurationBuilder::new().with_ice_servers(ice).build())
            .with_setting_engine(
                SettingEngineBuilder::default()
                    .with_sctp_max_receive_buffer_size(1024 * 1024)
                    .build(),
            )
            .with_handler(Arc::new(Handler {
                peer: slot,
                generation,
                shared: shared.clone(),
                events: events.clone(),
            }))
            .with_dedicated_reactor_pool_size(0)
            .with_data_channel_send_buffer_limit(1024 * 1024)
            .with_udp_addrs(vec!["0.0.0.0:0"])
            .build()
            .await
            .map_err(failure)?,
    );
    let mut peer = Peer {
        connection,
        channels: [None, None],
        outgoing: [None, None],
        tasks: Vec::new(),
        generation,
        described: false,
        candidates: Vec::new(),
        candidate_count: 0,
    };
    for (lane, label) in ["actions", "state"].iter().enumerate() {
        let channel = peer
            .connection
            .create_data_channel(
                label,
                Some(RTCDataChannelInit {
                    ordered: lane == 0,
                    negotiated: Some(lane as u16),
                    max_retransmits: if lane == 0 { None } else { Some(0) },
                    ..Default::default()
                }),
            )
            .await
            .map_err(failure)?;
        attach(&mut peer, slot, channel, shared, events).await?;
    }
    Ok(peer)
}

async fn description(peer: &mut Peer, signal: Signal, host: bool) -> Result<Option<Signal>> {
    match signal {
        Signal::Description { sdp, kind } => {
            if peer.described || (kind == "answer") != host {
                return Err("Unexpected WebRTC description".into());
            }
            let remote = if host {
                RTCSessionDescription::answer(sdp)
            } else {
                RTCSessionDescription::offer(sdp)
            }
            .map_err(failure)?;
            peer.connection
                .set_remote_description(remote)
                .await
                .map_err(failure)?;
            peer.described = true;
            for candidate in peer.candidates.drain(..) {
                peer.connection
                    .add_ice_candidate(candidate)
                    .await
                    .map_err(failure)?;
            }
            if !host {
                let answer = peer.connection.create_answer(None).await.map_err(failure)?;
                let signal = Signal::Description {
                    sdp: answer.sdp.clone(),
                    kind: "answer".into(),
                };
                peer.connection
                    .set_local_description(answer)
                    .await
                    .map_err(failure)?;
                return Ok(Some(signal));
            }
        }
        Signal::Candidate { candidate, mid } => {
            if peer.candidate_count >= 64 {
                return Err("Too many ICE candidates".into());
            }
            peer.candidate_count += 1;
            let candidate = RTCIceCandidateInit {
                candidate,
                sdp_mid: Some(mid),
                ..Default::default()
            };
            if peer.described {
                peer.connection
                    .add_ice_candidate(candidate)
                    .await
                    .map_err(failure)?;
            } else if peer.candidates.len() < 64 {
                peer.candidates.push(candidate);
            } else {
                return Err("Too many ICE candidates".into());
            }
        }
    }
    Ok(None)
}

async fn run(
    config: Config,
    shared: Arc<Shared>,
    mut outgoing: mpsc::Receiver<Outbound>,
    stop: oneshot::Receiver<()>,
) {
    let mut peers: [Option<Peer>; 4] = std::array::from_fn(|_| None);
    let work = async {
        let socket_config = WebSocketConfig::default()
            .max_message_size(Some(SIGNAL_LIMIT))
            .max_frame_size(Some(SIGNAL_LIMIT));
        let (mut socket, _) = tokio::time::timeout(
            Duration::from_secs(10),
            tokio_tungstenite::connect_async_with_config(
                &config.address,
                Some(socket_config),
                true,
            ),
        )
        .await
        .map_err(failure)?
        .map_err(failure)?;
        socket
            .send(Message::Text(config.request().into()))
            .await
            .map_err(failure)?;
        let mut session = Session::new(config);
        let (events, mut incoming) = mpsc::channel(64);
        loop {
            tokio::select! {
                message = socket.next() => {
                    let message = message.ok_or("Room service disconnected")?.map_err(failure)?;
                    let action = match message {
                        Message::Text(text) => session.receive(text.as_bytes(), &shared)?,
                        Message::Ping(_) | Message::Pong(_) => continue,
                        Message::Close(_) => return Err("Room service disconnected".into()),
                        _ => return Err("Invalid signaling packet".into()),
                    };
                    match action {
                        Action::None => {},
                        Action::HostReady => {
                            shared.begin_peer(0);
                            if let Some(old) = peers[0].take() { retire(old); }
                        }
                        Action::End => return Ok::<(), String>(()),
                        Action::Left(slot) => {
                            let generation = shared.generation(slot);
                            shared.disconnected(slot, generation);
                            if let Some(peer) = peers[usize::from(slot)].take() { retire(peer); }
                        }
                        Action::Peer(slot) => {
                            if let Some(old) = peers[usize::from(slot)].take() {
                                shared.begin_peer(slot);
                                retire(old);
                            }
                            let peer = create_peer(&session, &shared, &events, slot).await?;
                            let offer = peer.connection.create_offer(None).await.map_err(failure)?;
                            let signal = Signal::Description { sdp: offer.sdp.clone(), kind: "offer".into() };
                            peer.connection.set_local_description(offer).await.map_err(failure)?;
                            peers[usize::from(slot)] = Some(peer);
                            socket.send(Message::Text(signal.message(slot).into())).await.map_err(failure)?;
                        }
                        Action::Signal(slot, signal) => {
                            if peers[usize::from(slot)].is_none() {
                                if session.config.host { continue; }
                                let generation = shared.generation(slot);
                                if generation != 0 && !shared.live(slot, generation) { continue; }
                                peers[usize::from(slot)] = Some(create_peer(&session, &shared, &events, slot).await?);
                            }
                            let peer = peers[usize::from(slot)].as_mut().unwrap();
                            if shared.live(slot, peer.generation) {
                                match description(peer, signal, session.config.host).await {
                                    Ok(Some(response)) => socket.send(Message::Text(response.message(slot).into())).await.map_err(failure)?,
                                    Ok(None) => {},
                                    Err(reason) => shared.reject_peer(slot, peer.generation, &reason),
                                }
                            }
                        }
                    }
                }
                Some(event) = incoming.recv() => match event {
                    Internal::Signal(slot, generation, signal) => {
                        if shared.live(slot, generation) {
                            socket.send(Message::Text(signal.message(slot).into())).await.map_err(failure)?;
                        }
                    }
                    Internal::PeerClosed => {},
                },
                Some(packet) = outgoing.recv() => {
                    if shared.send_generation(packet.peer, packet.lane, packet.bytes.len()) != Some(packet.generation) { continue; }
                    let Some(peer) = &peers[usize::from(packet.peer)] else { continue; };
                    let Some(sender) = &peer.outgoing[usize::from(packet.lane)] else { continue; };
                    let (slot, generation, lane) = (packet.peer, packet.generation, packet.lane);
                    if sender.try_send(packet).is_err() && lane == 0 {
                        // An accepted reliable packet cannot be silently discarded.
                        // Close this peer; its sender task closes the connection.
                        shared.disconnected(slot, generation);
                    }
                }
            }
            for (slot, peer) in peers.iter_mut().enumerate() {
                if peer
                    .as_ref()
                    .is_some_and(|peer| !shared.live(slot as u8, peer.generation))
                {
                    retire(peer.take().unwrap());
                }
            }
            if shared.failed.load(Ordering::Acquire) {
                return Ok(());
            }
        }
    };
    tokio::select! {
        result = work => { if let Err(reason) = result { shared.fail(&reason); } },
        _ = stop => {},
    }
    for peer in peers.iter_mut().filter_map(Option::take) {
        let _ = tokio::time::timeout(Duration::from_secs(1), peer.connection.close()).await;
    }
}

pub struct Transport {
    pub shared: Arc<Shared>,
    config: Config,
    outgoing: mpsc::Sender<Outbound>,
    stop: Option<oneshot::Sender<()>>,
    thread: Option<JoinHandle<()>>,
}
impl Transport {
    pub fn open(config: Config) -> Option<Self> {
        if !config.valid() {
            return None;
        }
        let runtime = tokio::runtime::Builder::new_current_thread()
            .enable_all()
            .build()
            .ok()?;
        let shared = Shared::new();
        let (outgoing, receiver) = mpsc::channel(32);
        let (stop, stopped) = oneshot::channel();
        let state = shared.clone();
        let settings = config.clone();
        let thread = std::thread::Builder::new()
            .name("kf-network".into())
            .spawn(move || {
                runtime.block_on(run(settings, state, receiver, stopped));
            })
            .ok()?;
        Some(Self {
            shared,
            config,
            outgoing,
            stop: Some(stop),
            thread: Some(thread),
        })
    }
    pub fn poll(&mut self) -> Option<Event> {
        self.shared.poll()
    }
    pub fn send(&mut self, peer: u8, lane: u8, bytes: &[u8]) -> bool {
        if !self.config.remote(peer) {
            return false;
        }
        let Some(generation) = self.shared.send_generation(peer, lane, bytes.len()) else {
            return false;
        };
        self.outgoing
            .try_send(Outbound {
                peer,
                lane,
                generation,
                bytes: bytes.to_vec(),
            })
            .is_ok()
    }
}
impl Drop for Transport {
    fn drop(&mut self) {
        self.shared.stopped.store(true, Ordering::Release);
        if let Some(stop) = self.stop.take() {
            let _ = stop.send(());
        }
        if let Some(thread) = self.thread.take() {
            let _ = thread.join();
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn ready(shared: &Shared, slot: u8) -> u32 {
        let generation = shared.begin_peer(slot);
        shared.connected(slot, 0, generation);
        shared.connected(slot, 1, generation);
        generation
    }
    fn packet(peer: u8, lane: u8, generation: u32, marker: u8) -> Outbound {
        Outbound {
            peer,
            lane,
            generation,
            bytes: vec![marker],
        }
    }

    #[tokio::test]
    async fn blocked_actions_do_not_stall_other_peers_or_state() {
        let shared = Shared::new();
        let slow = ready(&shared, 1);
        let fast = ready(&shared, 2);
        while shared.poll().is_some() {}
        let (slow_tx, slow_rx) = mpsc::channel(2);
        let (fast_tx, fast_rx) = mpsc::channel(2);
        let (state_tx, state_rx) = mpsc::channel(2);
        let (started, blocked) = oneshot::channel();
        let state = shared.clone();
        let slow_task = tokio::spawn(async move {
            let mut started = Some(started);
            send_packets(&state, slow_rx, move |_, _| {
                started.take().unwrap().send(()).unwrap();
                std::future::pending::<Result<()>>()
            })
            .await;
        });
        slow_tx.send(packet(1, 0, slow, 1)).await.unwrap();
        blocked.await.unwrap();
        let (delivered, mut received) = mpsc::channel(4);
        let output = delivered.clone();
        let state = shared.clone();
        let fast_task = tokio::spawn(async move {
            send_packets(&state, fast_rx, move |lane, bytes| {
                let output = output.clone();
                async move {
                    output.send((lane, bytes[0])).await.unwrap();
                    Ok(())
                }
            })
            .await;
        });
        let state = shared.clone();
        let state_task = tokio::spawn(async move {
            send_packets(&state, state_rx, move |lane, bytes| {
                let output = delivered.clone();
                async move {
                    if bytes[0] == 20 {
                        return Err("State buffer full".into());
                    }
                    output.send((lane, bytes[0])).await.unwrap();
                    Ok(())
                }
            })
            .await;
        });
        fast_tx.send(packet(2, 0, fast, 10)).await.unwrap();
        fast_tx.send(packet(2, 0, fast, 11)).await.unwrap();
        state_tx.send(packet(1, 1, slow, 20)).await.unwrap();
        state_tx.send(packet(1, 1, slow, 21)).await.unwrap();
        let delivered = tokio::time::timeout(Duration::from_secs(1), async {
            let mut messages = Vec::new();
            for _ in 0..3 {
                messages.push(received.recv().await.unwrap());
            }
            messages
        })
        .await
        .expect("A blocked reliable send held another channel");
        assert_eq!(
            delivered
                .iter()
                .filter(|p| p.0 == 0)
                .map(|p| p.1)
                .collect::<Vec<_>>(),
            [10, 11]
        );
        assert!(
            delivered.contains(&(1, 21)),
            "Unreliable send failure stopped later state"
        );
        assert!(!slow_task.is_finished());
        assert!(shared.can_send(1, 0, 1));
        drop(fast_tx);
        drop(state_tx);
        fast_task.await.unwrap();
        state_task.await.unwrap();
        tokio::time::timeout(Duration::from_secs(3), slow_task)
            .await
            .unwrap()
            .unwrap();
        assert!(!shared.failed.load(Ordering::Acquire));
        assert!(!shared.can_send(1, 0, 1));
        assert!(shared.can_send(2, 0, 1));
        let event = shared.poll().unwrap();
        assert_eq!((event.kind, event.peer), (DISCONNECTED, 1));
        assert!(shared.poll().is_none());
    }

    #[tokio::test]
    async fn queued_and_in_flight_sends_cannot_disconnect_a_replacement_peer() {
        let shared = Shared::new();
        let generation = ready(&shared, 1);
        let (sender, receiver) = mpsc::channel(2);
        sender.send(packet(1, 0, generation, 1)).await.unwrap();
        ready(&shared, 1);
        send_packets(&shared, receiver, |_, _| async {
            panic!("Stale queued bytes reached the replacement connection");
        })
        .await;
        assert!(shared.can_send(1, 0, 1));

        let generation = shared.generation(1);
        let (sender, receiver) = mpsc::channel(2);
        let (started, blocked) = oneshot::channel();
        let (release, wait) = oneshot::channel();
        let state = shared.clone();
        let task = tokio::spawn(async move {
            let mut started = Some(started);
            let mut wait = Some(wait);
            send_packets(&state, receiver, move |_, _| {
                started.take().unwrap().send(()).unwrap();
                let wait = wait.take().unwrap();
                async move {
                    wait.await.unwrap();
                    Err("Old channel closed".into())
                }
            })
            .await;
        });
        sender.send(packet(1, 0, generation, 2)).await.unwrap();
        blocked.await.unwrap();
        ready(&shared, 1);
        while shared.poll().is_some() {}
        release.send(()).unwrap();
        task.await.unwrap();
        assert!(shared.can_send(1, 0, 1));
        assert!(!shared.failed.load(Ordering::Acquire));
        assert!(shared.poll().is_none());
    }
}
