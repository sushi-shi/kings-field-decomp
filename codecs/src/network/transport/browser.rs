use super::*;
use crate::ffi::transport::browser as bridge;
use std::collections::BTreeMap;
use std::sync::atomic::AtomicU32;
use std::sync::{LazyLock, Weak};

const REJECT_QUEUE: u8 = 9; // Internal marker; never accepted from JavaScript.

struct Raw {
    owner: Option<u8>,
    announcement: bool,
    kind: u8,
    peer: u8,
    generation: u32,
    data: Vec<u8>,
}
#[derive(Default)]
struct Pending {
    events: VecDeque<Raw>,
    reset: [bool; 4],
}
struct Inbox {
    shared: Arc<Shared>,
    events: Mutex<Pending>,
}
static SESSIONS: LazyLock<Mutex<BTreeMap<u32, Weak<Inbox>>>> =
    LazyLock::new(|| Mutex::new(BTreeMap::new()));
static NEXT_SESSION: AtomicU32 = AtomicU32::new(1);

pub fn event(handle: u32, kind: u8, peer: u8, lane: u8, generation: u32, data: &[u8]) {
    let Some(inbox) = SESSIONS
        .lock()
        .unwrap()
        .get(&handle)
        .and_then(Weak::upgrade)
    else {
        return;
    };
    if generation != 0 && !inbox.shared.live(peer, generation) {
        return;
    }
    match kind {
        4 => inbox.shared.connected(peer, lane, generation),
        5 => inbox.shared.disconnected(peer, generation),
        6 => {
            if data.is_empty() || data.len() > PACKET_LIMIT {
                inbox
                    .shared
                    .reject_peer(peer, generation, "Invalid data channel packet");
                return;
            }
            inbox.shared.push(Event {
                kind: PACKET,
                identity: [0; 32],
                peer,
                lane,
                generation,
                data: data.to_vec(),
            });
        }
        0..=3 | 7..=8 => {
            // The service authenticates `from`. Decode routing in Rust before
            // buffering so forwarded peer signals cannot spend the room budget.
            // Session::receive still validates admission, roles and contents.
            let (owner, reset) = if kind == 1 {
                match serde_json::from_slice::<RoomMessage>(data) {
                    Ok(RoomMessage::Signal { from, .. }) if from < 4 => (Some(from), None),
                    Ok(RoomMessage::Peer { slot, .. }) if slot < 4 => (Some(slot), Some(slot)),
                    Ok(RoomMessage::Left { slot }) if slot < 4 => (Some(slot), None),
                    Ok(RoomMessage::Ready { .. }) => (Some(0), Some(0)),
                    _ => (None, None),
                }
            } else {
                (if generation != 0 { Some(peer) } else { None }, None)
            };
            let mut pending = inbox.events.lock().unwrap();
            if let Some(slot) = reset {
                pending.events.retain(|raw| raw.owner != Some(slot));
                pending.reset[usize::from(slot)] = true;
            }
            if generation != 0 && pending.reset[usize::from(peer)] {
                return; // Callbacks from the peer being replaced.
            }
            if owner.is_some()
                && pending
                    .events
                    .iter()
                    .any(|raw| raw.kind == REJECT_QUEUE && raw.owner == owner)
            {
                return;
            }
            let (count, bytes) = pending
                .events
                .iter()
                .filter(|raw| raw.owner == owner)
                .fold((0, 0), |(count, bytes), raw| {
                    (count + 1, bytes + raw.data.len())
                });
            if count >= QUEUE_EVENTS || bytes + data.len() > QUEUE_BYTES {
                if let Some(slot) = owner {
                    pending
                        .events
                        .retain(|raw| raw.owner != owner || raw.announcement);
                    // A peer announcement may still precede this in the queue.
                    // Reject after it is processed, using its new generation.
                    let generation = if pending.reset[usize::from(slot)] {
                        0
                    } else {
                        generation
                    };
                    pending.events.push_back(Raw {
                        owner,
                        announcement: false,
                        kind: REJECT_QUEUE,
                        peer: slot,
                        generation,
                        data: Vec::new(),
                    });
                } else {
                    inbox.shared.fail("Room signaling queue exceeded its limit");
                }
                return;
            }
            pending.events.push_back(Raw {
                owner,
                announcement: reset.is_some(),
                kind,
                peer,
                generation,
                data: data.to_vec(),
            });
        }
        _ => inbox.shared.fail("Invalid browser network event"),
    }
}

#[derive(Default)]
struct Peer {
    exists: bool,
    described: bool,
    describing: bool,
    candidate_count: u8,
    candidates: Vec<Signal>,
}
pub struct Transport {
    pub shared: Arc<Shared>,
    handle: u32,
    inbox: Arc<Inbox>,
    session: Session,
    peers: [Peer; 4],
}
impl Transport {
    pub fn open(config: Config) -> Option<Self> {
        if !config.valid() {
            return None;
        }
        let handle = NEXT_SESSION
            .fetch_update(Ordering::AcqRel, Ordering::Acquire, |id| id.checked_add(1))
            .ok()?;
        let shared = Shared::new();
        let inbox = Arc::new(Inbox {
            shared: shared.clone(),
            events: Mutex::new(Pending::default()),
        });
        SESSIONS
            .lock()
            .unwrap()
            .insert(handle, Arc::downgrade(&inbox));
        if !bridge::open(handle, &config.address) {
            SESSIONS.lock().unwrap().remove(&handle);
            return None;
        }
        Some(Self {
            shared,
            handle,
            inbox,
            session: Session::new(config),
            peers: std::array::from_fn(|_| Peer::default()),
        })
    }
    fn create_peer(&mut self, slot: u8) -> Result<(), String> {
        let generation = self.shared.begin_peer(slot);
        self.inbox.events.lock().unwrap().reset[usize::from(slot)] = false;
        self.peers[usize::from(slot)] = Peer {
            exists: true,
            ..Default::default()
        };
        let ice =
            serde_json::to_string(&self.session.ice).map_err(|_| "Invalid ICE configuration")?;
        if !bridge::peer(
            self.handle,
            slot,
            generation,
            self.session.config.host,
            &ice,
        ) {
            return Err("Cannot create WebRTC peer".into());
        }
        Ok(())
    }
    fn description(&mut self, slot: u8, signal: Signal) -> Result<(), String> {
        if !self.peers[usize::from(slot)].exists {
            if self.session.config.host {
                return Ok(());
            }
            self.create_peer(slot)?;
        }
        let peer = &mut self.peers[usize::from(slot)];
        match &signal {
            Signal::Description { kind, .. } => {
                if peer.described
                    || peer.describing
                    || (kind == "answer") != self.session.config.host
                {
                    return Err("Unexpected WebRTC description".into());
                }
                peer.describing = true;
            }
            Signal::Candidate { .. } => {
                if peer.candidate_count >= 64 {
                    return Err("Too many ICE candidates".into());
                }
                peer.candidate_count += 1;
                if !peer.described {
                    peer.candidates.push(signal);
                    return Ok(());
                }
            }
        }
        if !bridge::description(self.handle, slot, &serde_json::to_string(&signal).unwrap()) {
            return Err("Invalid WebRTC signal".into());
        }
        Ok(())
    }
    fn process(&mut self, raw: Raw) -> Result<(), String> {
        if raw.generation != 0 && !self.shared.live(raw.peer, raw.generation) {
            return Ok(());
        }
        match raw.kind {
            0 => {
                if !bridge::signal(self.handle, &self.session.config.request()) {
                    return Err("Cannot send room request".into());
                }
            }
            1 => match self.session.receive(&raw.data, &self.shared)? {
                Action::None => {}
                Action::HostReady => {
                    self.shared.begin_peer(0);
                    self.inbox.events.lock().unwrap().reset[0] = false;
                    bridge::drop_peer(self.handle, 0);
                    self.peers[0] = Peer::default();
                }
                Action::End => bridge::close(self.handle),
                Action::Peer(slot) => self.create_peer(slot)?,
                Action::Left(slot) => {
                    let generation = self.shared.generation(slot);
                    self.shared.disconnected(slot, generation);
                    bridge::drop_peer(self.handle, slot);
                    self.peers[usize::from(slot)] = Peer::default();
                }
                Action::Signal(slot, signal) => {
                    let generation = self.shared.generation(slot);
                    if generation != 0 && !self.shared.live(slot, generation) {
                        return Ok(());
                    }
                    if let Err(reason) = self.description(slot, signal) {
                        self.shared
                            .reject_peer(slot, self.shared.generation(slot), &reason);
                    }
                }
            },
            2 => return Err("Room service disconnected".into()),
            3 => {
                let signal: Signal =
                    serde_json::from_slice(&raw.data).map_err(|_| "Invalid local WebRTC signal")?;
                if !signal.valid() || !bridge::signal(self.handle, &signal.message(raw.peer)) {
                    return Err("Cannot send WebRTC signal".into());
                }
            }
            7 => {
                return Err(
                    String::from_utf8(raw.data).unwrap_or_else(|_| "Browser network error".into())
                )
            }
            8 => {
                let peer = &mut self.peers[usize::from(raw.peer)];
                peer.described = true;
                for signal in peer.candidates.drain(..) {
                    if !bridge::description(
                        self.handle,
                        raw.peer,
                        &serde_json::to_string(&signal).unwrap(),
                    ) {
                        return Err("Cannot apply ICE candidate".into());
                    }
                }
            }
            REJECT_QUEUE if self.session.config.remote(raw.peer) => {
                let mut generation = self.shared.generation(raw.peer);
                if generation == 0 {
                    // A guest may overflow before consuming its first offer.
                    generation = self.shared.begin_peer(raw.peer);
                }
                self.shared.reject_peer(
                    raw.peer,
                    generation,
                    "Peer signaling queue exceeded its limit",
                );
            }
            _ => return Err("Invalid browser event".into()),
        }
        Ok(())
    }
    pub fn poll(&mut self) -> Option<Event> {
        while !self.shared.failed.load(Ordering::Acquire)
            && !self.shared.stopped.load(Ordering::Acquire)
        {
            let raw = {
                let mut pending = self.inbox.events.lock().unwrap();
                pending.events.pop_front()
            };
            let Some(raw) = raw else {
                break;
            };
            let (peer, generation) = (raw.peer, raw.generation);
            if let Err(reason) = self.process(raw) {
                if generation != 0 {
                    self.shared.reject_peer(peer, generation, &reason);
                } else {
                    self.shared.fail(&reason);
                }
            }
        }
        for (slot, peer) in self.peers.iter_mut().enumerate() {
            if peer.exists
                && !self
                    .shared
                    .live(slot as u8, self.shared.generation(slot as u8))
            {
                bridge::drop_peer(self.handle, slot as u8);
                *peer = Peer::default();
            }
        }
        self.shared.poll()
    }
    pub fn send(&mut self, peer: u8, lane: u8, bytes: &[u8]) -> bool {
        self.session.config.remote(peer)
            && self.shared.can_send(peer, lane, bytes.len())
            && bridge::send(self.handle, peer, lane, bytes)
    }
}
impl Drop for Transport {
    fn drop(&mut self) {
        self.shared.stopped.store(true, Ordering::Release);
        SESSIONS.lock().unwrap().remove(&self.handle);
        bridge::close(self.handle);
    }
}
