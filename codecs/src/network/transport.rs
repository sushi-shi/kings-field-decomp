//! Connection ownership and bounded events shared by native and browser clients.
use super::KF_NET_PROTOCOL_VERSION;
use serde::{Deserialize, Serialize};
use serde_json::json;
use std::collections::VecDeque;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};

#[cfg(target_os = "emscripten")]
mod browser;
#[cfg(not(target_os = "emscripten"))]
mod native;
#[cfg(target_os = "emscripten")]
pub use browser::event as browser_event;
#[cfg(target_os = "emscripten")]
pub use browser::Transport;
#[cfg(not(target_os = "emscripten"))]
pub use native::Transport;

pub const PACKET_LIMIT: usize = 60000;
pub const SIGNAL_LIMIT: usize = 65536;
const QUEUE_EVENTS: usize = 128;
const QUEUE_BYTES: usize = 2 * 1024 * 1024;
pub const ROOM: u8 = 0;
pub const CONNECTED: u8 = 1;
pub const DISCONNECTED: u8 = 2;
pub const PACKET: u8 = 3;
pub const HOST_WAITING: u8 = 4;
pub const ENDED: u8 = 5;
pub const ERROR: u8 = 6;

pub fn identity(text: &str) -> Option<[u8; 32]> {
    if text.len() != 64
        || !text
            .bytes()
            .all(|b| b.is_ascii_digit() || (b'a'..=b'f').contains(&b))
    {
        return None;
    }
    let mut bytes = [0; 32];
    for (byte, pair) in bytes.iter_mut().zip(text.as_bytes().chunks_exact(2)) {
        let digit = |b: u8| if b <= b'9' { b - b'0' } else { b - b'a' + 10 };
        *byte = digit(pair[0]) * 16 + digit(pair[1]);
    }
    Some(bytes)
}
fn identity_text(bytes: &[u8; 32]) -> String {
    bytes.iter().map(|byte| format!("{byte:02x}")).collect()
}

#[derive(Clone)]
pub struct Config {
    pub address: String,
    pub room: String,
    pub resources: String,
    pub recipe: String,
    pub resume: String,
    pub credential: String,
    pub roster: [[u8; 32]; 4],
    pub host: bool,
}
impl Config {
    pub fn valid(&self) -> bool {
        identity(&self.credential).is_some_and(|id| id != [0; 32])
            && self.address.len() <= 2048
            && (self.address.starts_with("ws://") || self.address.starts_with("wss://"))
            && [&self.room, &self.resources, &self.recipe, &self.resume]
                .iter()
                .all(|s| s.len() <= 128)
    }
    pub fn request(&self) -> String {
        json!({"type": if !self.resume.is_empty() { "resume" } else if self.host { "create" } else { "join" },
            "protocol": KF_NET_PROTOCOL_VERSION, "resources": self.resources, "recipe": self.recipe,
            "room": self.room, "resume": self.resume, "credential": self.credential,
            "roster": self.roster.iter().map(identity_text).collect::<Vec<_>>()}).to_string()
    }
    pub fn remote(&self, peer: u8) -> bool {
        peer < 4 && if self.host { peer != 0 } else { peer == 0 }
    }
}

pub struct Event {
    pub kind: u8,
    pub peer: u8,
    pub lane: u8,
    pub data: Vec<u8>,
    pub generation: u32,
    pub identity: [u8; 32],
}
impl Event {
    fn control(kind: u8, peer: u8, text: &str) -> Self {
        Self {
            kind,
            peer,
            lane: 0,
            data: text.as_bytes().to_vec(),
            generation: 0,
            identity: [0; 32],
        }
    }
}
#[derive(Default)]
struct Queue {
    events: VecDeque<Event>,
    bytes: usize,
}
#[derive(Default)]
struct PeerState {
    generation: u32,
    ready: u8,
    closed: bool,
}
pub struct Shared {
    queue: Mutex<Queue>,
    pub failed: AtomicBool,
    pub stopped: AtomicBool,
    peers: [Mutex<PeerState>; 4],
    pub resume: Mutex<String>,
    identities: [Mutex<[u8; 32]>; 4],
}
impl Shared {
    fn new() -> Arc<Self> {
        Arc::new(Self {
            queue: Mutex::new(Queue::default()),
            failed: AtomicBool::new(false),
            stopped: AtomicBool::new(false),
            peers: std::array::from_fn(|_| Mutex::new(PeerState::default())),
            resume: Mutex::new(String::new()),
            identities: std::array::from_fn(|_| Mutex::new([0; 32])),
        })
    }
    pub fn current(&self, peer: u8, generation: u32) -> bool {
        !self.stopped.load(Ordering::Acquire) && peer < 4 && self.generation(peer) == generation
    }
    pub fn live(&self, peer: u8, generation: u32) -> bool {
        if peer > 3
            || generation == 0
            || self.stopped.load(Ordering::Acquire)
            || self.failed.load(Ordering::Acquire)
        {
            return false;
        }
        let state = self.peers[usize::from(peer)].lock().unwrap();
        state.generation == generation && !state.closed
    }
    fn generation(&self, peer: u8) -> u32 {
        self.peers[usize::from(peer)].lock().unwrap().generation
    }
    pub fn begin_peer(&self, peer: u8) -> u32 {
        let mut queue = self.queue.lock().unwrap();
        let mut state = self.peers[usize::from(peer)].lock().unwrap();
        *state = PeerState {
            generation: state.generation + 1,
            ..Default::default()
        };
        let generation = state.generation;
        drop(state);
        queue
            .events
            .retain(|event| event.peer != peer || event.generation == 0);
        queue.bytes = queue.events.iter().map(|event| event.data.len()).sum();
        generation
    }
    pub fn connected(&self, peer: u8, lane: u8, generation: u32) {
        if peer > 3 || lane > 1 {
            return;
        }
        // Readiness and generation change together; a stale callback cannot
        // mark a replacement connection ready between separate atomic updates.
        let mut state = self.peers[usize::from(peer)].lock().unwrap();
        if state.generation != generation || state.closed {
            return;
        }
        let old = state.ready;
        state.ready |= 1 << lane;
        drop(state);
        if old != 3 && old | (1 << lane) == 3 {
            self.push(Event {
                generation,
                identity: *self.identities[usize::from(peer)].lock().unwrap(),
                ..Event::control(CONNECTED, peer, "")
            });
        }
    }
    pub fn disconnected(&self, peer: u8, generation: u32) {
        if self.close_peer(peer, generation) {
            self.push(Event {
                generation,
                ..Event::control(DISCONNECTED, peer, "")
            });
        }
    }
    fn close_peer(&self, peer: u8, generation: u32) -> bool {
        if peer > 3 || generation == 0 {
            return false;
        }
        let mut state = self.peers[usize::from(peer)].lock().unwrap();
        if state.generation != generation || state.closed {
            return false;
        }
        state.ready = 0;
        state.closed = true;
        true
    }
    pub fn reject_peer(&self, peer: u8, generation: u32, reason: &str) {
        // Discard this connection's pending packets and readiness notification.
        // Preserve other peers and the room's admission/reconnect credentials.
        let mut queue = self.queue.lock().unwrap();
        if !self.close_peer(peer, generation) {
            return;
        }
        queue
            .events
            .retain(|event| event.peer != peer || event.generation != generation);
        queue.bytes = queue.events.iter().map(|event| event.data.len()).sum();
        drop(queue);
        let reason: String = reason.chars().take(256).collect();
        self.push(Event {
            generation,
            ..Event::control(DISCONNECTED, peer, &reason)
        });
    }
    #[cfg(any(test, target_os = "emscripten"))]
    pub fn can_send(&self, peer: u8, lane: u8, size: usize) -> bool {
        self.send_generation(peer, lane, size).is_some()
    }
    fn send_generation(&self, peer: u8, lane: u8, size: usize) -> Option<u32> {
        if peer > 3
            || lane > 1
            || size == 0
            || size > PACKET_LIMIT
            || self.failed.load(Ordering::Acquire)
            || self.stopped.load(Ordering::Acquire)
        {
            return None;
        }
        let state = self.peers[usize::from(peer)].lock().unwrap();
        if state.ready == 3 && !state.closed {
            Some(state.generation)
        } else {
            None
        }
    }
    pub fn fail(&self, reason: &str) {
        let mut queue = self.queue.lock().unwrap();
        if self.stopped.load(Ordering::Acquire) || self.failed.swap(true, Ordering::AcqRel) {
            return;
        }
        queue.events.clear();
        let reason: String = reason.chars().take(256).collect();
        let event = Event::control(ERROR, 0, &reason);
        queue.bytes = event.data.len();
        queue.events.push_back(event);
    }
    fn end(&self, reason: &str) {
        let mut queue = self.queue.lock().unwrap();
        if self.stopped.swap(true, Ordering::AcqRel) {
            return;
        }
        // Socket/channel close callbacks can already be queued when the room
        // ends. Preserve the terminal event instead of initiating a reconnect.
        queue.events.clear();
        let event = Event::control(ENDED, 0, reason);
        queue.bytes = event.data.len();
        queue.events.push_back(event);
    }
    pub fn push(&self, event: Event) {
        if self.failed.load(Ordering::Acquire) || self.stopped.load(Ordering::Acquire) {
            return;
        }
        if event.kind == PACKET && !self.live(event.peer, event.generation) {
            return;
        }
        if event.data.len() > PACKET_LIMIT || event.peer > 3 || event.lane > 1 {
            self.fail("Invalid network event");
            return;
        }
        let mut queue = self.queue.lock().unwrap();
        if self.failed.load(Ordering::Acquire) || self.stopped.load(Ordering::Acquire) {
            return;
        }
        if event.generation != 0 && !self.current(event.peer, event.generation) {
            return;
        }
        if matches!(event.kind, PACKET | CONNECTED) && !self.live(event.peer, event.generation) {
            return;
        }
        if event.kind == PACKET && event.lane == 1 {
            if let Some(index) = queue
                .events
                .iter()
                .position(|old| old.kind == PACKET && old.lane == 1 && old.peer == event.peer)
            {
                let old = queue.events.remove(index).unwrap();
                queue.bytes -= old.data.len();
            }
        }
        // State has one coalesced slot per peer. Reliable packets have independent
        // allowances so a flooding guest cannot spend another guest's budget.
        if event.kind == PACKET && event.lane == 0 {
            let (count, bytes) = queue
                .events
                .iter()
                .filter(|old| old.kind == PACKET && old.lane == 0 && old.peer == event.peer)
                .fold((0, 0), |(count, bytes), old| {
                    (count + 1, bytes + old.data.len())
                });
            if count >= QUEUE_EVENTS || bytes + event.data.len() > QUEUE_BYTES {
                drop(queue);
                self.reject_peer(
                    event.peer,
                    event.generation,
                    "Peer packet queue exceeded its limit",
                );
                return;
            }
        } else if event.generation == 0 {
            let (count, bytes) = queue
                .events
                .iter()
                .filter(|old| old.generation == 0)
                .fold((0, 0), |(count, bytes), old| {
                    (count + 1, bytes + old.data.len())
                });
            if count >= QUEUE_EVENTS || bytes + event.data.len() > QUEUE_BYTES {
                drop(queue);
                self.fail("Room event queue exceeded its limit");
                return;
            }
        }
        queue.bytes += event.data.len();
        queue.events.push_back(event);
    }
    pub fn poll(&self) -> Option<Event> {
        let mut queue = self.queue.lock().unwrap();
        while let Some(event) = queue.events.pop_front() {
            queue.bytes -= event.data.len();
            if event.generation == 0 || self.current(event.peer, event.generation) {
                return Some(event);
            }
        }
        None
    }
}

#[derive(Clone, Deserialize, Serialize)]
pub struct IceServer {
    pub urls: String,
    #[serde(default)]
    pub username: String,
    #[serde(default)]
    pub credential: String,
}
fn valid_ice(servers: &[IceServer]) -> bool {
    servers.len() <= 8
        && servers.iter().all(|s| {
            s.urls.len() <= 2048
                && s.username.len() <= 256
                && s.credential.len() <= 256
                && ["stun:", "stuns:", "turn:", "turns:"]
                    .iter()
                    .any(|prefix| s.urls.starts_with(prefix))
        })
}
#[derive(Deserialize)]
#[serde(tag = "type")]
enum RoomMessage {
    #[serde(rename = "created", alias = "joined")]
    Admitted {
        slot: u8,
        room: String,
        resume: String,
        identity: String,
        #[serde(rename = "iceServers")]
        ice: Vec<IceServer>,
    },
    #[serde(rename = "peer")]
    Peer {
        slot: u8,
        identity: String,
        #[serde(rename = "iceServers")]
        ice: Vec<IceServer>,
    },
    #[serde(rename = "peer-left")]
    Left { slot: u8 },
    #[serde(rename = "signal")]
    Signal {
        from: u8,
        #[serde(flatten)]
        signal: Signal,
    },
    #[serde(rename = "host-ready")]
    Ready {
        #[serde(rename = "iceServers")]
        ice: Vec<IceServer>,
    },
    #[serde(rename = "host-wait")]
    Waiting,
    #[serde(rename = "ended")]
    Ended,
    #[serde(rename = "error")]
    Error {
        reason: String,
        #[serde(default)]
        retryable: bool,
    },
}
#[derive(Clone, Deserialize, Serialize)]
#[serde(untagged)]
pub enum Signal {
    Description {
        sdp: String,
        #[serde(rename = "descriptionType")]
        kind: String,
    },
    Candidate {
        candidate: String,
        mid: String,
    },
}
impl Signal {
    pub fn valid(&self) -> bool {
        match self {
            Self::Description { sdp, kind } => {
                sdp.len() <= 60000 && matches!(kind.as_str(), "offer" | "answer")
            }
            Self::Candidate { candidate, mid } => candidate.len() <= 4096 && mid.len() <= 128,
        }
    }
    pub fn message(&self, peer: u8) -> String {
        let mut value = serde_json::to_value(self).unwrap();
        value["type"] = json!("signal");
        value["to"] = json!(peer);
        value.to_string()
    }
}
pub enum Action {
    None,
    HostReady,
    Peer(u8),
    Left(u8),
    Signal(u8, Signal),
    End,
}
pub struct Session {
    pub config: Config,
    pub ice: Vec<IceServer>,
    admitted: bool,
}
impl Session {
    pub fn new(config: Config) -> Self {
        Self {
            config,
            ice: Vec::new(),
            admitted: false,
        }
    }
    pub fn receive(&mut self, bytes: &[u8], shared: &Shared) -> Result<Action, String> {
        if bytes.len() > SIGNAL_LIMIT {
            return Err("Oversized signaling message".into());
        }
        let message: RoomMessage =
            serde_json::from_slice(bytes).map_err(|_| "Invalid signaling message")?;
        match message {
            RoomMessage::Admitted {
                slot,
                room,
                resume,
                identity: public_id,
                ice,
            } => {
                let public_id = identity(&public_id)
                    .filter(|id| *id != [0; 32])
                    .ok_or("Invalid player identity")?;
                if self.admitted
                    || slot > 3
                    || (slot == 0) != self.config.host
                    || room.is_empty()
                    || room.len() > 128
                    || resume.is_empty()
                    || resume.len() > 128
                    || !valid_ice(&ice)
                {
                    return Err("Invalid room admission".into());
                }
                self.admitted = true;
                self.ice = ice;
                *shared.resume.lock().unwrap() = resume;
                shared.push(Event {
                    identity: public_id,
                    ..Event::control(ROOM, slot, &room)
                });
                Ok(Action::None)
            }
            RoomMessage::Peer {
                slot,
                identity: public_id,
                ice,
            } if self.admitted
                && self.config.host
                && self.config.remote(slot)
                && valid_ice(&ice) =>
            {
                let public_id = identity(&public_id)
                    .filter(|id| *id != [0; 32])
                    .ok_or("Invalid peer identity")?;
                // TURN credentials from admission may have expired. Both
                // backends read this configuration when creating the new peer.
                self.ice = ice;
                *shared.identities[usize::from(slot)].lock().unwrap() = public_id;
                Ok(Action::Peer(slot))
            }
            RoomMessage::Left { slot }
                if self.admitted && self.config.host && self.config.remote(slot) =>
            {
                Ok(Action::Left(slot))
            }
            RoomMessage::Signal { from, signal }
                if self.admitted && self.config.remote(from) && signal.valid() =>
            {
                Ok(Action::Signal(from, signal))
            }
            RoomMessage::Ready { ice } if self.admitted && !self.config.host && valid_ice(&ice) => {
                self.ice = ice;
                Ok(Action::HostReady)
            }
            RoomMessage::Waiting if self.admitted && !self.config.host => {
                shared.push(Event::control(HOST_WAITING, 0, ""));
                Ok(Action::Left(0))
            }
            RoomMessage::Ended if self.admitted => {
                shared.end("");
                Ok(Action::End)
            }
            RoomMessage::Error { reason, retryable }
                if !reason.is_empty() && reason.len() <= 256 =>
            {
                if retryable {
                    Err(reason)
                } else {
                    shared.end(&reason);
                    Ok(Action::End)
                }
            }
            _ => Err("Unexpected room service response".into()),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    fn config(host: bool) -> Config {
        Config {
            address: "ws://127.0.0.1:1234".into(),
            room: "invite".into(),
            resources: "resources".into(),
            recipe: "avatars".into(),
            resume: String::new(),
            credential: "01".repeat(32),
            roster: [[0; 32]; 4],
            host,
        }
    }
    fn admit(session: &mut Session, shared: &Shared) {
        let slot = if session.config.host { 0 } else { 1 };
        session.receive(json!({"type":"joined", "slot":slot, "room":"invite", "resume":"secret", "identity":"01".repeat(32), "iceServers":[]}).to_string().as_bytes(), shared).unwrap();
    }
    #[test]
    fn authenticated_peer_identity_crosses_only_the_typed_boundary() {
        let shared = Shared::new();
        let mut session = Session::new(config(true));
        admit(&mut session, &shared);
        assert_eq!(shared.poll().unwrap().identity, [1; 32]);
        for id in [
            "x".repeat(64),
            "0".repeat(64),
            "é".repeat(32),
            "a".repeat(66),
        ] {
            let message =
                json!({"type":"peer", "slot":1, "identity":id, "iceServers":[]}).to_string();
            assert!(session.receive(message.as_bytes(), &shared).is_err());
        }
        let message = json!({"type":"peer", "slot":1, "identity":"ab".repeat(32), "iceServers":[]})
            .to_string();
        assert!(matches!(
            session.receive(message.as_bytes(), &shared),
            Ok(Action::Peer(1))
        ));
        let generation = shared.begin_peer(1);
        shared.connected(1, 0, generation);
        shared.connected(1, 1, generation);
        assert_eq!(shared.poll().unwrap().identity, [0xab; 32]);
        let mut missing = config(false);
        missing.credential.clear();
        assert!(!missing.valid());
    }
    #[test]
    fn negotiation_refreshes_ice_only_after_role_and_bounds_validation() {
        for host in [true, false] {
            let shared = Shared::new();
            let mut session = Session::new(config(host));
            let fresh = json!([{"urls":"turn:relay.example:3478", "username":"fresh", "credential":"secret"}]);
            let message = |ice: serde_json::Value| {
                if host {
                    json!({"type":"peer", "slot":1, "identity":"ab".repeat(32), "iceServers":ice})
                } else {
                    json!({"type":"host-ready", "iceServers":ice})
                }
            };
            assert!(session
                .receive(message(fresh.clone()).to_string().as_bytes(), &shared)
                .is_err());
            assert!(session.ice.is_empty());
            admit(&mut session, &shared);
            session.ice.push(IceServer {
                urls: "turn:relay.example:3478".into(),
                username: "expired".into(),
                credential: "old".into(),
            });
            let action = session
                .receive(message(fresh.clone()).to_string().as_bytes(), &shared)
                .unwrap();
            assert!(matches!(
                (host, action),
                (true, Action::Peer(1)) | (false, Action::HostReady)
            ));
            assert_eq!(serde_json::to_value(&session.ice).unwrap(), fresh);
            for invalid in [
                json!([{"urls":"https://relay.example"}]),
                json!([{"urls":format!("turn:{}", "x".repeat(2048))}]),
                json!([{"urls":"turn:relay", "username":"x".repeat(257)}]),
                json!([{"urls":"turn:relay", "credential":"x".repeat(257)}]),
                json!(vec![fresh[0].clone(); 9]),
                json!(null),
            ] {
                assert!(session
                    .receive(message(invalid).to_string().as_bytes(), &shared)
                    .is_err());
                assert_eq!(serde_json::to_value(&session.ice).unwrap(), fresh);
            }
            let wrong_role = if host {
                json!({"type":"host-ready", "iceServers":[]})
            } else {
                json!({"type":"peer", "slot":1, "identity":"ab".repeat(32), "iceServers":[]})
            };
            assert!(session
                .receive(wrong_role.to_string().as_bytes(), &shared)
                .is_err());
            assert_eq!(serde_json::to_value(&session.ice).unwrap(), fresh);
            if host {
                let invalid_identity =
                    json!({"type":"peer", "slot":1, "identity":"0".repeat(32), "iceServers":[]});
                assert!(session
                    .receive(invalid_identity.to_string().as_bytes(), &shared)
                    .is_err());
                assert_eq!(*shared.identities[1].lock().unwrap(), [0xab; 32]);
                assert_eq!(serde_json::to_value(&session.ice).unwrap(), fresh);
            }
        }
    }
    #[test]
    fn signaling_requires_admission_and_correct_peer_role() {
        let shared = Shared::new();
        let mut session = Session::new(config(false));
        let offer =
            json!({"type":"signal", "from":0, "sdp":"description", "descriptionType":"offer"})
                .to_string();
        assert!(session.receive(offer.as_bytes(), &shared).is_err());
        admit(&mut session, &shared);
        assert!(matches!(
            session.receive(offer.as_bytes(), &shared),
            Ok(Action::Signal(0, _))
        ));
        assert!(session
            .receive(br#"{"type":"peer","slot":2}"#, &shared)
            .is_err());
        assert!(session
            .receive(
                br#"{"type":"signal","from":2,"candidate":"x","mid":"0"}"#,
                &shared
            )
            .is_err());
        assert!(session
            .receive(
                br#"{"type":"signal","from":256,"candidate":"x","mid":"0"}"#,
                &shared
            )
            .is_err());
        assert!(session
            .receive(&vec![b' '; SIGNAL_LIMIT + 1], &shared)
            .is_err());
        assert_eq!(*shared.resume.lock().unwrap(), "secret");
    }
    #[test]
    fn room_end_survives_socket_and_channel_close_callbacks() {
        let shared = Shared::new();
        let mut session = Session::new(config(false));
        admit(&mut session, &shared);
        let generation = shared.begin_peer(0);
        shared.connected(0, 0, generation);
        shared.connected(0, 1, generation);
        shared.disconnected(0, generation);
        assert!(matches!(
            session.receive(br#"{"type":"ended"}"#, &shared),
            Ok(Action::End)
        ));
        shared.fail("Room service disconnected");
        shared.push(Event::control(DISCONNECTED, 0, ""));
        assert!(!shared.can_send(0, 0, 10));
        assert_eq!(shared.poll().unwrap().kind, ENDED);
        assert!(shared.poll().is_none());
    }
    #[test]
    fn permanent_room_rejections_end_with_their_reason_but_temporary_errors_can_retry() {
        for admitted in [false, true] {
            let shared = Shared::new();
            let mut settings = config(false);
            settings.resume = "existing-reconnect-token".into();
            let mut session = Session::new(settings);
            if admitted {
                admit(&mut session, &shared);
            }
            assert!(matches!(
                session.receive(br#"{"type":"error","reason":"Room unavailable"}"#, &shared),
                Ok(Action::End)
            ));
            shared.fail("Room service disconnected");
            shared.push(Event::control(DISCONNECTED, 0, ""));
            let ended = shared.poll().unwrap();
            assert_eq!(ended.kind, ENDED);
            assert_eq!(ended.data, b"Room unavailable");
            assert!(shared.poll().is_none());
            assert!(!shared.failed.load(Ordering::Acquire));
        }
        let shared = Shared::new();
        let mut session = Session::new(config(false));
        assert!(
            matches!(session.receive(br#"{"type":"error","reason":"Host reconnecting","retryable":true}"#, &shared), Err(reason) if reason == "Host reconnecting")
        );
        assert!(!shared.stopped.load(Ordering::Acquire));
        for message in [
            json!({"type":"error","reason":"x".repeat(257)}),
            json!({"type":"error","reason":""}),
            json!({"type":"error","reason":"No room","retryable":"yes"}),
        ] {
            assert!(session
                .receive(message.to_string().as_bytes(), &shared)
                .is_err());
            assert!(!shared.stopped.load(Ordering::Acquire));
        }
    }
    #[test]
    fn stale_callbacks_cannot_reopen_replacement_peer() {
        let shared = Shared::new();
        let old = shared.begin_peer(1);
        shared.connected(1, 0, old);
        assert!(shared.poll().is_none());
        shared.connected(1, 1, old);
        let current = shared.begin_peer(1);
        assert!(shared.poll().is_none());
        shared.connected(1, 0, old);
        shared.connected(1, 1, old);
        assert!(!shared.can_send(1, 0, 10));
        shared.connected(1, 0, current);
        shared.connected(1, 1, current);
        assert_eq!(shared.poll().unwrap().kind, CONNECTED);
        shared.disconnected(1, old);
        assert!(shared.can_send(1, 0, 10));
        assert!(shared.poll().is_none());
        shared.disconnected(1, current);
        shared.connected(1, 0, current);
        shared.connected(1, 1, current);
        assert!(!shared.can_send(1, 0, 10));
        assert_eq!(shared.poll().unwrap().kind, DISCONNECTED);
        assert!(shared.poll().is_none());
    }
    #[test]
    fn ordinary_disconnect_preserves_packets_received_before_close() {
        let shared = Shared::new();
        let generation = shared.begin_peer(1);
        shared.connected(1, 0, generation);
        shared.connected(1, 1, generation);
        shared.push(Event {
            kind: PACKET,
            peer: 1,
            lane: 0,
            generation,
            identity: [0; 32],
            data: vec![42],
        });
        shared.disconnected(1, generation);
        shared.push(Event {
            kind: PACKET,
            peer: 1,
            lane: 0,
            generation,
            identity: [0; 32],
            data: vec![99],
        });
        assert_eq!(shared.poll().unwrap().kind, CONNECTED);
        assert_eq!(shared.poll().unwrap().data, vec![42]);
        assert_eq!(shared.poll().unwrap().kind, DISCONNECTED);
        assert!(shared.poll().is_none());
    }
    #[test]
    fn rejected_peer_cannot_poison_other_peers_or_reopen_with_late_callbacks() {
        let shared = Shared::new();
        *shared.resume.lock().unwrap() = "room-resume".into();
        let bad = shared.begin_peer(1);
        let good = shared.begin_peer(2);
        for (slot, generation) in [(1, bad), (2, good)] {
            shared.connected(slot, 0, generation);
            shared.connected(slot, 1, generation);
            shared.push(Event {
                kind: PACKET,
                peer: slot,
                lane: 0,
                generation,
                identity: [0; 32],
                data: vec![slot],
            });
        }
        shared.reject_peer(1, bad, "Invalid WebRTC signal");
        shared.connected(1, 0, bad);
        shared.connected(1, 1, bad);
        shared.push(Event {
            kind: PACKET,
            peer: 1,
            lane: 0,
            generation: bad,
            identity: [0; 32],
            data: vec![99],
        });
        assert!(!shared.failed.load(Ordering::Acquire));
        assert!(!shared.live(1, bad));
        assert!(shared.can_send(2, 0, 1));
        assert_eq!(shared.poll().unwrap().peer, 2);
        assert_eq!(shared.poll().unwrap().data, vec![2]);
        let rejected = shared.poll().unwrap();
        assert_eq!((rejected.kind, rejected.peer), (DISCONNECTED, 1));
        assert_eq!(rejected.data, b"Invalid WebRTC signal");
        assert!(shared.poll().is_none());
        assert_eq!(shared.queue.lock().unwrap().bytes, 0);
        assert_eq!(*shared.resume.lock().unwrap(), "room-resume");
        let replacement = shared.begin_peer(1);
        shared.connected(1, 0, replacement);
        shared.connected(1, 1, replacement);
        shared.reject_peer(1, bad, "Late rejection");
        assert!(shared.can_send(1, 0, 1));
        assert_eq!(shared.poll().unwrap().kind, CONNECTED);
        assert!(shared.poll().is_none());
    }
    #[test]
    fn state_coalesces_but_reliable_byte_overflow_rejects_only_its_peer() {
        let shared = Shared::new();
        let generation = shared.begin_peer(1);
        for value in 0..200 {
            shared.push(Event {
                kind: PACKET,
                identity: [0; 32],
                peer: 1,
                lane: 1,
                generation,
                data: vec![value],
            });
        }
        assert_eq!(shared.poll().unwrap().data, vec![199]);
        assert!(shared.poll().is_none());
        let healthy = shared.begin_peer(2);
        for _ in 0..34 {
            shared.push(Event {
                kind: PACKET,
                peer: 2,
                lane: 0,
                generation: healthy,
                identity: [0; 32],
                data: vec![2; PACKET_LIMIT],
            });
        }
        for _ in 0..40 {
            shared.push(Event {
                kind: PACKET,
                identity: [0; 32],
                peer: 1,
                lane: 0,
                generation,
                data: vec![0; PACKET_LIMIT],
            });
        }
        assert!(!shared.failed.load(Ordering::Acquire));
        assert!(shared.live(2, healthy));
        for _ in 0..34 {
            let event = shared.poll().unwrap();
            assert_eq!((event.kind, event.peer), (PACKET, 2));
            assert_eq!(event.data.len(), PACKET_LIMIT);
        }
        let event = shared.poll().unwrap();
        assert_eq!((event.kind, event.peer), (DISCONNECTED, 1));
        assert!(shared.poll().is_none());
    }
    #[test]
    fn each_peer_has_a_reliable_allowance_and_state_cannot_spend_it() {
        let shared = Shared::new();
        let generations = [shared.begin_peer(1), shared.begin_peer(2)];
        for (index, generation) in generations.iter().enumerate() {
            let peer = index as u8 + 1;
            shared.connected(peer, 0, *generation);
            shared.connected(peer, 1, *generation);
        }
        while shared.poll().is_some() {}
        for value in 0..QUEUE_EVENTS {
            for (index, generation) in generations.iter().enumerate() {
                shared.push(Event {
                    kind: PACKET,
                    peer: index as u8 + 1,
                    lane: 0,
                    generation: *generation,
                    identity: [0; 32],
                    data: vec![value as u8],
                });
            }
        }
        for value in 0..200 {
            shared.push(Event {
                kind: PACKET,
                peer: 2,
                lane: 1,
                generation: generations[1],
                identity: [0; 32],
                data: vec![value],
            });
        }
        assert!(shared.can_send(1, 0, 1) && shared.can_send(2, 0, 1));
        shared.push(Event::control(ROOM, 0, "same-room"));
        shared.push(Event {
            kind: PACKET,
            peer: 1,
            lane: 0,
            generation: generations[0],
            identity: [0; 32],
            data: vec![255],
        });
        assert!(!shared.failed.load(Ordering::Acquire));
        assert!(!shared.live(1, generations[0]));
        assert!(shared.can_send(2, 0, 1));
        for value in 0..QUEUE_EVENTS {
            let event = shared.poll().unwrap();
            assert_eq!((event.kind, event.peer, event.lane), (PACKET, 2, 0));
            assert_eq!(event.data, vec![value as u8]);
        }
        assert_eq!(shared.poll().unwrap().data, vec![199]);
        assert_eq!(shared.poll().unwrap().kind, ROOM);
        assert_eq!(shared.poll().unwrap().kind, DISCONNECTED);
        assert!(shared.poll().is_none());
        assert_eq!(shared.queue.lock().unwrap().bytes, 0);
        let replacement = shared.begin_peer(1);
        shared.push(Event {
            kind: PACKET,
            peer: 1,
            lane: 0,
            generation: replacement,
            identity: [0; 32],
            data: vec![42],
        });
        assert_eq!(shared.poll().unwrap().data, vec![42]);
        for _ in 0..QUEUE_EVENTS {
            shared.push(Event {
                kind: PACKET,
                peer: 1,
                lane: 0,
                generation: replacement,
                identity: [0; 32],
                data: vec![7],
            });
        }
        let newest = shared.begin_peer(1);
        shared.push(Event {
            kind: PACKET,
            peer: 1,
            lane: 0,
            generation: newest,
            identity: [0; 32],
            data: vec![8],
        });
        shared.reject_peer(1, replacement, "Late overflow");
        assert!(shared.live(1, newest));
        assert_eq!(shared.poll().unwrap().data, vec![8]);
        assert!(shared.poll().is_none());
    }
}
