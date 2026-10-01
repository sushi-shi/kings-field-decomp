# Port status

Linux and WebAssembly build and run the original game/opening through native
platform, rendering, audio and save interfaces. See [architecture](../PORTING.md)
and [technical constraints](port-findings.md) before changing behavior.

## Verified scope

- User checks cover Linux controls, general rendering, music/menu/attack sounds
  and brief combat. Native save/load worked through the original menus and a
  fresh process.
- The user confirmed browser audio. Isolated Chromium checks restored the resource
  cache and preserved a seeded save byte-for-byte across a full browser restart.
  Original-menu save/load also passed across a page reload. Loading that save
  through the game menu after a full browser restart remains unexercised.
- The reported plaque edge and flame behavior were reproduced/confirmed in retail;
  neither is an active port defect. The renderer is not claimed pixel-exact.
- Forced GAME/OPEN cycles and floor-entry/save comparisons passed sanitizers.
  They do not establish natural story completion or full-game coverage.

## Remaining work

Co-op implementation is active and incomplete. The target remains real 2–4 player
Linux/browser co-op, with a player-authoritative 20 Hz world, private rooms and
self-hosted signaling/TURN. Rust owns networking and all incoming protocol
decoding; retain a small codec-style C ABI and C++ gameplay.
Prioritize public browser hosting and character readiness. Save refinement is
deferred at the user's request, and automated gameplay scenarios are stopped;
transport-only checks and local character previews remain available.

Current transport evidence includes four native clients, guest token-based resume,
and Chromium/native exchange through Rust WebRTC, including 60 KB reliable
packets and the state channel. Packet parsing, fragment assembly, input sequencing,
bounded queues and stale callback rejection have regression coverage. Two actual
native games admit/synchronize through Rust. These transport checks alone do not
prove complete multiplayer gameplay.

The website launcher now imports local resources, hosts a room with a copyable
code, or joins by entering that code. The room service can serve the WASM build
on the same origin; public deployments need HTTPS/WebSocket routing and configured
STUN/TURN. Native game tests exercise forced guest and host connection loss,
token-based automatic reconnect, and a fresh world epoch after host recovery.
Death during the disconnect grace period remains spectator state on reconnect.
Two isolated Chromium players also import their own local discs, host/join by
code, synchronize the real world through a forced authenticated TURN relay,
recover a dropped guest connection, and return to the lobby with cached resources
after the room closes. Public internet deployment is not yet verified.
Linux deployment templates now cover the room-service systemd unit, same-origin
nginx HTTPS/WebSocket proxy and shared-secret coturn configuration, with the
release layout and setup steps in the README. No public service was installed.
The coturn template parses and opens an overridden loopback listener with the
pinned binary; this is configuration coverage, not an internet relay check.
The nginx template now passes a local HTTPS/WSS transport check using temporary
loopback listeners and a disposable certificate. It verifies the HTTP redirect,
byte-identical HTML/JS/WASM delivery, private-file rejection, browser admission
and rejection reasons over `/rooms`, and 60 KB reliable packets plus state traffic
with a native host. Browser statistics confirm TCP TURN relay use. Only the test
certificate is trusted in the isolated browser; native signaling connects directly
to the local room service. This does not validate native WSS or the target server's
domains, certificates, firewall or systemd paths. Those still need deployment
verification. No gameplay was started. The example advertises UDP, TCP and TLS
TURN through a bounded comma-separated `TURN_URL` list. Browsers can fall back
to TCP or TLS; the pinned native Rust library explicitly skips TCP/TLS TURN URLs
and still requires UDP access. The coturn template includes a TCP 5349 TLS listener
and relay-domain certificate/key paths; those files must be installed and renewed
on the target server. Its TLS path now passes the transport-only `--turn-tls`
check: a verified local TLS handshake, browser admission over HTTPS/WSS, exclusive
`turns:` candidates, selected `tls` relay statistics, 60 KB reliable packets and
state traffic with a native host. The native host remains UDP-capable. The local check
uses a disposable certificate and removes private-address denies solely for its
loopback peers; it does not establish public certificate or firewall correctness.
An incomplete TURN URL/secret pair now fails at room-service startup instead of
silently disabling the relay.
Room-capacity rejection exposed a service crash: an excess client's malformed
frame could emit an unhandled error during its close handshake. The error
handler is now installed before the capacity check. A raw-socket regression
reproduces that frame and confirms the existing room and health endpoint survive;
the room-service suite covers this and the incomplete TURN configuration.

A later audit found that hosts reused their admission-time TURN credentials for
new peers, even after the one-hour expiry. Guests also retained old credentials
when a host returned. Peer and host-ready messages now carry freshly issued ICE
configuration; the shared Rust session validates it before either transport
backend creates a connection. A room-service regression advances the clock past
both expiries, verifies credential signatures and checks delivery before the new
offer. The regression failed before the fix and now passes. Rust checks
cover role/admission gates, bounded credentials and preserving the previous
configuration on rejection. Both client builds, four-native-client transport
exchange/resume and browser/native packet exchange pass. The browser transport
harness had an outdated malformed-RLE offset (31-byte story tail instead of
schema 11's 35); correcting that restores its rejection control. No gameplay
was started. These checks do not verify a long-running public TURN session.

The room-service suite also covers relay-list bounds,
normalization and shared credential signatures. The transport-only crossplay
harness accepts `--turn-tcp`: it starts isolated coturn, restricts the browser to
TCP relay candidates, verifies the selected candidate's `relayProtocol` is `tcp`,
and exchanges 60 KB reliable packets plus state traffic with a native host.
This passes without loading game resources. It verifies the browser's TCP relay
path, not native operation on a UDP-blocked network or a public deployment.

Native outgoing data previously awaited SCTP sends inside the shared signaling
loop. A congested reliable channel could stall all peers for two seconds, then
fail the whole transport. Each channel now drains a bounded queue independently;
reliable timeout/overflow disconnects only its peer, while state may be dropped.
Rust checks hold one send pending, verify ordered delivery to another peer and
continued state traffic, then verify the timeout leaves the other peer usable.
They also cover stale queued bytes and an in-flight failure after peer
replacement. The native build, Rust suite, four-native-client exchange/resume
and browser/native TCP relay exchange pass. These are transport-only checks;
public-network congestion and gameplay were not exercised.

Permanent room-service rejections now end the transport with their original
reason. Previously a failed resume could keep retrying a missing room or invalid
token until the reconnect timeout replaced the useful diagnosis. The service
marks temporary capacity and host-reconnection errors `retryable`; Rust bounds
and validates that response, preserving terminal reasons through socket-close
callbacks. C++ displays the ended-event reason rather than always claiming the
host closed the room. Both builds, all ten service tests and the Rust suite pass.
Native and browser transport fixtures verify a rejected resume reports `Room
unavailable`, then complete normal packet exchange and native guest recovery.
No gameplay scenario was started for this change.

Peer negotiation/data-channel failures previously failed the entire host transport.
Native and browser backends now reject just that peer, purge its pending packets
and readiness notification, and discard subsequent callbacks/signals. Native
connection closure cannot hold the signaling loop. Ordinary disconnection keeps
already received valid packets in order; a room-service host-ready notification
permits a fresh host generation. Shared Rust checks cover rejection isolation,
queue accounting, credentials, stale callbacks, replacements and ordinary-close
ordering. Transport fixtures now support both host directions and a third client
with malformed SDP or a text data-channel message. Both builds and the Rust suite
pass. Both hosts reject malformed SDP and text packets while an existing healthy
connection completes its exchange in the same room. The browser-host text-packet
case also passes through HTTPS/WSS and selected TLS TURN. Four native clients
complete guest and host resume with fresh packet exchange after each. These are
transport-only checks; no gameplay was started.

Incoming reliable packet queues now have separate 128-packet/2 MiB allowances
for each peer, plus one coalesced state packet. Overflow purges/rejects only its
source; other peers and room-control events retain their budgets and ordering.
Replacing a peer reclaims stale queued packets. Browser signaling assigns both
forwarded messages and local callbacks to bounded peer queues; queued overflow
can reject a peer whose announcement/first offer has not yet been consumed.
New announcements remove old signals, callbacks and rejection markers.
Server-side forwarding backpressure also used to close the recipient host.
It now closes the guest on either side of forwarding, reserving room for the
resulting peer-left message; a host sending to a slow guest keeps its room.
The service regression pauses an actual host socket, floods forwarded SDP and
confirms the same room accepts a healthy guest afterward.
All twelve service tests pass, including both forwarding directions. Rust checks
cover independent count/byte limits, full healthy queues, state coalescing, room
events and replacement generations. Both client builds and native host/guest
reconnection pass. Transport fixtures verify native packet overflow, browser
packet overflow through TLS TURN, and browser signaling overflow before a peer's
announcement is consumed, while a healthy connection finishes in the same room.
Without an ICE server, the same-page browser-to-browser fixture exchanges only
mDNS candidates and remains in ICE's new state in this environment. Configuring
local STUN opens that direct connection. The `--stun-only` control advertises no
TURN servers, retains ordinary browser ICE policy and requires both selected
candidates to be non-relay; both host directions pass over HTTPS/WSS. The
browser-host packet-overflow scenario also passes, with selected server-reflexive
and peer-reflexive candidates. Its TLS relay control still passes. Public cross-network NAT and
firewall combinations remain unverified. No browser privacy settings were changed.

World snapshots now use the same Rust boundary. C++ supplies pointer-free records
and trusted resource limits; Rust owns field encoding, bounded RLE, persistent
floor record parsing and validation before publishing decoded state. Malformed
indices, links and runs are rejected without modifying the destination. Native
startup and a Chromium/WASM codec round trip pass. Gameplay and rendering remain
C++; this boundary reduces parser exposure but does not establish RCE immunity.

Online root menus now run as local presentation tasks independent of prediction
and incoming snapshots. Equipment, combat-spell selection, consumables, support
magic and discarding use reliable host-authorized commands with explicit replies.
The host checks character generation, epoch, duplicate sequence, life/connection
state, inventory ownership and action-specific rules. Local sound/HUD settings
survive snapshots. Protocol v3 includes bounded dialogue/shop replies and excludes
older clients through the same room and packet compatibility checks.
Unit checks cover authority, personal inventory isolation and menu continuation
across snapshot replacement. A short two-browser check opens the guest's root
menu and uses the starting herb, verifies the host's acceptance and continued
world snapshots, then reconnects normally. Keep automated gameplay checks short
and isolated. Leave navigation/playthrough checks to manual playtesting unless
they are straightforward to automate.

NPC interactions now execute the original dialogue/exchange logic on the host and
return the selected page to the initiating player. Dialogue progression resumes
when that interaction closes. Shop menus use host-validated buy/sell commands;
each trade checks the active merchant, current proximity, gold, ownership and
stack capacity. Limited gold-cross stock remains personal. Focused checks cover
these rules and malformed interaction replies. Merchant navigation and full
dialogue/quest flows still need manual playtesting.

Shared quest rewards now record campaign completion separately from each
character's receipts. The key, Healing and harp exchanges consume only the
initiator's offering and reward every admitted character, including disconnected
members and spectators. Fire Ball dialogue and sanctuary magic use the same
ledger. New characters start at level one, inherit the campaign's reached-floor
progress and receive completed rewards once. Full item stacks retain pending
rewards until room becomes available. Snapshots and campaign saves preserve both
ledgers, and Rust rejects unknown reward bits or receipts for incomplete quests.
Focused tests cover real exchange commands, repeated dialogue states, late
admission, pending rewards, prediction isolation and durable saves.

The floor-two character transfer and floor-five Moonlight Sword transformation
now run as host-driven shared scenes. Authenticated dialogue completion starts
them; snapshots carry the phase, initiating character, camera origin and reserved
object identity. Combat, player controls, menus, travel and new admissions pause
while snapshots and reconnections continue. The transformation consumes one
Dragon Sword from its initiator and leaves the result available through personal
loot claims. Its decorative blast advances without dispatching combat damage.
Short headless fixtures cover guest initiation, every phase through the Rust
codec, reconnect/resnapshot, prediction isolation, occupied drop pools and
inventory isolation; malformed phases, cameras and references are rejected.
These scenes still need visual gameplay review.

The floor-five boss reveal now detects any living connected player's entry.
Both dialogue pages are shared; each connected admitted character, including
spectators, acknowledges the current page through a bounded reliable command.
Disconnected readers and waiting newcomers do not block progression. The host
applies the encounter's attack animations and terrain change once, after the
second page. Page and acknowledgement state survive snapshots; ordinary actions
remain blocked during the reveal. Headless checks cover guest initiation, facing,
prediction, reader order, stale/duplicate pages, spectators, disconnects, and the
final encounter state. The actual boss-page presentation still needs gameplay
review. Floor-one area scripts also now use party occupancy: any living member
can arm a trigger, and it completes when all living members leave the area.

Guests can activate lift doors, paired hinged doors and linked switches through
the reliable command channel. The host checks identity, proximity, NPC priority,
facing and locks before changing the shared object. A paired door uses one
request for both leaves. Focused runtime checks cover these authorizations.

Loose items, gold and container contents now use personal claim masks stored with
each campaign character. The host grants each component once, checks current
range, locks, facing, inventory capacity and object generation, and keeps other
players' copies available. Reused drop slots reset claims and advance generation;
delayed confirmations cannot collect a replacement drop. Claims are included in
snapshots and campaign saves. Pickups disappear only for their claimant, and
container opening is local presentation. Focused checks cover duplicate claims,
independent party copies, floor separation, reconnect, durable saves, full stacks,
gold overflow and reused slots. The browser import/host/join/menu/reconnect smoke
test also passed with the new snapshot format; natural pickup UI needs playtesting.

Keys, chalice/seals, harp and illusion staff now share host-authorized item logic
with solo play. Guests can unlock shared objects, activate linked mechanisms and
start the original harp floor deformation using their own inventory. The mirror
shows local information from the synchronized world. Focused checks cover wrong
keys, range, ownership, personal consumption and occupied/full effect pools.
Green Dragon Staff use now requests a shared return to the current floor's
entrance. The host validates ownership, cancels local continuations, advances the
epoch, applies the floor-five entry variant when needed and publishes a full
snapshot before guests resume. All admitted characters move together; health,
inventory, death and disconnect state remain intact. Focused tests cover
authorization, pending-request duplication, collision occupancy, prediction
isolation and preservation of character state. Natural staff use and the
floor-five variant transition still need gameplay verification.

Guests can read signs and inscription panels after host validation of proximity,
object identity and image-number bounds. Presentation uses the existing image
viewer locally while the shared world keeps running.

Guests can activate unlocked restoration points; a living guest entering the
floor-three sanctuary triggers restoration and shared magic rewards independently
of the host. Existing spectator revival rules apply.

Guest save-point interactions now receive host validation and show a five-second
notice asking the host to save. The notice does not open a menu or suspend the
shared world. Focused authorization checks confirm that inspection neither writes
a checkpoint nor heals/revives anyone; an isolated native rendering check verifies
notice expiration, retained-frame restoration and cleanup when leaving a session.
The browser client builds with the same notice renderer. Natural save-point use
and browser notice presentation still need gameplay verification.

Focused combat checks now run all five offensive spells, Moonlight's beam and
ground trails, and radial blasts through the actual effect dispatcher in an empty
room fixture. These exposed and fixed missing party damage from Moonlight trails
and Wind Cutter using elemental rather than physical damage against teammates.
The checks cover lethal friendly fire, piercing, projectile owner exclusion,
multiple nearby blast victims, spectator exclusion, child-effect ownership and
absence of PvP XP, loot or training. A real enemy-kill check covers full XP for
admitted spectators and no duplicate award for a dead enemy. These use bounded
synthetic placements and damage values, not a natural combat playthrough.

Both original campaign exits now publish a terminal world snapshot before the
host leaves. The host waits for full-snapshot acknowledgements, including a
reconnect opportunity for interrupted admitted players within the existing
15-second barrier window. A guest that has received this state enters the ending
even if the connection then disappears; ordinary room closure still returns to
the lobby. The original shimmer, screen transition and audio fade precede the
ending on each client. Focused checks exercise both exit predicates, terminal
wire validation, frozen progression and pending living/spectating readers. A
small fixture host also sent the terminal state to an isolated real native game:
it reached the ending program after orderly closure and abrupt host loss. A
nonterminal closure returned to lobby. These checks avoid navigation; they do
not establish natural co-op exit use or complete cinematic playback.

Required completion work:

- Extend world validation coverage across floors and combat states. Rust now
  rejects snapshots whose rendered map cells have out-of-range
  rotation indices, and rejects unknown collision-cell kinds at the Rust boundary.
  The invalid-rotation regression failed before the fix. All five extracted retail
  floors pass full/fast round-trips of their five map grids through the C codec ABI
  (`coop-runtime-test --map-grids RESOURCE_DIRECTORY`). This checks terrain,
  not all actor/effect states on those floors. Rust now rejects out-of-world actor,
  object and NPC positions, invalid placed-actor home coordinates, extreme
  vertical/effect coordinates and unsafe floor-deformation counters. A malformed
  actor-coordinate regression failed before these checks. Native spell updates
  now retire off-map projectiles before terrain reads, including lightning that
  expires just after movement, and ground trails that cross the edge. Off-map
  Fire Wall probes and blocked-cell neighbor reads are guarded. Attribute-zero
  lightning aiming and jump attacks use that attribute's own height entry;
  nonzero attributes retain their preceding-entry lookup. Direct fixtures cover
  these paths, full/fast snapshot rejection and valid dynamic-actor sentinel
  tiles. Native and ASan/UBSan suites pass with no sanitizer diagnostics, and the
  browser build passes. Actual native/browser clients also pass admission,
  starting-area menu use and reconnect checks in both host directions over local
  TURN with these bounds. Combat validation now rejects invalid death-drop IDs,
  unavailable clips selected by pending actor actions and billboard sequences
  that would exceed the sprite table on a later update. Ten real spell variants,
  including alternate lightning sprites and their child effects, pass full/fast
  snapshots over 20 updates. Header inspection rejects unavailable floor/variant
  combinations before the client attempts to load their resources.
  `coop-runtime-test --world-states RESOURCE_DIRECTORY` loads actual retail data
  for floors 1–4 and all three floor-five variants, then checks full/fast snapshots
  over 20 world updates per variant. All seven pass natively and under ASan/UBSan
  with real asset limits. No saves or navigation are involved. This covers initial
  and briefly advanced worlds, not every actor/effect or campaign state; further
  state-family validation coverage remains.
  Replay and
  live guest updates now share a four-tick/200 ms prediction budget; suspended
  gameplay continuations also stop at that limit. Small local camera corrections
  blend over four render frames, with immediate resets for full resynchronization,
  epoch/generation changes, death, story scenes and large corrections. This changes
  presentation only. Focused checks cover the combined replay/live cap, wrapped
  angles, repeated corrections, teleports and extreme coordinates. Actual wandering
  enemy and scatter-projectile updates agree across two clones without changing
  their source state. A short isolated browser movement check over local TURN
  recovered from 150 ms delayed snapshots and a 500 ms snapshot gap; the resulting
  gameplay view was inspected. NPC/map operations now use world-owned random state,
  while weapon spread and poison rolls use player-owned streams. Both survive
  full/fast snapshots and campaign saves. Isolated NPC wandering, loot scatter and
  poison checks confirm that unrelated draws do not alter replay; solo keeps its
  original shared sequence. Authoritative snapshots now carry a bounded sound
  history; guests play each event once from their own listening position, with
  local effects settings. Prediction is silent, local menus remain audible, and
  synchronization skips historical sounds. Native fixtures cover capture beyond
  the host's hearing range, repeated delivery, missed snapshots, history overflow,
  muting, spectator listening and prediction suppression; Rust rejects malformed
  sound records. The isolated browser lobby/menu/reconnect check passed over
  local TURN with the new schema. Orbiting hazards avoid restarting a sound
  while any party member remains nearby.
  Remote player, actor, spell, map-object and NPC corrections now blend through
  separate render poses over four frames; watched-player corrections also apply
  to a spectator's camera. Identity/model changes, full resynchronization, travel,
  story scenes and large jumps reset offsets. Live records and animation-cache
  owners remain untouched. Native and ASan/UBSan checks cover all five families,
  wrapped angles, repeated corrections, replacement slots and snapshot isolation.
  The browser `--remote-smoothing` check moved the host in the starting passage
  while the guest received 150 ms delayed snapshots and a 500 ms gap. Recovery,
  reconnect and profile reload passed over local TURN; the guest's rendered view
  of the host body was inspected. Browser/native builds pass.
  Projectile impact and blast-child presentation now waits for host confirmation,
  keeps displayed animation/scale from rewinding, and hides retired impacts when
  prediction replays them. Bounded slot histories track generation, owner and
  model; ordinary full snapshots preserve them. Travelling projectiles remain
  predicted, including recovery from incorrect collision predictions. Short
  fixtures cover real Fire Ball/radial updates, retirement, slot reuse, age
  saturation and full/fast snapshot round trips. Short effects entirely missed
  during packet loss are not recovered; continuous hazards still use pose
  correction without one-shot animation history.
  Native and ASan/UBSan suites, generated-binding checks and the WASM build pass.
  The browser host/join, starting-item, TURN, reconnect and profile-reload check
  passes with this schema; its menu assertion excludes character-selection
  replies on the shared action channel. Impact visuals use direct runtime
  fixtures rather than a browser combat/navigation scenario.
- Verify shared story scenes and the
  remaining campaign progression, including the ending.
  Verify personal loot UI, NPC dialogue,
  shops and the complete menu paths with gameplay. Menus and
  focus changes must keep the online world running and bodies vulnerable.
  Inventory, equipment, gold, magic, shop stock and ordinary loot are personal;
  quest progress is shared with once-per-character rewards including later joins.
- Preserve full accidental friendly fire for melee, spells and area attacks,
  with no PvP rewards. Award full enemy XP to admitted connected party members,
  including spectators; keep personal training and original enemy difficulty.
  The starting-area `--combat` check now verifies actual browser-to-native melee
  damage through lethal health loss. It reads authoritative full snapshots through
  Rust's metadata decoder, confirms that the shared world continues, and checks
  that reconnect preserves identity, party slot and the host's death. This covers
  a native host with a browser attacker; spell damage and other gameplay paths
  retain their separate coverage limits. The same live check passed with the
  native host under ASan/UBSan, including its confirmed spectator transition.
  A separate two-browser `--spell-flow` fixture now verifies one actual guest
  Fire Ball cast at the starting entry, without movement. The host's one HP
  reaches zero, the guest spends four MP and stays alive, neither member gains
  XP/gold/training, and world updates continue. Reconnect preserves the party
  and host spectator state. The fixture uses full retail snapshot validation
  under ASan/UBSan; native and sanitizer suites also pass. The initial test's
  epoch-equality assumption was removed because host scheduling barriers can
  advance epochs independently of checkpoint restores. Verification checks the
  actual death and personal state instead. Other live spells and native/browser
  spell combat remain unverified.
  Confirmed death/spectator and revival
  transitions now show brief in-game notices, with persistent browser status for
  spectators. Guest prediction cannot trigger these notices.
- Persistent local profiles now authenticate through the room service. Character
  identities are SHA-256 hashes, carried through Rust's typed boundary and saved
  with the campaign. A fresh guest process/page reclaims the same slot using its
  profile and room code; a saved roster reserves absent players' slots when the
  host creates a new room. Room tests cover reverse join order, wrong credentials
  and a public identity being unusable as its credential. Native and browser
  restart checks passed, including relayed browser traffic. Protocol 15 / world
  schema 11 reject earlier development snapshots. Browser campaign selection now
  uses Rust metadata previews and checks local ownership; the in-game catalogue
  also shows saves from other floors without validating them against the current
  floor's animation assets. Gameplay loading retains full resource validation.
  A two-browser test captured an actual starting-area world as an isolated
  checkpoint fixture, reopened the host from the launcher, and verified that
  the saved guest identity passed safe admission in the new room over TURN.
  Corrupt and other-profile slots were disabled. The separate `--save-flow`
  check now places the real party directly at a retail save point with a dead
  guest, uses the actual save menu, reads the durable IndexedDB checkpoint and
  verifies revival. Rehosting preserves both profiles, HP and personal gold.
  Returning living guests still await a safe entry point, as the current
  admission rule requires; this test checks their preserved state while waiting.
  It does not navigate to the save point or establish native/browser save
  crossplay. Extend safe admission/rejoin coverage.
  Disconnected bodies remain vulnerable for a
  grace period. Host loss waits/resynchronizes or ends the room; no host migration.
- Verify shared-floor travel, variant compatibility, all-living-player entrance
  gates and epoch/load barriers. New characters start at level 1 and spawn at the
  canonical floor entry. Spectators follow the party.
  Party travel now lives with the other gameplay operations; the session owns
  its load barrier. Direct retail round trips reproduced two occupancy defects:
  a spectating host's same-floor warp removed an unregistered body, and a floor
  change registered the host twice. The transition balances the temporary host
  record and relies on the loader's single registration. Travel also clears
  attacks and casting poses. `--party-travel` checks variant and floor round
  trips, personal state, snapshots and the full occupancy grid for both living
  and spectating hosts; native and ASan/UBSan runs pass. The terminal-exit
  regression also checks that prediction cannot travel and that ending handoff
  leaves no temporary occupancy. Native/sanitizer suites, the browser build and
  repository tests pass. The two-browser `--save-flow` scenario also uses the
  guest's actual inventory menu to activate a fixture-provided Green Dragon
  Staff. Both members return to the floor entrance with a new epoch and unchanged
  HP, gold, identities and staff count. Holding the loaded acknowledgment for
  600 ms prevents live state and simulation from advancing; releasing it resumes
  ticks. Rehosting still loads the earlier manual save. A separate two-browser
  `--travel-flow` fixture now starts at the real floor-five entry/gate with actors
  cleared for safe admission. One host step off and back verifies the shared
  entrance condition, floor-five/four travel, a held load acknowledgment, personal
  state and reconnect without retriggering the arrival gate. The
  `--travel-crossplay` extension also passes with two browsers and two native
  sanitizer clients, filling all four slots. All members retain HP, gold, profile
  identity, cell and staff count across that transition; separate browser and
  native guest reconnects preserve the arrived party. Native and ASan/UBSan
  suites pass. This covers that browser-hosted floor-five/four route; other
  live-client routes and native-hosted travel remain unverified.
- Finish host-only manual campaign saves and loading, successful-save/healing
  revival, death spectating, and wipe restoration to the last manual checkpoint
  or campaign start. Preserve solo saves; no autosaves or teammate revival.
  An injected synchronous IndexedDB `put` exception reproduced a false-success
  bug: the empty transaction's later completion overwrote the failure. The
  campaign writer now detaches its callbacks and aborts before reporting that
  error. A callback-only check passes for quota/general failures, an already
  inactive transaction, late events during a retry, successful completion,
  unavailable storage and strict-durability rejection. Linux and WASM rebuilds
  pass. The `--save-failure` gameplay regression reproduced the original defect;
  it has not been rerun after the fix because automated gameplay checks were
  stopped at the user's request.
  The new `--wipe-flow` browser fixture retains floor-five enemies and exercises
  repeated host deaths and checkpoint restoration without navigation. It exposed
  a real WebAssembly stack overflow: the notice renderer's 33 KiB pixel array
  overflowed the 64 KiB stack inside checkpoint continuations and corrupted audio
  state. The bitmap now uses bounded heap storage. With stack guards enabled,
  repeated notices and restoration pass, including restored host HP, position,
  gold and profile identity. This covers the captured checkpoint fixture, not a
  natural campaign playthrough. The normal release browser build also passes
  that regression; native and ASan/UBSan suites and the isolated notice
  rendering/expiration check pass. Diagnostic linker settings were removed.
- KFIII extraction now produces a Rust-validated local character pack with 39
  textured base meshes, canonical archive-slot IDs and an actual content hash.
  Native/browser clients select bodies; host-authorized selection is included
  in snapshots and campaign saves, and remote living players render them with
  separate texture atlases. Native and browser clients now import the US KFIII
  disc through a bounded Rust reader/converter; the browser caches the derived
  pack and selection separately from KF1 resources and saves. Multiplayer
  launch requires character resources. The standing slot-41 body now has authored
  walking, slash, thrust and casting poses driven by replicated movement/attack
  phase and a bounded casting timer, plus KF1 weapon/shield attachments.
  Pose checks cover hand attachment, hit
  timing and floor clearance. A short isolated two-browser scene exercised
  movement and a sword swing over local TURN; native/WASM builds and all three
  native and ASan/UBSan suites passed. The compatibility hash includes the
  presentation recipe. Casting uses the free arm and preserves the weapon pose;
  accepted combat and support casts start it, while failed casts do not. Native
  and sanitizer checks cast Fire Ball directly at the starting entry, round-trip
  its gesture through full/fast snapshots and check expiry after twelve updates.
  Protocol-15 native/browser host/join, menu actions and reconnect checks passed
  over local TURN on retry; the first run timed out in menu input automation.
  The lobby now previews the locally loaded body, with drag/keyboard rotation
  and an indication of whether it has an authored rig or uses its original pose.
  It uses the game's skinning on copied vertices and a separate WebGL context;
  startup releases its GPU resources. Visual inspection identified and corrected
  arm weights that pulled the standing body's tunic hem and left inner hand
  vertices behind. Regression checks preserve the tunic and move the whole hand.
  All 38 previews, pointer/keyboard rotation, context restoration and startup
  cleanup passed in Chromium. Front/side casting captures were inspected. Native
  and sanitizer suites and the short two-browser walking/swing/reconnect check
  also passed with the updated presentation recipe.
  Slot 39 now has its own joints, clothing weights and weapon/shield attachment
  positions, sharing the walking/slash/thrust/casting motion logic. Its long tunic
  remains on the torso. Front and side casting previews were inspected, and the
  two-browser `--pose` check showed walking and sword windup while preserving
  slot 39 through reconnect/profile reload. The launcher obtains rig availability
  from the same C++ definition used by gameplay. Direct sideways/diagonal pose
  checks exposed foot penetration; clearance now checks both sole edges for
  those two rigged bodies.
  Native and ASan/UBSan suites pass with forward, sideways and diagonal foot
  probes, and both client builds pass. The 38-body browser preview check covers
  each rig's availability, casting, base-mesh restoration and context lifecycle.
  Slot 24 now also has walking, slash, thrust and casting poses. Its two arms
  have separate pivots/grips, and waist weights keep its long coat out of arm
  motion. Inspection identified a carried book beside the weapon hand; the
  presentation loader removes that separate 16-triangle component, retaining
  the source pack and using the same curated mesh in previews and gameplay.
  That change used recipe `kf3-standing-pose-v5`. Native and ASan/UBSan suites pass
  for the new body and the existing rigs, including sideways/diagonal sole
  clearance and hand attachments. All 38 browser previews pass; slot 24 has
  exactly 712 remaining triangles, and its front/side casting views were
  inspected. The short two-browser walk/sword-swing check passes over local
  TURN, including reconnect and profile reload. Its walking and windup captures
  were inspected. An initial run hit a CDP context-destruction race at the
  explicitly requested page reload; that polling wait now tolerates navigation
  until the new page is ready, while other evaluation failures remain fatal.
  Slot 14 is now a fourth animated standing body. Its 54-triangle cane is removed
  from the presentation mesh (618 triangles remain), with separate pivots for
  the extended weapon arm and the hanging free arm. The head boundary selects
  all 95 unique vertices of the original head/beard component and excludes the
  raised collar. Walking, slash/thrust attachments, casting, collar separation
  and sole clearance pass the native deformation checks. Linux/WASM builds and
  all 38 browser previews pass; slot 14's front/side casting views were inspected.
  The new `--preview-only` path imports just character resources and starts no
  gameplay or TURN service. No live gameplay check was run for this body.
  That change used presentation recipe `kf3-standing-pose-v6`.
  Slot 22 is now a fifth animated standing body, preserving its hunched posture
  and long coat. Its carried tool is a separate 28-triangle component; removing
  its exact 16 positions leaves 691 body triangles. A bounding box would also
  remove hand and coat faces, so it is not used. The body's forward head pivot,
  arm boundaries, hand attachments and heel clearance have focused native
  deformation checks. Those checks and Linux/WASM builds pass. All 38 local
  browser previews pass; the new body's front/side casting captures were
  inspected, with no game startup or live gameplay checks. That change used
  recipe `kf3-standing-pose-v7`.
  Slot 5 is now a sixth animated standing body. Its originally raised forearm
  is lowered into an authored rest pose before skinning, and the offset mesh
  is recentered over the player. Its separate component was identified as a
  pipe by an isolated geometry view; removing its 80 triangles leaves 729 body
  triangles. Geometry bounds alone include two neighboring hand faces, so its
  atlas bounds also constrain removal. Both hand faces remain. Native checks
  cover inner-hand skinning, coat separation, walking/slash/thrust attachments
  and long-shoe clearance. Linux/WASM builds and all 38 character-only previews
  pass; the new front/side rest and casting views were inspected. No game
  startup or live gameplay check was run. That change used recipe
  `kf3-standing-pose-v8`, owned by the presentation loader.
  Slot 19 is now a seventh animated standing body. Its two clasped forearms are
  opened into a rest pose while retaining all 791 triangles. All 62 unique
  forearm/hand positions were checked against the original connected components
  to assign the fingers crossing the center line to the correct arm. The cuff
  joins share the gradual correction; the waist and skirt stay unchanged.
  Native deformation checks cover inner fingers during attacks, weapon
  attachments, casting beside the dress and forward/sideways sole clearance.
  Linux/WASM builds and all 38 character-only previews pass; the new front/side
  rest and casting views were inspected. No game startup or live gameplay check
  was run. That change used recipe `kf3-standing-pose-v9`.
  The roster audit found Orladin in MOF 545, outside the original NPC archive
  range. Native/browser disc import now includes that archive and appends the
  throne at presentation ID 43; previous IDs are unchanged. This brings the
  pack to 39 meshes, representing 37 of the catalogue's 39 entries plus two
  extra elf meshes. Sick Airon remains unlocated, and Yvette (MO 18) remains
  excluded for the palette issue below. The [catalogue mapping](port-findings.md#kfiii-catalogue-coverage)
  distinguishes these gaps from the count of imported meshes.
  The browser selector now uses the verified catalogue names, with alphabetical
  groups for animated characters and original poses. Group membership comes
  from the actual game rig availability; unnamed extra elf meshes retain neutral
  labels. Selection/cache/network IDs remain unchanged, and the preview's
  accessible label follows the selected name.
  Both client builds, Rust snapshot bounds and C++ character-selection checks
  pass. Native BIN/ISO and browser disc conversion match the independent
  17,574,076-byte Python pack. All 39 character-only previews pass; the throne's
  front/side views were inspected. The flake packaging and binding checks pass.
  Gameplay remains untested for this addition.
  Slot 26 (John Creel) is now an eighth animated standing body. Its crossed
  hands are separated into a rest pose with all 620 triangles retained.
  Curation was checked against each original connected component: the vest,
  chest ornament, head, trousers and shoes remain unchanged. All 46 unique
  hand positions stay assigned to their original arm. Arm atlas bounds and
  height separate them from the clothing; only the elbow joins blend.
  Linux/WASM builds and focused checks pass for hand attachments, complete
  finger movement, vest separation and forward/sideways shoe clearance.
  All 39 character-only previews pass; slot 26's standing, side and casting
  views were inspected. No gameplay scenario was started.
  Slot 1 (Lyn Reinhardt) is now the ninth animated body. Its crossed forearms
  open into a rest pose with all 1,106 triangles retained. Comparing the native
  curated mesh with its original connected components confirms that the torso,
  head, long clothing, feet and earrings are unchanged by curation. Complete
  hands rotate together; the elbow joins blend into the upper arms. Textured
  casting review exposed inward elbow vertices missing from the skinning mask;
  the corrected mask includes them without pulling the waist or cloth.
  Linux/WASM builds and runtime checks pass for casting, elbow/thumb movement,
  weapon attachments, clothing separation and forward/sideways foot clearance.
  All 39 isolated character previews pass; slot 1's idle and casting poses were
  inspected from the front and side. No gameplay was started for this addition.
  The lobby now exposes Rest, Walk, Step sideways, Swing, Thrust and Cast poses
  with a phase slider, using the same skinning on a mesh copy. Controls appear
  only for rigged bodies, preserve the selected phase through WebGL context
  restoration, and keep a fixed camera while scrubbing. The broader visual
  preview exposed slot 1's fixed legs beneath moving feet: its leg mask now
  moves complete skin/anklet/shoe surfaces while keeping the long cloth panels
  and hip scabbard on the torso. Runtime checks cover that separation as well
  as foot clearance. Both builds pass; character-only browser checks exercise
  all nine rigs through the pose selector and phase slider. No gameplay starts.
  Slot 27 (Olivier Veyrac) is now the tenth animated body. Curation removes
  exactly the separate 74-triangle sword, retaining all 922 body triangles.
  All 26 unique positions in each hand receive one rotation; native component
  comparison confirms that only arms change and the torso, head, collar, legs
  and feet retain their source geometry. The rig separates the raised collar
  from the lower face and waist plates from thigh motion, and accounts for the
  splayed feet when keeping soles above the floor. Jamie Porter's mesh has a
  different bent-leg stance and cannot reuse this rig unchanged.
  Linux/WASM builds and runtime checks pass for complete hands, equipment,
  collar separation and sole clearance. All 39 character previews pass with
  pose/phase controls on ten rigs. Olivier's idle, walking, sideways, swing and
  casting views were inspected without starting gameplay.
  The current presentation recipe is `kf3-standing-pose-v13`, so its
  compatibility hash differs from earlier development versions.
  Casting still needs visual review in multiplayer. Finish living-humanoid
  curation, rigs and corrections for other meshes. One texture remains
  unresolved. The opening-program investigation found five valid TIM palette
  rectangles in `OP/OP.D`; none supplies slot 18's `(96,500)` palette. Its four
  affected triangles are a small neck ornament with nonzero texture indices.
  Reproducible addresses, hashes and rectangle extents are recorded in
  [findings](port-findings.md#kfiii-character-palette-still-unresolved).
  Runtime VRAM/material evidence is still needed; no substitute palette was
  added. No retail assets enter source control.
- Native and browser startup now hash the selected resource tree before room
  admission and campaign loading, using the same sorted path/length/content
  format as browser disc verification. Native clients no longer claim the retail
  identity for modified files. Hashing streams file contents with bounded tree
  depth, file count and total bytes; it rejects links and nonregular files.
  Focused checks cover canonical byte order, empty files, same-size edits,
  renamed paths, oversized trees and recovery after verification failure.
  Isolated real native clients passed admission, reconnect and profile restart;
  a guest with a one-byte resource change was rejected before world synchronization.
  The browser import/host/join/reconnect test passed over local TURN and checked
  that both room requests carried the independently verified retail digest.
  The `--crossplay` browser scenario now runs actual native and browser games
  in both host directions with matching Japanese resources and isolated profiles.
  A native guest joined and reconnected to a browser host; a browser guest joined
  a native host, selected an avatar, used the starting herb through the menu while
  snapshots continued, and reconnected. Browser candidate statistics confirmed
  TURN relay use in both directions, and the rendered native-hosted view was
  inspected. This startup/menu scenario does not establish crossplay travel,
  saves or combat; the separate melee scenario above covers lethal friendly fire.
  Finish deployment and
  TURN verification. Validate real native/browser gameplay, reconnect, travel,
  saves and combat, not only transport packets or world startup.

| Item | Next useful step |
| --- | --- |
| Natural ending and re-entry | Use a suitable save or user-reached ending to verify the original transition and starting another game without stale state. |
| MAGIC bar once stayed empty | Deferred at the user's request until a reproduction or new failing-state evidence arrives. No demonstrated defect; do not reset charge speculatively. |
| Rendering fidelity | Compare equivalent retail inputs/frames; remaining differences include launch-model modulation, GL coverage/interpolation and presentation chronology. |
| Collision result types | Audit producers/consumers before refactoring `map_object_probe_door_closing`; high-word kinds, rejection flags, all-ones and literal-one results are not one uniform bitset. |
| Resource boundaries | Carry end pointers through unchecked traversal; OPEN packet unions, animation metadata and upstream projection lifetimes still need separate work. |

The scheduled GAME TMD enqueuer conversion is complete. C-style casts and other
legacy views remain elsewhere; choose any next family explicitly and preserve
its original behavior. English support and the in-game language switch are in
PR #34, separate from the current `port` branch.

Work on the current user-selected task. Keep verification claims bounded to what
was exercised, use isolated diagnostic state, and do not restart or steer the
user's session without authorization. Update this file when current status changes;
commit history records the implementation sequence.
