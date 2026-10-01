# Port architecture and maintenance

See [README](README.md) for building and playing, [status](docs/port-status.md)
for remaining work, and [technical notes](docs/port-findings.md) for format and
retail evidence that constrains changes.

## Source ownership

The original code owns movement, AI, combat, animation, menus, scripts and
progression. Keep changes local to portability, verified defects and explicit
port policies. Use plain structs, functions and scoped enums; no inheritance,
RTTI or exceptions. Preserve calculation widths, rounding and random-call order.

| Location | Responsibility |
| --- | --- |
| `src/game`, `src/open` | Gameplay and opening/ending policy, separate phase state |
| `src/lib` | Shared operations compiled once, with explicit phase-owned inputs |
| `src/platform` | SDL host, local extraction, files, input, saves and lifecycle |
| `src/renderer`, `src/audio` | Native draw commands/shaders and software mixer |
| `include/kf` | C++ declarations and authoritative C codec interfaces |
| `codecs` | One Rust static library: audio/TIM parsers, networking and packet validation, with a C FFI module |
| `web` | Browser launcher, resource cache and save persistence |
| `build.json`, `cmake` | Original translation-unit ownership and namespace wrappers |
| `tests` | Native regressions and isolated runtime scenarios |

One coordinator calls opening, game and ending. Each phase owns its arena and
initialized globals; reset both initialized storage and BSS on re-entry. Shared
functions receive call-local views of the correct owner. Do not retain those
views across phase transitions. Original nested loops yield through SDL/Asyncify.

## Types and casts

Correct declarations and runtime models before removing casts. Decode file bytes
with explicit widths, endianness and bounds; native runtime objects use native
pointers and sizes. Keep explicit numeric conversions where narrowing, wrapping
or signedness is intentional. Pointer reinterpretation requires proven alignment,
lifetime and extent. Preserve arena rewind boundaries and real loaded lengths.

The codec headers under `include/kf/{lib,audio,net}`, collected by `codecs/ffi.h`,
own shared structs, enums and constants. `codecs/src/ffi/bindings.rs` is committed
bindgen output with compile-time layout checks. Ordinary builds consume it;
bindgen is needed only after changing those headers:

```sh
nix develop -c bash codecs/bindings.sh
nix develop -c bash codecs/bindings.sh --check
```

The flake pins bindgen and checks that regeneration produces the committed file.
The parsers forbid unsafe Rust; raw pointers and caller-owned output buffers are
confined to `codecs/src/ffi`. Buffers must be disjoint and remain valid for the call.

For co-op, Rust owns native WebRTC/WebSocket connections, signaling validation,
bounded queues, peer generations, packet framing, input sequencing and ordered
fragment assembly. `include/kf/net/transport.h` exposes an opaque handle with
open/send/poll/close calls. Browser WebRTC uses a JavaScript bridge; Rust owns its
session state and validates signaling. Both sides create fixed channels 0
(ordered reliable actions) and 1 (unordered state with no retransmits).
Native channel sends run in independent tasks so one guest's SCTP backpressure
cannot hold signaling or another channel's traffic. Each task has a bounded
queue (32 reliable packets or two state packets); state may be dropped. A
reliable send timeout or queue overflow disconnects that peer, preserving the
rest of the transport. Peer replacement aborts the old tasks, and generation
checks reject queued bytes and late failures from the previous connection.
Rejected peer descriptions, excess candidates and invalid data-channel messages
disconnect that peer without failing the room transport. Rejection purges its
pending packets/readiness and blocks later callbacks until a new peer generation;
ordinary disconnects retain valid packets received before closure. Native close
handshakes run outside the shared signaling loop. A host-ready room message
explicitly permits a new host connection after rejection or disconnection.
Incoming reliable packets have a separate 128-packet/2 MiB allowance per peer,
plus one coalesced state packet. Room events have their own bounded allowance.
Overflow rejects and purges only its peer; beginning a replacement generation
reclaims the old allowance. Native receivers wake peer cleanup on overflow.
The browser also assigns forwarded signals and local callbacks to per-peer
128-event/2 MiB queues. Pending announcements replace old signals/callbacks;
an overflow marker rejects after the relevant announcement is processed, even
when no connection existed when the burst arrived. New announcements discard
old overflow markers. On forwarding backlog, the room service closes the guest
whether it is flooding the host or too slow to receive the host's signals, and
reserves space for its peer-left notification.
C++ owns simulation and gameplay authorization. Rust decodes world snapshots into
pointer-free C records and validates indices against the loaded resource limits.
Header validation rejects unavailable floor/variant combinations before C++ loads
their resources. Combat checks include pending actor animation selections, death
drop IDs and complete billboard frame sequences, including frames produced on the
next update.
Spatial records also have coordinate bounds; placed actors must have an in-map
home position. Effects may briefly cross the map boundary during movement, so
their decoder permits that final step and simulation retires them before terrain
access. Floor-deformation records use counters in the position union and receive
separate validation. Spawning, impact expiry and ground-trail movement each guard
their own terrain reads; blocked boundary cells treat outside neighbors as walls.
The game session retains room credentials in memory, retries interrupted
connections for up to 15 seconds and requires a full snapshot before guest
prediction resumes. A resumed host advances the world epoch before publishing it.
The room service supplies fresh ICE credentials with each peer announcement and
host-ready message. Rust validates and replaces the session's configuration before
either backend creates the new connection; an old room-admission credential must
not be reused for a late join or host recovery. Existing peers keep their own
connection configuration. Deploy the matching room service with both clients.
Room-service rejections are terminal unless marked `retryable`. Rust preserves
their bounded reason in the ended event, including before admission and during
resume; later socket-close callbacks cannot replace it with a reconnect error.
The game displays that reason and ends the session. Temporary service capacity
and host-reconnection responses retain the ordinary transport-error path.
`TURN_URL` can advertise up to seven comma-separated relay URLs with one shared
secret. The browser supports the example's TCP and TLS relay fallbacks; native webrtc
0.21 skips TCP/TLS TURN URLs and requires UDP. Run the transport-only check with
`node services/rooms/crossplay.test.mjs build/linux/coop-crossplay-host build/wasm --turn-tcp`
inside the development shell. It verifies the selected browser relay protocol
and packet exchange without loading gameplay resources.
Add `--https` to run the browser through the deployment nginx template with a
disposable local certificate. The harness checks the HTTP redirect, exact HTTPS
web artifacts, static-file restrictions and the `/rooms` WSS upgrade. Its extra
route serves only the transport fixture; production static routes still use the
room service's allowlist. Certificate trust is limited to that test certificate
and the isolated browser. Native signaling remains direct loopback WebSocket.
`--browser-host` reverses the transport fixture's host direction. `--reject-peer`
adds a third client that submits malformed SDP while the healthy connection is
open; `--reject-packet` negotiates that third connection and sends a forbidden text
message. Both require the host to disconnect only that peer, ignore its late
signals and finish the healthy exchange in the same room. These use no game data.
`--overflow-packets` briefly pauses application polling after the third peer
connects and fills its reliable queue. `--browser-host --overflow-signals` pauses
before that peer's announcement is consumed, then fills its signaling allowance.
Both require the queue-specific rejection reason and a surviving healthy peer.
`--stun-only` starts local STUN without advertising TURN, retains the browser's
ordinary ICE policy and checks that selected candidate pairs contain no relay.
With `--browser-host --overflow-packets`, this also checks the third browser
connection directly. In this environment, mDNS-only browser connections do not
open; configured STUN establishes the direct path, and `--turn-tls` checks relay.
`--turn-tls` enables HTTPS/WSS and restricts the browser to the coturn template's
TLS listener. A verified TLS handshake and the selected candidate's `tls` relay
protocol establish the encrypted relay path; a successful TCP connection alone
does not. Native peers still use UDP-capable transport. Production coturn requires
its own trusted relay-domain certificate, private key and TCP 5349 listener.
Cargo and the flake pin transport dependencies and the Emscripten standard
library's dependencies; builds remain offline. Release Rust panics abort and
never unwind through the C ABI.

## Runtime boundaries

- **Resources:** extract the Japanese ISO or BIN/CUE locally, preserving file
  contents and paths. Load COM/MIX/TMD/MIM/TIM/SEQ/VAB through bounded reads.
  Keep original arena ownership; no replacement gameplay asset format.
- **Rendering:** original producers choose visibility, lighting, materials and
  depth. Submit copied vertices to the native GLES3/WebGL2 renderer. Sort whole
  faces, preserve equal-depth head insertion and split quads afterwards. Reused
  morph scratch pointers must not survive submission. Retained-frame expose or
  resize presents stored pixels without repeating blends.
  Co-op character packs are derived local presentation assets: Rust validates
  their bounded meshes/atlases, while C++ owns GPU texture lifetime and draw
  submission. Their atlases never overwrite KF1's texture memory. Only stable
  model IDs enter snapshots; the room checks a hash of the entire pack and the
  presentation recipe version. Bump that version when changing rigs or attachments.
  The authored slot-1, slot-5, slot-14, slot-19, slot-22, slot-24, slot-26, slot-27, slot-39 and slot-41 rigs
  use replicated motion, weapon attack phase and a twelve-update casting gesture
  timer. Accepted combat/support casts start the
  timer; failed casts do not. Normal world updates expire it even during menus,
  and death, revival and party entry reset it. The free arm casts while the
  weapon arm retains its attack;
  posing never changes collision. Equipment uses KF1 inventory models in an
  owned cache and restores the shared TMD binding after drawing.
  Joint positions, hand attachments and clothing boundaries belong to each
  body. Each arm has its own pivots and grip position; waist boundaries separate
  the slot-24 coat from its asymmetric arms. Its carried book is removed from
  the validated presentation mesh so equipped weapons have a free hand. The
  slot-14 rig retains its extended weapon arm, removes the separate cane, and
  keeps the beard on the head while excluding the raised collar from head motion.
  Slot 22 keeps its hunched posture and long coat, with separate arm boundaries
  and a forward head pivot. Its carried tool is removed by its exact component
  positions; a bounding box would also remove parts of the hand and coat.
  Slot 5's separate pipe is removed, retaining adjacent hand faces. Its source
  forearm is lowered around the elbow into a standing rest pose before skinning,
  rotating normals with the surface, and its offset geometry is recentered.
  Depth boundaries include its inner hands while leaving the coat fixed.
  Slot 19's clasped forearms are opened into a rest pose without removing faces.
  The original fingers cross the center line, so their surface depth determines
  which arm moves them during curation. Their later skinning excludes adjacent
  dress panels, and the sleeve joins share the gradual resting-pose correction.
  Slot 26 opens its crossed forearms using the arm/hand atlas regions and a
  height band that excludes the chest ornament and shoes. Each complete hand
  receives one rotation; the elbow joins blend into the unchanged upper arms.
  The vest, trousers, head and shoes retain their source geometry before posing.
  Slot 1 opens its crossed forearms with separate elbow rotations, retaining all
  geometry. Skin/bracelet atlas regions and spatial bounds exclude the waist
  and long cloth. The rig includes the inward-curving elbow and thumb surfaces;
  they must follow their complete arm during casting and attacks.
  Its leg mask separates skin, anklets and shoes from the long panels and hip
  scabbard by atlas region. Moving only the feet stretches the exposed ankles;
  the full legs need hip and knee motion beneath the fixed cloth.
  Slot 27 removes its separate carried sword and opens the gripping hands, with
  each complete hand receiving one rotation. Its lower head outline distinguishes
  the face from an overlapping raised collar. Waist plates remain on the torso;
  the splayed foot bounds use a separate center from the hip pivots.
  The source pack stays unchanged; previews and gameplay use the same curated mesh.
  Foot clearance considers the entire sole during forward and sideways
  steps. Partial/seated meshes retain their original pose until separately rigged.
  The browser lobby previews those validated meshes in a separate WebGL context,
  using the game's skinning on a copy of the selected mesh. Drag/keyboard rotation
  and the pose/phase controls draw on demand. Pose choices feed the same
  `AvatarMotion` inputs as gameplay; the preview's camera stays fixed around
  rigged bodies while scrubbing. Context restoration retains the selected pose
  and phase. Starting gameplay releases the preview's GPU resources.
  Preview failure does not invalidate character resources or block play.
  Guest entity corrections live in session-owned presentation arrays. Rendering
  receives corrected poses separately from the live records, whose animation
  caches retain stable owners. Collision, aiming and snapshots use simulation
  positions. Small corrections blend over four frames; resynchronization,
  teleports and changes of entity generation/model discard the offsets.
  One-shot projectile impacts and blast children wait for host confirmation.
  A bounded per-slot render history keeps their appearance from rewinding and
  suppresses replay after retirement; snapshots carry a saturating effect age.
  Projectile travel remains predicted. Wrong predicted collisions can be
  corrected without permanently hiding the projectile. Full snapshots preserve
  impact history; reconnects and world/member changes reset it. These caches
  contain no simulation or animation-allocation pointers.
  The Rust KFIII importer requests bounded ISO/BIN byte ranges; the browser
  supplies Blob slices and the native host supplies file reads. Only the four
  character archives, including MOF for Orladin's throne, are read. Rust conversion
  is checked byte for byte against
  the independent Python extractor, including atlas layout and integer rounding.
  Online profiles are private 32-byte random credentials in save storage.
  Only the room service receives the credential; it authenticates players and
  assigns saved roster slots using SHA-256 identities. Rust validates those
  identities before copying fixed-size fields across the transport ABI.
  Snapshots/campaigns contain public identities only, never credentials or room
  resume tokens. Reopening a guest uses its profile and room code; reopening a
  host creates a new room from a manual checkpoint, not a hidden world autosave.
  Online startup computes the selected tree's actual path/length/content SHA-256
  with the disc import manifest format. Room admission and campaign compatibility
  use that digest; the retail constant is only the browser disc-validation target.
  Online random draws belong to actors, effects, players or the shared world;
  snapshots and campaign saves include those streams. NPC/map operations share
  the world stream; weapon spread and poison rolls use the affected player.
  Cosmetic timing, such as the ambient harp sound and floor-item animation, stays
  outside simulation streams. Solo gameplay retains the process-wide sequence.
  Death/spectator and revival notices follow confirmed local membership changes:
  host simulation or an accepted host snapshot, never guest prediction/replay.
  Repeated snapshots of a dead player do not restart the notice.
  Campaign catalogues use Rust's bounded metadata decoder without binding the
  saved world to the currently loaded floor. A catalogue preview is not gameplay
  validation: loading must still check the entire world against its own floor's
  assets. The browser only offers compatible saves owned by the local profile.
- **Timing:** use the focus-aware shared clock and absolute deadlines. GAME world
  rendering owns one three-host-tick interval (about 20 updates/s), including
  scripts. OPEN scene0 uses 22 updates/s; other loops retain their own waits.
  These stable rates are port policy. Do not change per-step arithmetic or add a
  second complete gameplay interval after presentation.
- **Input:** original button logic consumes keyboard/controller actions. Mouse
  look updates view angles. Focus loss pauses and releases capture; browser
  callbacks collect input and service platform work without entering gameplay.
- **Audio:** preserve sound selection, scheduling, spatial and volume decisions.
  Decode banks/sequences into mixer-owned storage that outlives arena rewinds.
  Browser audio starts from a gesture; sample time controls playback timing.
  Co-op simulation captures a bounded history of 32 positional sound events in
  authoritative snapshots. Guests consume each sequence once using their local
  listening position and effects setting; prediction never emits audio. Initial
  synchronization, reconnects and world/member changes skip old history. Local
  menu sounds stay local. Capture scopes surround synchronous simulation calls,
  never coroutine suspension lifetimes.
- **Saves:** preserve save-point rules and original menus. Serialize versioned
  fixed-width fields, rebuild transient references, and report success only after
  native/IndexedDB persistence completes. Browser stores can still be evicted.
  If campaign request creation throws, detach its transaction callbacks before
  aborting so a late event cannot report success or overwrite a subsequent save.
- **Errors:** required in-game resource and allocation failures use `host_fail`.
  Native failures release capture, pause audio, display the message and exit 1
  after dismissal. Optional file failures return to callers; startup CLI/import
  errors use console diagnostics.

## Behavior constraints when refactoring

- Movement updates Z before testing X; later checks and blocking calls must see
  live state. Runtime map coordinates use `z, x`; warp endpoints use `x, z`.
- Pools traverse live slots in ascending order. Preserve free-slot guards, tie
  behavior, final actor bindings and default output distances. Placement sentinels
  stop reading source records, but remaining destination slots still need reset.
- Armor regeneration precedes drain in head/body/shield/arm/leg order. Keep each
  HP adjustment because clamping and death handling make combined deltas differ.
- GAME camera paths fetch then increment; OPEN paths increment then fetch.
  Preserve sentinel, frame-decrement and transition order.
- Menu labels sometimes write only part of a row, reusing earlier glyphs. Preserve
  input priority, cancel paths and dialogue grant/consume/notification order.
- Signed shifts, byte wrapping, partial vector/matrix writes and aliasing are
  meaningful. `matrix_interpolate` updates rotation only, preserving translation.
  Module snapshots recursively copy C arrays, including nested arrays.
- A floor-local definition number is not a global actor identity. Keep unresolved
  resource meanings explicit rather than inventing names.

## Verification

Run in the pinned shell; do not commit retail assets or generated build products.
Build Linux and WASM after code changes and inspect the relevant callers and diff.
For tooling/build changes, also run `nix flake check -L`.

```sh
nix develop -c env ASAN_OPTIONS=detect_leaks=0 python3 -m unittest discover -s tests -v
nix develop -c cmake --preset sanitize
nix develop -c xvfb-run -a -s '-screen 0 1600x1200x24' \
  python3 tests/runtime_scenarios.py --data /path/to/extracted/disc
```

Disabling leak scanning is needed only where process inspection is blocked;
address and undefined-behavior checking stay enabled. Native regressions cover
COM copy spans and malformed bounds, scalar/array resets, matrix interpolation
and required/optional file errors. CTest also runs Rust tests for signaling roles,
stale callbacks and bounded queues, plus the C ABI packet/runtime regressions.
The room-service integration drivers exercise real native/native and
native/browser channels separately; a successful build alone is not coverage.
`node services/rooms/ending.test.mjs build/linux /path/to/data /path/to/characters.kfa`
uses a small fixture host and isolated real native clients to check terminal-state
delivery, abrupt host loss and the nonterminal room-close control. It enters the
ending without navigating levels; it does not verify a natural campaign exit or
the complete cinematic. Use `build/sanitize` to run it against sanitizer binaries.
`SDL_AUDIODRIVER=dummy xvfb-run -a build/linux/coop-runtime-test --notice-check /tmp/notice.ppm`
checks the five-second guest notice, retained-frame restoration and session-exit
cleanup in an isolated window, without loading resources or navigating a level.
`SDL_AUDIODRIVER=dummy xvfb-run -a build/linux/coop-runtime-test --world-states /path/to/data`
loads all five retail floors and all three floor-five variants directly. Each
loaded world runs 20 updates with full/fast snapshot round-trips against its real
asset limits. At the first entry it also casts Fire Ball and checks the replicated
gesture through expiry. It neither reads nor writes saves and requires no navigation.
Use `build/sanitize` with `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1`
for the same checks under sanitizers. This is a loaded-world smoke check, not a
complete combat or progression playthrough.
`SDL_AUDIODRIVER=dummy xvfb-run -a build/linux/coop-runtime-test --party-travel /path/to/data`
places a party directly at real floor-five gates. It runs a variant-one/two round
trip and a floor-five/four round trip with living and spectating hosts, a living
guest, a spectator and an absent member. It checks personal state, snapshots and
the complete collision occupancy grid. This exercises the gameplay transition;
the session still owns epoch changes and network load barriers. It does not
establish travel behavior between live network clients. The fixture uses the same
phase initialization as normal game entry, including resets between cases.
The browser lobby driver's `--prediction` scenario moves briefly in the starting
area with 150 ms delayed snapshots and a 500 ms snapshot gap, then checks recovery.
The exact shared replay/live limit is covered by the native runtime suite; the
browser scenario checks transport recovery and rendering, not numerical timing.
Its `--preview` scenario renders every imported body locally, checks casting and
rotation, restores a lost preview context and verifies cleanup at game startup.
It uses no gameplay input; this is character presentation coverage, not co-op
combat coverage.
`--preview-only` runs the local character checks without TURN, KF1 resource
import or game startup. Supply `-` for the Japanese disc argument and the normal
character pack/disc argument. This permits rig review while gameplay checks are
stopped; it does not cover preview cleanup at game startup.
Set `KF_TEST_AVATAR` to capture that selected body's front/side rest pose as well
as the rigged bodies' casting captures.
`KF_TEST_AVATAR=39` selects that guest body for the lobby driver's `--pose`
scenario: one short starting-area step and sword swing, followed by reconnect.
The `--combat` scenario takes the same native executable/resource arguments as
`--crossplay`. It uses a fresh native host and browser guest, one short starting
step and at most 36 sword swings to check lethal friendly fire. Captured full
snapshots are inspected by `coop-runtime-test --snapshot-summary FILE` through
Rust's metadata decoder. It checks that the world continues and reconnecting
preserves membership and the host's death. It does not cover spells or travel.
`--spell-flow` uses those arguments with two browsers and a prepared entry-point
snapshot. The guest has Fire Ball and the host has one HP, just ahead of its
projectile muzzle. One normal magic input checks lethal friendly fire, MP use,
unchanged XP/gold/training, continued simulation and reconnect without revival.
It needs no movement. The snapshot helper runs through the retail decoder and
can use the sanitizer binary; this is browser gameplay coverage for Fire Ball,
not a live check of every spell or native/browser spell combat.
The `--save-flow` scenario takes those same executable/resource arguments and
uses two isolated browsers. `coop-runtime-test --save-fixture RESOURCES INPUT OUTPUT`
decodes a captured full snapshot against retail assets, places the host facing a
real save point and marks the guest dead. Three menu inputs exercise the actual
save path. The check reads the durable IndexedDB checkpoint and verifies revival.
The fixture also gives the guest a Green Dragon Staff. Using its real inventory
menu returns both players to the floor entry; withholding the guest's loaded
acknowledgment for 600 ms verifies that live snapshots and simulation wait at the
barrier. Releasing it resumes ticks with a new epoch and preserved personal state.
Rehosting then restores the earlier manual checkpoint. Returning living guests
retain their saved state while awaiting the existing safe-entry admission rule.
`--inspect-world RESOURCES INPUT` validates the full snapshot and reports the
epoch/floor/variant and all nonempty members' identities, presence, HP/MP, gold,
XP/training, cells and staff counts. These test-only helpers introduce no gameplay
teleport API. This checks
two-browser return travel, not cross-floor or native/browser travel.
The `--travel-flow` scenario uses the same arguments and seeds the party at the
real floor-five entry/gate, clearing actors in that fixture so admission remains
safe. One short host step out and back checks the all-living-member gate, travel
to floor four, a held loaded acknowledgment, preserved personal state and
reconnect without immediately retriggering the entrance. `--travel-fixture`
prepares that snapshot through the same retail decoder as the save fixture.
`--travel-crossplay` extends this check with two native guests, filling all four
party slots. It checks the same load barrier and personal state, then reconnects
both a browser guest and a native guest on floor four. The native clients can use
the sanitizer executable. This covers a browser-hosted floor-five/four transition;
other routes and native-hosted travel still need separate verification.
The `--wipe-flow` scenario uses `--wipe-fixture`, which retains the retail actors.
It checks repeated host death notices and checkpoint restoration without input.
This reproduced a browser stack overflow in the notice bitmap allocation; that
buffer now lives on the heap. The reproduction and fix were checked with
WebAssembly stack guards; native ASan alone did not expose the smaller stack.

The runtime driver builds isolated observers, enters floors 1–5 then 1, exercises
save/load and GAME/OPEN re-entry, and uses separate saves and display. `--movement`
adds fixed input sequences; `--source-root` selects a reference checkout and
`--baseline` compares its six saves byte for byte. This does not verify natural
stairs/warps, every combat/script path, a full ending, audio/pixel equivalence or
browser behavior. Use input scenarios only when authorized; otherwise keep
observations without input and leave the user's client and saves untouched.
