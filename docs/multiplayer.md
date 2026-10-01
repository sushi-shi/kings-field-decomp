# Multiplayer implementation

The browser lobby hosts or joins a four-player party using a room code. The
host owns simulation and authorizes inventory, combat, progression and travel.
Guests predict movement and reconcile with authoritative snapshots. Players
remain in the same party and can accidentally hurt or kill one another.

## C++ boundaries

The game, resource decoders, protocol and networking session are C++20. There is
no Cargo build or generated language binding. This follows `port`'s checked
byte-reader and owned-container approach:

- `src/net/codec.cpp` validates packet framing, commands, input bundles and
  fragment bounds. Encoding uses explicit field widths and byte order.
- `src/net/world.cpp` validates snapshots into temporary owned records. A full
  structural preflight runs before loading a different floor; resource-aware
  validation must then succeed before publishing the simulation. Rejection
  after a resource switch ends the guest session.
- `src/net/transport_session.cpp` owns signaling validation, generations and
  bounded event queues. Callbacks enqueue data; they never enter gameplay.
- `transport_native.cpp` uses pinned libdatachannel for WebSocket, ICE, DTLS and
  SCTP. `transport_web.cpp` uses the browser WebRTC bridge. Both feed the same
  session and expose the same open/send/poll/close interface.
- `src/platform/avatar_{pack,source,disc}.cpp` validate character packs and
  import bounded ranges from local ISO/BIN files. Character findings are in
  [avatar notes](avatar-notes.md).

The reliable actions lane is ordered. The state lane is unordered with no
retransmits; pending received state is coalesced per peer. Reliable receive
queues allow 128 events or 2 MiB per peer, independently of room events and
other peers. Native send buffering is capped at 32 maximum-sized reliable
packets or two state packets; reliable buffering without drain progress for
two seconds disconnects that peer. Stale callbacks cannot affect replacements.
Malformed peer traffic and peer queue overflow close only the offending peer.

Checked parsing and limits remain necessary in C++; the language does not make
unchecked engine or networking-library operations memory safe.

## Resources and presentation

Each player supplies their own Japanese KF1 and US KFII/KFIII resources. Room
admission compares the selected game-resource tree and character presentation
recipe hashes. Select the same language before joining; language changes are
disabled during multiplayer. Disc data and character packs are never transferred
between peers or uploaded by the lobby.

The pack currently contains 39 textured meshes, including pose variants and the
throne model. Ten bodies have movement, melee and casting poses. The complete
catalogue roster and all remaining bodies are not yet animated; Yvette's palette
and Airon's sick form remain unresolved. See the avatar notes for evidence.

Private profiles identify returning characters; public hashes appear in the
party roster. Campaign checkpoints preserve the party and world, while the
browser/native save directory retains the private credential. Save work is not
a priority for the current migration.

## Verification

Inside `nix develop`:

```sh
cmake --preset linux
cmake --build --preset linux
ctest --test-dir build/linux --output-on-failure
ASAN_OPTIONS=detect_leaks=0 python -m unittest discover -s tests
emcmake cmake --preset wasm
cmake --build --preset wasm
node --test services/rooms/server.test.mjs
node services/rooms/webrtc.test.mjs build/linux/coop-transport-test
node services/rooms/crossplay.test.mjs build/linux/coop-crossplay-host build/wasm --https --stun-only --reject-packet
node services/rooms/crossplay.test.mjs build/linux/coop-crossplay-host build/wasm --turn-tls --reject-peer
```

These use isolated fixtures and do not navigate the running game. The snapshot
fixture corpus matched the previous implementation byte for byte (5,127,900
bytes, SHA-256 `349a361d2f7c85f69d0b59efb0747f74ed6068f4aab7b3d48eb02b8847802f9c`).
The SLUS-00255 character import also matched the existing 17,574,076-byte pack.
Malformed-input, queue-isolation and coroutine cancellation checks run without
game resources. Existing optional resource fixtures can exercise all five retail
map grids and the actual character import.

The local browser TLS TURN relay check also passes through the HTTPS/WSS
deployment template. Relay negotiation can take tens of seconds while native
libnice exhausts direct candidates and DTLS retries; the transport fixtures
allow 60 seconds for the exchange and exit as soon as it completes.

Public NAT/firewall combinations and sustained gameplay across travel, combat,
reconnection and campaign restoration still need user testing. Local transport
success does not establish those behaviors. Deployment templates and launcher
instructions are in the [README](../README.md).
