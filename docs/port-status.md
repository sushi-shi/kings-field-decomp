# Remaining port work

This is the prioritized work queue. [PORTING.md](../PORTING.md) is the source-port
contract; [port-findings.md](port-findings.md) holds the detailed evidence and
history. Update this queue when an item's status changes, not just the history.

The common-resource loader preserves its original cross-chunk table copies with
file-bound checks. Fatal native errors remain visible in a dialog. See the
[resource loader regression and checks](resource-loader-regression.md).
The [broader cleanup audit](cleanup-runtime-audit.md) records the lighting fix,
remaining error-handler coverage, cross-version runtime comparisons and limits.

## Priority order

No reported problem remains under active investigation. The user reproduced
the plaque edge in the verified retail-disc emulator session. Flames, Linux
inputs and the general Linux visual check are also user-confirmed.
Browser/ending checks remain verification gaps; MAGIC is deferred until the user
reproduces it. Additional cast cleanup is separate maintenance, not an active bug.

Semantic naming cleanup is recorded in [semantic-naming.md](semantic-naming.md):
game/script identities, menu glyphs, resource fields, rendering/audio scales and
platform/browser encodings now have names at their owners. Original values and
retail quirks are preserved; unresolved meanings remain explicit. Linux/WASM and
native package/launcher builds passed. This does not close the runtime
verification gaps below or the separate compiler-warning/type cleanup.

Source organization cleanup: all 24 `.inc` fragments have been consolidated into
ordinary translation units or their single owning source files. Shared functions
live under `src/lib`; entry/reset coordination lives under `src/game` and
`src/open`. No source fragments or empty `src` directories remain. This refactor
is based on `port`, separate from the concurrent cast-cleanup work. Fresh Linux
package/launcher checks and a WASM build passed. An eight-second no-input native
opening run reached its scheduled timeout without a reported startup failure.

Control-flow cleanup: 68 of 72 `goto` statements and all ten `switch (0)` blocks
have been replaced by focused helpers, guard clauses and ordinary loops. Static
retail CFG checks guided the boundaries; same-tick transitions, random-call order,
common update tails and resource ownership remain part of the original logic.
An empty animation clip now fails explicitly instead of using an uninitialized
keyframe pointer. Fresh Linux/WASM builds linked, with no new compiler warnings.
A clean-context review covered all 20 changed source files, caught an accidentally
removed free-actor-slot guard, and verified its restoration; no findings remain
unresolved. No gameplay run was performed for this cleanup. The four remaining
jumps are all in `map_object_probe_forward`: defer that function's refactor until
the overlapping switch/signedness PRs #14/#15 are resolved. This is port-only
maintenance, not a decomp backport or closure of the runtime gaps below.

Helpers/readability cleanup: domain macros now have typed operations, player
motion/map coordinates/dialogue no longer need runtime word views, and seven
fixed-pool traversals use ordinary forward range loops. Player movement uses
live references and named cell queries; armor periodic effects share a helper
without combining or reordering HP adjustments. The rationale, behavior-sensitive
boundaries and concrete follow-up queue are in
[readability-cleanup.md](readability-cleanup.md). This is source-port maintenance,
not a claim about original source spelling or a new gameplay implementation.
Fresh Linux/WASM builds linked with unchanged compiler diagnostics; a separate
clean-context review found no actionable regressions. No gameplay run was made.

The stacked follow-up passes cover pool searches/copies, player-update helpers,
rendering visibility, typed XYZ operations, explicit state ownership, menu/dialogue
operations and shared camera-path math. Shared stateful operations have ordinary
translation units; no gameplay framework or replacement controller was introduced.
The complete stack links in clean Linux/WASM builds with the same 57 compiler
diagnostics on each target. Collision-result modeling and bounded resource loading
remain separate work; see the cleanup document for their boundaries.

### 1. Update cadence — opening corrected, GAME pacing bypasses closed; flame report resolved

- Evidence: the user reports both running too fast. The normal game loop has a
  three-tick limiter; the opening uses a separate wait path. No missing explicit
  animation divider was found in the inspected original code. A user-authorized
  no-input native fly-through took 8.506 seconds from first camera step to last
  presentation, at about 59.64 presentation calls/second. A second, explicitly
  authorized no-input run of the retail disc in pinned PCSX-Redux measured the
  same camera point 1→15 interval at 22.313 emulated seconds (22.295 host seconds),
  versus 8.224 native seconds: the old port was about 2.71 times faster over that
  interval. The emulator showed variable two-/three-/four-VBlank camera-state
  intervals. This is an emulator/OpenBIOS reference, not original hardware.
  The reference observer clients exited. The user chose stable retail-informed rates,
  preserving original per-step logic instead of reproducing workload slowdowns.
  Scene0 now uses a local 22-update/s deadline (rounded from 21.96 observed), built
  for Linux/WASM. Its authorized no-input Linux rerun measured 22.273082 active
  seconds for point 1→15, about 0.18% shorter than the emulator reference, with
  unchanged 508 camera-step/507 presentation-call counts. It exited successfully;
  focus pauses are excluded from that active-time comparison. Gameplay retains
  its three-tick minimum. A bounded caller trace also identifies render-driven
  flame/notification updates and GAME's floor-5 pacing bypass, addressed below.
  The following title-card phase was measured separately: native first-to-last
  shade markers took 17.751919 active seconds versus 17.747422 emulated seconds,
  with complete fade ramps and a native 16.666046-second 1,000-wait hold. These
  observations support retaining its existing approximately 60 Hz timing; no
  title change is needed. Both no-input title observers exited successfully.
- Implemented next: GAME `render_frame` now performs the existing three-tick
  wait once. Nine caller-side waits were removed so normal gameplay, warps,
  lighting fades and blocking interactions do not wait twice. The formerly
  unpaced floor-5 camera/effect loops and brief extra world redraws now share
  that approximately 20-update/s cadence. Sprite/notification increments,
  visibility rules and script step counts are unchanged. This is a deliberate
  port timing policy for those paths, not a recovered retail floor-5 rate.
  OPEN, menu presentation, retained-frame callbacks and audio timing are unchanged.
  Linux/WASM linked; both reviews are clear. A short no-input starting-room
  observer counted 120 presentations/120 waits (70 player updates), at 20.000054
  wait intervals/s in active time, and exited successfully. This checks startup
  and ordinary pacing, not later cutscenes or subjective flame appearance.
- Flame report closed (2026-09-20): after comparing the Linux client with the
  pinned retail emulator, the user confirmed that the flame flickers analogously
  in both and that this was the original behavior. No flame-specific slowdown
  is needed. This is a user visual confirmation, not an exact timing measurement
  or confirmation of all other animations.
- Next: the user asked to fix known dependencies without making more manual
  scene measurements a prerequisite. The no-input scene0/title checks are
  complete; do not repeat them without a relevant change or failure. During
  ordinary play, check any reported issue with the affected scripted interactions;
  do not reopen the resolved flame report without new evidence. Other OPEN
  scenes retain their own waits; no arbitrary global
  slowdown or unverified animation divider was added.
- Close when: any demonstrated pacing discrepancy is corrected and the user
  checks the affected sequences. Do not apply an arbitrary global slowdown.
- Detail: [flames and other frame-dependent logic](port-findings.md#flame-animation-rate-and-frame-dependent-logic),
  [opening](port-findings.md#opening-cutscene-speed-report),
  [emulator comparison](port-findings.md#pinned-emulator-opening-measurement),
  [update policy and boundaries](port-findings.md#stable-update-policy-and-loop-boundaries),
  [GAME pacing correction](port-findings.md#game-world-cadence-boundary),
  [corrected observation](port-findings.md#corrected-opening-observation),
  [title observation](port-findings.md#title-card-timing-observation).

### 2. Plaque showing through the starting-room wall — reproduced in retail, closed

- Evidence: captured opaque plaque faces draw over the wall. Depth testing is
  disabled, so this is not Z-buffer fighting. The separate behind-camera door
  intrusion has a rasterizer correction and a bounded successful native check.
  A bounded resource/transform calculation now reproduces all 16 captured plaque
  camera vertices and the covering wall's translation/depth; no placement
  discrepancy was found for that pose.
- Closed (2026-09-20): the user confirmed the edge is also visible in the retail
  emulator. The live command used `retail.cue`, referencing the original BIN,
  whose SHA-256 was rechecked against the pinned retail digest. No candidate
  executables, memory patches or input automation were used for that session.
  This establishes the reported effect with retail code in the pinned emulator,
  not original-hardware or pixel-exact equivalence. Keep the original face-order
  behavior; no depth buffer, geometry change or authored-bias adjustment is needed.
- Detail: [occlusion findings](port-findings.md#starting-room-plaque-occlusion-and-behind-camera-doors).
  The [rendering coverage table](port-findings.md#rendering-model-coverage) records
  wider uncertainties; it is not a claim of exact rendering or a new parity project.

### 3. Linux inputs and general rendering — user-verified, closed

- Input closed (2026-09-20): the user confirmed the Linux input fixes are
  verified: fresh-input pause, menu Escape, Caps-to-Esc remapping and mouse
  capture restoration after modal interaction. Do not request these checks again
  without a new regression. Browser pointer-lock permissions remain separate.
- General visual check closed (2026-09-20): the user reports that rendering looks
  fine. No further general Linux visual check is requested without a new issue.
  This is not pixel-exact retail validation or a performance benchmark, and the
  separately reported plaque occlusion was also reproduced in retail (item 2).
- Detail: [input fixes](port-findings.md#pause-menu-escape-and-linux-keyboard-remapping),
  [capture restoration](port-findings.md#restoring-mouse-capture-after-modal-interaction),
  [pixel changes](port-findings.md#pixel-arithmetic-correction).

### 4. Browser audio and persistence — verification gaps

- Evidence: local disc import, opening, game/menu rendering, cache reload and
  original-menu save/load across a page reload have been exercised. Audible
  browser playback and save/cache retention across a full browser restart have
  not. Persistent-storage permission was denied in the checked headless profile.
- Next: ask for a short real-browser check of music/menu sounds and the existing
  save/cache after a normal browser restart. Check mouse capture under browser
  permission rules during that session; a required click is not a native bug.
- Close when: audible playback and restart persistence work, and storage/capture
  permission limitations are visible to the user. Do not promise survival after
  site-data deletion or turn this into a power-loss test campaign.

### 5. Natural ending and application re-entry — verification gap

- Evidence: owner-local resets are implemented and three forced diagnostic
  game/opening cycles passed sanitizers. That did not exercise the natural story
  ending and its return path.
- Next: use a suitable user-provided save or a user-reached ending to check the
  original transition and starting another game. Do not invent a long automated
  walkthrough or treat the diagnostic cycles as proof of story completion.
- Close when: the natural ending/return path and subsequent entry work without
  stale module state. Until a suitable save/session exists, keep this explicitly
  pending and move to the next actionable item.

### 6. Type/cast cleanup — scheduled GAME enqueuer batch closed

- Evidence: C-style casts remain. The floor-sprite facing/count model is fixed in
  the port, but its reconstruction attribution remains unresolved.
- Implemented: GAME's three TMD enqueuers now consume decoded object/face/normal
  values through bounded byte access. Their original 12/4/2 mode cases, material
  selection and rendering arithmetic remain intact. No C-style casts remain in
  `src/game/render_enqueuers.cpp`; this was boundary modelling, not cast renaming.
  Linux/WASM rebuilt and linked, and a brief no-input starting-room observer
  completed without a decoder failure. This is not coverage of every packet mode,
  actor, menu or later map.
- Prerequisite progress: actual file/chunk lengths now reach GAME/OPEN TMD
  registration, including standalone menu models and the separate asset-registry
  route for actors, events, effects and weapons. Slot, registry and current
  selection records carry the pointer and matching extent together. Embedded
  asset sizes/offsets, TMD header/object-table extents and selected vertex arrays
  are checked. Existing resource ownership and arena rewinds are retained.
  Linux/WASM linked; no-input Linux starting-room and scene0 observers exited
  successfully. Both code reviews found no must-fix issue. These are bounded
  resource checks, not proof of full TMD/animation safety or all loading paths.
- Both code reviews are clear; stop after this batch. Additional cast families require
  separate scheduling; this is not whole-repository cast removal. OPEN packet
  unions and other legacy serialized views remain. Animation metadata and
  upstream projection-source lifetimes are not proved safe by enqueuer bounds.
  Candidate decomp backports remain a separate, evidence-gated task, not a
  prerequisite for running the port. Follow the
  [type/cast policy](../PORTING.md#types-and-casts) for any later scheduled family.
- Detail: [TMD resource extents](port-findings.md#tmd-resource-extents-for-the-enqueuer-cleanup),
  [completed GAME decoder batch](port-findings.md#game-enqueuer-byte-decoding-and-cast-removal).

### 7. MAGIC bar once stayed empty — deferred to user reproduction

- Evidence: one user report, no reproduction or captured failing state. Initial
  source inspection found explicit no-selected-spell and Skull Armor refill
  gates; neither has been established as the cause of the report. The bounded
  charge/selection/save/HUD trace is now complete: eligible refill has positive
  gain with the shipped spell data, selection/load-return rebuild the spell
  pointer, and the HUD derives its width from charge each frame. No demonstrated
  port defect was found, and no gameplay fix was made.
- Deferred (2026-09-20): the user asked to ignore this for now and will try to
  reproduce it first. No active investigation or repeated check requests. Resume
  only when the user provides a reproduction or new failing-state evidence;
  do not repeat the static trace, reset charge speculatively or add a framework.
- Close when: evidence explains the state as intended behavior, or a demonstrated
  defect is fixed and checked. Lack of reproduction is not a fix.
- Detail: [MAGIC charge-state trace](port-findings.md#magic-bar-charge-state-trace).

## Already established / working rules

The original game and opening run in one application; extraction/file loading,
native draw commands, audio, saves and module resets are implemented. Linux and
WASM link. The user confirmed native music/menu/attack sounds and a short combat
check. Native save/load worked through the original menus, including a fresh
process. These are bounded checks, not proof that the whole game is finished.

Work on the highest actionable item above; do not open unrelated investigations
while it is active. A new user request can change the order. Record evidence,
next action and closure for each fix, obtain reviewer and holistic review, and
build Linux/WASM after code changes. Ask the user for brief gameplay checks; do
not steer clients with timed inputs or restart their session without permission.
The user has authorized repeated native/retail measurement runs as needed; keep
those no-input unless a deterministic replay or user-driven interaction is
separately agreed, and use isolated saves/settings for diagnostic clients.
No new unit-test campaign, replacement gameplay, Psy-Q layer or emulator project.
