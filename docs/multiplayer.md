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

The browser waits in a roster lobby while resources/world state synchronize;
only the host starts gameplay. Connected guests spawn in clear nearby positions
instead of overlapping at a tile center. Private profiles support reconnects.
Only the host persists the campaign. Rehosting reserves saved characters and
displays per-character claim codes, allowing a guest on another device to keep
the saved identity, inventory and avatar. Codes are room-scoped, host-visible
bearer credentials. Absent characters are retained, and unused slots admit new
players up to the four-character limit. The host may start alone.

Against a fresh unarmoured character's 30 HP, the starter sword deals 10 HP per
fully charged friendly hit; partial charge and armour still affect damage.
Death currently removes the third-person body and switches the victim to
spectating a living party member; there is no third-person death animation yet.

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
node services/rooms/browser-lobby.test.mjs build/wasm /path/to/KF1.bin /tmp/lobby.png --lobby /path/to/characters.kfa
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

### Sanitizer and analyzer audit (2026-10-01)

The multiplayer audit used the pinned Nix tools, isolated fixtures and no manual
navigation. Retail coverage means 20 simulation updates and full/fast snapshot
round trips for floors 1–4 and all three floor-5 variants, plus bounded party
travel fixtures; it is not a complete playthrough.

| Check | Result and limits |
| --- | --- |
| ASan + UBSan + `_GLIBCXX_ASSERTIONS` | Seven CTest suites, all-floor snapshots and party travel pass with the normal renderer. An additional forced-Mesa-softpipe travel run reports a buffer overread inside the driver's framebuffer blit; that configuration remains unresolved. Leak detection disabled for the environment restriction below. |
| Valgrind | Five isolated codec suites, runtime/campaign-save suite and all-floor fixtures: zero memory errors; no definitely, indirectly or possibly lost blocks. Library allocations remain reachable at exit. |
| MemorySanitizer | Resource codecs, record codecs, protocol and world codec pass. Avatar suite is inconclusive: the same uninitialized read reproduces with just `std::set<unsigned>` insertion/destruction against the uninstrumented standard library. |
| Parser fuzzing | 20,094 multiplayer/avatar/importer inputs and 63,366 resource inputs under ASan/UBSan; no sanitizer failures. Two short, seeded 60-second runs. |
| Clang analyzer, clang-tidy, cppcheck | All 132 production translation units checked. Actionable bounds/allocation findings fixed; changed units rechecked with Clang tools. Remaining diagnostics were reviewed, not blanket-suppressed. Completion does not mean zero warnings. |
| ThreadSanitizer | Seven CTest suites and the all-floor fixture pass. Native WebRTC integration reports lock-order inversions in libdatachannel and race reports involving uninstrumented dependencies; this integration is **not clean**. The lock inversions remain a potential dependency deadlock. Browser WebRTC uses a different backend and is not covered by native TSan. |
| LeakSanitizer | Even an empty-program probe fails because process inspection is restricted in this environment. |

The audit fixes unsupported actor attachment indexing, unchecked allocation of
required map-object effects, uninitialized stationary-effect directions and
boundary-door collision writes. Regression assertions compare avatar fields
instead of reading struct padding. Resource fixtures initialize the selected
language before loading retail data.

Run the sanitizer and resource fixtures inside `nix develop`:

```sh
cmake --preset sanitize -DCMAKE_CXX_FLAGS=-D_GLIBCXX_ASSERTIONS
cmake --build --preset sanitize
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build/sanitize --output-on-failure
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 xvfb-run -a build/sanitize/coop-runtime-test --world-states /path/to/extracted/KF1
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 xvfb-run -a build/sanitize/coop-runtime-test --party-travel /path/to/extracted/KF1
```

The multiplayer fuzz harness needs no retail assets, filesystem access or live
network. Optional seeds are individual world snapshots, packets and character
packs. Split the length-prefixed output of `coop-world-codec-test` into separate
files before using it as a seed corpus.

```sh
mkdir -p build/fuzz-corpus
clang++ -std=c++20 -O1 -g -D_GLIBCXX_ASSERTIONS -Iinclude \
  -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer \
  tests/fuzz_multiplayer.cpp src/net/codec.cpp src/net/world.cpp \
  src/platform/avatar_pack.cpp src/platform/avatar_source.cpp src/platform/avatar_disc.cpp \
  -o build/fuzz-multiplayer
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 \
  build/fuzz-multiplayer build/fuzz-corpus -max_total_time=60 -timeout=5 \
  -rss_limit_mb=2048 -max_len=131072 -artifact_prefix=build/
```
