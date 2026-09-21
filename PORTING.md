# Direct source port to Linux and WASM

The [prioritized remaining-work list](docs/port-status.md) tracks current issues,
next actions and completion criteria. This document retains the implementation
contract and detailed progress; it is not the day-to-day work queue.

## Contract

The original source owns movement, AI, combat, animation, inventory, menus,
scripts, progression and transitions. Edit it in place only where portability
requires it. Neither a second gameplay runtime nor a replacement Psy-Q SDK is
the product. Preserve gameplay calculations and control flow, allowing focused
pointer-width, memory-ownership and genuine undefined-behavior fixes.

Use plain structures, functions and scoped enums. Keep existing nested loops;
WASM yields through Asyncify. One executable/app calls opening, game and ending
routines through an enum-driven coordinator. Namespaces separate original modules.
Restore module-owned initialized data as well as BSS on re-entry.

### Types and casts

Remove C-style casts by correcting types and boundaries, not by mechanically
renaming every cast. Eliminate redundant conversions, share accurate declarations,
and separate independent runtime concepts even when the resource packs them into
one byte. Scoped enums model alternatives; an enum around a packed byte does not
validate it or separate a count from flags.

Use explicit numeric conversions where narrowing, signedness or wrapping is
intentional, preserving the original calculation and its conversion points.
Decode serialized bytes with explicit widths, endianness and bounds before
constructing runtime objects. Confine necessary pointer reinterpretation to real
binary/API boundaries with valid alignment, object lifetime and extent; a named
cast does not make an invalid access safe. Do not cast away constness to conceal
an incorrect interface. This remains plain structs/functions/enums, without an
inheritance hierarchy or a replacement gameplay implementation.

This is the cleanup policy, not a claim that existing C-style casts are gone.
Potential reconstruction corrections follow the separate evidence gates in
[docs/port-findings.md](docs/port-findings.md); port safety changes are not
automatically appropriate for the decomp branch.

## Direct boundaries

### Files and extraction

The launcher accepts the user's Japanese ISO or BIN/CUE. A separate local extractor
produces ordinary files before gameplay, retaining contents and directory structure,
including root system screens. Linux uses `--data`; WASM mounts the same tree in
its filesystem and caches locally. No upload and no new gameplay asset format.

Replace CD commands, sector records, retries and polling with paths, lengths and
bounded file reads. Preserve resource ownership and arena lifetimes. Existing
loaders interpret COM/MIX/TMD/MIM/TIM/SEQ/VAB; decode packed structures there only
when their runtime representation changes.

### Rendering

Keep existing visible-cell/entity, animation, HUD, menu, lighting and effect
decisions. Convert those producers to native DrawFace commands with copied
vertices, material, depth and blend information. Preserve whole-face sorting and
equal-depth insertion order; triangulate afterwards. Never retain reused morph
scratch pointers. Replace GTE register state with explicit transforms, projection
and lighting; retain fixed-point gameplay math without emulating a GTE.

Presentation and menus submit directly to the GLES3/WebGL2 renderer. Support
retained frame contents, clipping, clear policy and texture/palette updates.
Decode STAT.DAT menu graphics into native UI descriptions at loading time; runtime
UI objects must not remain GPU packets.

The port owns the GLSL ES 3.00 vertex and fragment shaders in
`src/renderer/renderer.cpp`, shared by Linux GLES3 and browser WebGL2. They map
screen-space vertices to clip space, modulate textures and apply RGB5 writeback,
dithering and integer transparency blending. Raw/modulated texture mode and
flat/Gouraud shading are explicit native face properties. Transform, projection
and lighting calculations currently run on the
CPU; shader ownership does not imply that these calculations have moved to the
GPU. These shaders are port infrastructure, not reconstructed PS1 source.

Retail's visibility and ordering behavior is the target. Original producers
test projected-coordinate winding and supply the same screen coordinates to the
native face queue. After whole-face ordering and the original quad split, the
backend rejects each triangle whose screen-coordinate span exceeds 1023 in X or
511 in Y, before viewport clipping. This restores a rasterizer rule omitted from
the port; it does not introduce a near plane or change authored depth biases.
The initial camera-space culling and near-plane correction has been removed.
The native projection now evaluates the documented integer reciprocal, screen
rounding/saturation and original signed fog coefficient directly. It has no
implicit register, command or FIFO interface. Retaining native commands and
correcting these arithmetic rules does not establish exact rendering. Native
lighting now retains the integer matrix, saturation and fog interpolation stages.
Blended triangles read a clipped snapshot of prior framebuffer contents rather
than the attached render target. Snapshot performance is unmeasured; modulation
currently follows later GPU arithmetic, not the launch-model truncation.
Complete inputs, interpolation, coverage and frame history still need verification
against retail. Opaque geometry still uses painter ordering without a depth
buffer. Check remaining occlusion against retail before attributing it to
an inherent painter-order limitation.
The stage-by-stage evidence and remaining gaps are indexed in
[the rendering model coverage notes](docs/port-findings.md#rendering-model-coverage).

### Timing and input

Use one shared clock and absolute deadlines, retaining three-VBlank gameplay
cadence without adding a second full gameplay interval after presentation.
Replace input-release/event busy loops with yielding waits. Linux pumps SDL;
WASM suspends via Asyncify. Remove BIOS interrupts/events, not emulate them.

The user selected stable, retail-informed update rates with the original per-step
logic intact, rather than reproducing workload-dependent console slowdowns.
Choose rates at the owning loop, not through a global slowdown or changes to
movement/animation increments. Explicit clock-based holds and audio sample timing
remain separate. Opening scene0 now has a 22-update/second deadline, rounded from
its measured retail-executable/emulator path duration; this is a port policy, not
a newly discovered retail constant. GAME world rendering now owns the existing
three-tick deadline, including scripted loops and brief extra world redraws;
callers do not wait a second gameplay interval. Applying that cadence to formerly
unpaced scripts is a port policy, not recovered retail timing. Other OPEN loops,
menu presentation and explicit holds retain their own waits. See the
[timing boundaries and current scope](docs/port-findings.md#stable-update-policy-and-loop-boundaries).

Original button-driven logic consumes keyboard/controller actions. Mouse look
changes view angles, not movement logic. Browser callbacks service platform work
and collect input; never recursively enter gameplay.

### Memory and numeric behavior

Allocate useful resource arenas normally and retain meaningful rewind lifetimes.
Remove fixed RAM, SDK heap switching and pointer truncation. Use native pointers
and allocation sizes for runtime objects, fixed-width fields for serialized data.
Replace hardcoded copy extents with typed operations. Preserve gameplay rounding,
overflow intent and random-number behavior, not host rand(). Fix concrete defects
locally, including the uninitialized animation keyframe ordinal.
The return-home and player-homing warnings also have retail-side uninitialized
inputs. The user approved using the actor's actual home and the player's
horizontal distance; both original port functions now compute those values
before use. These are deliberate behavior corrections, not missing-source
restorations. See the
[approved input corrections](docs/port-findings.md#approved-port-corrections).

### Audio

Retain original sound selection, scheduling, spatial and volume decisions. Replace
SDK calls with bank, voice and sequence operations backed by existing decoders
and a software mixer feeding SDL. Backend data outlives resource-arena rewinds.
Browser audio starts from a gesture. No silent-success audio placeholders.

### Saving

Retain save-point rules, slot selection, summaries and interaction. Replace card
events, formatting and sector plumbing with save files. Serialize explicit
versioned fixed-width fields, never pointers or native-structure dumps. Rebuild
transient references on load. Native and IndexedDB writes must be durable before
reporting success. PS1 memory-card interchange is not required.

## Implementation order and current state

1. Direct interfaces/build ownership: CMake builds the original game/opening
   modules and one platform entrypoint. The parallel gameplay rewrite, preview
   launcher and their test harnesses are removed, not merely excluded. Retained
   extraction/input utilities live under platform, drawing under renderer, and
   shared file-codec declarations under lib. The temporary Psy-Q declaration
   scaffold and old overlay mains/fixed-address startup are removed. The value-only
   vector value types retain binary layouts and explicit width conversion in geometry_types.h; no SDK
   runtime is implemented. build.json now lists portable source modules, and CMake
   watches it for regeneration. Shared implementations own separate translation
   units under `src/lib`, compiled in the game/opening namespaces where needed.
   Entry/reset coordination lives in each module's `entry.cpp`; save serialization
   and numeric tables live in their single owning source files. No source `.inc`
   fragments remain. The retired PlayStation build.py, linker
   helpers and overlay-bounds assembly are also removed from this worktree.
2. Files/memory: original game/opening resource loaders now read ordinary files.
   Item models use paths/lengths; all existing-buffer reads supply capacities.
   CD commands/polling are gone from compiled sources. Root screens read root
   files directly. Allocated required-load failures stop before parsing stale
   data. TIM loading receives exact lengths and uses the existing bounded codec.
   The arena uses native allocation sizes and typed allocation records. Heap
   allocations are released on reset; module exits release their arena, and game
   exit releases animation caches first. Old SDK heap-reset calls are removed.
   Initial actor/object/event clearing uses native object extents. Owner-local
   re-entry functions capture complete trivially-copyable globals before first
   execution and restore those initial values on subsequent entries. This is an
   in-process snapshot, not a serialized PS1 image; native pointers retain their
   native representation. Arena owners use explicit cleanup instead of snapshots.
   The launcher now extracts ISO2048 or original single-track MODE2/2352 BIN/CUE
   into a new ordinary directory before gameplay. It preserves all 428 original
   files, including KF paths, root screens and executables, and verifies the
   complete file-tree identity before publishing files. Existing directories are
   never replaced. Write failures leave reported partial output; extraction is
   not crash-atomic or power-loss durable. Browser extraction uses the same code,
   mounts ordinary files in MEMFS, and caches per-path bytes in a dedicated
   IndexedDB store. Cached trees are verified before mounting; old flattened
   archive caches are no longer linked or used by the product.
3. Timing/input/coordinator: original waits now use one absolute 60 Hz host clock;
   gameplay retains its three-tick deadline. Empty button loops yield explicitly.
   SDL supplies held-button snapshots, keyboard/controller mappings and direct
   mouse view-angle deltas. Menus and blocking scripts delimit mouse input; focus
   loss suspends the clock. Browser waits use Asyncify and capture checks use
   actual DOM pointer lock. An enum-driven coordinator calls original opening,
   game and ending routines. Re-entry restores module-owned state, including
   initialized paths, palettes and scroll state. Shared clock, RNG, persistent
   saves and backend ownership stay outside those resets. Newly introduced
   mutable globals must receive owner-local reset coverage. BIOS memory-card
   events are now removed from compiled sources.
4. Rendering: TIM uploads now populate renderer-owned texels/palettes directly,
   with decoded texture-cache invalidation on updates; no LoadImage implementation.
   Native face submission orders whole faces by depth and equal-depth LIFO before
   triangulation. Retained frame contents, clip rectangles and clear policy are
   supported. Original system/dialogue screen producers now submit native faces;
   modal resize/expose re-presents the retained target without blending again.
   Original game/opening model, map, billboard, HUD, panel and gradient producers
   now copy faces into one host-owned frame queue. General and menu presentation
   use that queue directly; ending background fades update native frame settings.
   TMD vertex/normal indices remain file element indices, without byte-offset
   preparation; face winding is explicit math. Lighting is evaluated from explicit
   per-draw matrices and module-owned color/ambient/fog values. Projection and
   model transforms now take explicit parameters, preserving weapon projection,
   compass transform-only rendering, and original depth/fog inputs. Translation
   placement writes only the three owned fields. Display initialization/cleanup
   uses native clear/retain policy; the unused hardware-only VRAM viewer is gone.
   Fixed-point matrix composition, scaling and vector application are direct,
   shared operations preserving rounding, saturation, wrap and aliasing. Original
   animation-cache morphs use bounded native delta arithmetic, without processing
   a dummy vertex; full-width vertex counts are checked before cache allocation.
   Sine/cosine retain the original quarter-wave samples and mirrored endpoints;
   XYZ matrix construction retains its distinct convention and rounding points.
   Arctangent retains the original CORDIC steps; square root retains its sampled
   approximation for nonnegative signed inputs, with a safe unsigned extension
   for high-bit inputs that previously indexed outside the table. RNG uses one
   application-wide BIOS-formula stream; seed one is a portable startup policy,
   not a claim about the original BIOS boot seed. STAT.DAT templates, sprite
   materials, text and prices are decoded at a bounded little-endian boundary;
   original menu producers now enqueue copied native faces. Authored template
   banks, glyph marks and insertion order remain intact. Text and number drawing
   stop at each row's capacity even without a terminator. Runtime-selected HUD,
   floor-item, actor-override and opening/ending materials are native descriptions.
   Polygon setup and texture-selector SDK declarations are removed. Unused
   primitive-buffer/ordering-table storage is gone; menu TIM reads use bounded
   asset scratch. Both full application builds linked at this stage.
5. Audio: original bank, sequence, fade, spatial and note callers now use a direct
   SDL software mixer. Bounded VAB/SEQ decoding retains compressed samples and
   events in backend-owned memory across arena rewinds. Each voice keeps ADPCM
   predictor history across loops. Authored envelopes, program/tone selection,
   pitch, serial pan and quadratic voice levels are evaluated natively; CC7 is
   captured at note-on and note-off/bend use sequence/program identity. There is
   no audio SDK implementation or remaining audio SDK declaration scaffold.
   The main thread renders elapsed shared-clock samples before sound commands,
   so short menu notes cannot disappear between device callbacks. Focus pause
   freezes state and retains queued audio. Native policies are linear sample
   interpolation, a comb/diffuser reverb, sample-clock/raw-tempo sequencing,
   floating-point gain calculations and about 93 ms of playback headroom. After
   starvation, elapsed state advances but only recent samples enter playback.
   These are not claims of bit-exact original DSP or tick scheduling. The examined
   supplied key-on/flush paths do not consume automatic vibrato/portamento fields;
   no new modulation is inferred from their presence in bank metadata.
   The launcher creates SDL/browser audio within the Play gesture before awaiting
   save-storage initialization. Retail pitch-reference notes can exceed the MIDI
   note-on range; the loader now accepts the authored B1 center note of 150.
   Saving: original slot menus now call ordinary per-slot storage. Version-one
   little-endian serialization names every player scalar, excludes native pointers,
   retains the original world byte streams, and includes stock/learned magic.
   A magic/version/length/checksum header and bounded decoding precede application.
   Equipment pointers are reconstructed; the original load-return path reloads
   floor/weapon/magic. Recreated floor effects retain their new pool references.
   Native writes use a unique temporary file, file fsync, atomic rename and
   directory fsync; default preference-directory ancestors are also synchronized.
   Browser writes copy bytes into dedicated strict IndexedDB transactions and
   await completion. Per-slot damaged/unavailable state does not disable healthy
   slots or explicit replacement. Card events/formatting/headers are gone; the
   original return label occupies the removed format row. Saved denominators,
   dialogue stages, object identities, links and drop coordinates are checked.
   The original world writer is bounded; restoration no longer forms a pointer
   before its floor array. Native save/load has been exercised through the original
   save-point and system menus, including a fresh process. Browser slot-two saving
   and loading through the original menus have been exercised across a page reload;
   its launcher requests persistent storage and explains eviction risk.
6. Run the actual game and fix encountered failures. Obsolete parallel gameplay,
   preview launcher/tests and SDK declaration scaffolding have now been removed.
   Renderer triangle commands are backend-private; original producers submit
   whole faces, sorted before triangulation. Reusable low-level TIM decoding,
   input collection and extraction utilities remain; this is not a claim that
   every unused utility has been eliminated.

Actors, combat and scripts enter together through the existing game loop; they
are not separate gameplay reimplementation milestones. Preserve unrelated edits
and keep work in the port worktree. After each step, obtain both a reviewer and a
holistic review, apply their findings, and continue.

## Verification and completion

No new unit-test campaign, parity framework, matching work or replacement-gameplay
tests. Build and link Linux/WASM, then briefly exercise actual opening, gameplay,
menus, combat, sound and save/load. Compiling objects, a map preview or empty
platform functions is not completion.

For interactive gameplay checks, ask the user to exercise the client. Do not
manually steer diagnostic clients with timed key presses. Automated gameplay
checks would require a deterministic input recording/replay mode; no such mode
is currently implemented or added to this scope. Earlier timed-input captures
remain bounded observations, not deterministic replay or retail-frame parity.

Linux and WASM original modules/support/host libraries compile after the converted
file, memory, input/timing, render and audio boundaries. Both reviewers checked resource
lifetimes, pacing, input, copied faces, lighting, projection, display retention,
fixed-point matrix/trig/morph operations, native menus/materials and removal of
obsolete packet/ordering-table storage. Animation count-guard and menu
glyph-bound findings were fixed and re-reviewed. Audio review caught and corrected
short-note callback timing, starvation backlog, sequence controls and gain/pan
issues. Save review corrected first-save directory durability, non-regular-file
blocking, damaged-slot isolation and restored-index/denominator validation.
Linux and WASM applications both link. Native opening and idle gameplay each ran
for 20 seconds under sanitizers with dummy SDL audio; this does not establish
audio playback correctness. The original starting room renders. Actual movement
exposed a floor-sprite initial-frame bug: the facing nibble was mistakenly part
of the random frame count, and is now excluded. Startup fixes initialize armor
IDs before equipment recalculation, avoid magic-table pointers for non-magic
effects, preserve wrapped occupancy checks without out-of-array pointers, and
handle the original null current-actor reset. Floor-item facing and count are now
separate runtime fields decoded at a bounded loading boundary. That change builds
on Linux/WASM and passed short native opening, movement, menu and sword-swing
diagnostic runs. Both reviewers checked the model and caller changes.

Re-entry builds link on Linux, WASM and the native sanitizer configuration. A
compiler declaration inventory covered all 90 compiled original translation
units: 123 mutable globals use owner-local snapshots and four memory-owner globals
use explicit cleanup; no mutable function-local statics were found. Both reviews
checked coverage and lifetime ordering. An isolated native diagnostic binary
selected the existing intro/skip flags after 60 updates, completing three
game-to-opening-to-game cycles without sanitizer diagnostics. Each observed game
entry restored the initial resource path, audio voice index, transition state
and empty arena; opening input state also reset. The diagnostic selectors are
not part of the shipping sources. This verifies repeated entry and cleanup, not
normal story triggers or ending completion.

After obsolete-code cleanup, Linux, WASM and the sanitizer client link, and the
pinned native package check passes. Both reviews confirmed that the ordered 90
original translation units, copied-face sorting, texture transparency, retained
presentation and input/extractor ownership were preserved. Review found and
removed the stale PlayStation build script and its obsolete manifest consumers.
An isolated sanitizer run rendered the opening, skipped to gameplay and loaded
the existing native save through the original menus, rendering the saved cross
without sanitizer diagnostics. This does not extend the browser save/audio or
combat claims below.

Native BIN/CUE and an ISO payload view of the supplied disc each produced 428
files byte-for-byte identical to the existing extracted tree. The native game
starts from that new tree. Chromium also imported the disc locally, rendered the
original starting room and root menu, and reloaded the verified per-file cache.
The browser also imported BIN/CUE and ISO, ran the opening, skipped into gameplay,
and reloaded the cache. Its audio context was running with an advancing clock;
this does not verify audible playback. Both reviews checked extraction and the
launcher; terminal-abort controls, missing persistence warnings and asynchronous
storage failure reporting were corrected. Browser audible playback remains open.
The user subsequently reported that the Linux
client's music and menu/attack sounds play normally. That listening check covers
those native sounds, not every bank, effect or browser playback.
The user also confirmed that sword damage, enemy defeat and damage to the player
worked in an earlier Linux run. This is a user-observed brief combat check, not
an automated replay or verification of every enemy and combat mechanic.

A separate native sanitizer client reached the floor-one save cross by normal
movement and door interaction. Its only diagnostic addition logged player and
save-point positions; it did not replace movement or invoke save functions directly.
The original slot/confirmation menu wrote an 8,964-byte slot-one file in a temporary
save directory. The load menu displayed the saved floor/HP/MP summary and returned
to the cross. A fresh sanitizer client without the diagnostic wrapper loaded that
same file through the original menus and also rendered the saved position. No
sanitizer diagnostics occurred during these interactions. External X-window destruction at the end of
both runs produced an X11 BadWindow error, so these are not clean-exit checks.
This is one native save/load path, not exhaustive slot, world-state or browser
persistence verification. The user's desktop client and saves were not involved.

An isolated Chromium profile imported the original BIN locally and used the native
slot-one save only to set up the save-point position. The original browser load
menu loaded that seed; interaction with the cross and the original save/confirm
menus then wrote a new 8,964-byte slot-two file through the application's storage
path. After a page reload, the verified resource cache was restored and the
original load menu displayed slot two's floor/HP/MP summary and returned to the
saved cross. Stored slot-two bytes were unchanged across reload; menu and position
captures were inspected. No application stop or browser exception was reported.
Persistent-storage permission was denied in this headless profile, and the launcher
displayed its eviction warning. This verifies a browser menu save and page-reload
round trip, not browser-process restart, eviction survival or power-loss durability.
The initial native seed insertion is setup, not evidence of a browser save.

No CD, Pad,
VSync, interrupt-control, TIM/display SDK, implicit transform/projection or
normal-lighting implementation is supplied or required by the converted sources.
Hands-on game verification remains incomplete.

User screenshots and an isolated native reproduction exposed two rendering
defects near the starting-room doors. An initial near-plane correction passed
Linux/WASM/sanitizer builds, a native opening/menu/sword/door run and a browser
save/load/page-reload round trip. It nevertheless changed facing and projection
policy beyond what retail supported and has been superseded, not banked as an
exact-retail correction.

A subsequent static check of GAME.EXE confirms projected-coordinate culling,
full projected depth, wall bias 200, hinged-door bias 15 and head insertion into
the ordering table. The original rasterizer's triangle-size restriction explains
the captured door intrusion: of 24 door triangles, 16 exceed the vertical limit
and the other eight are outside the viewport. The native backend now applies
that restriction and the original projected-coordinate path is restored.
Linux, WASM and sanitizer builds link with the replacement. A separate native
rerun kept the room visible at the reproduced behind-door angle and showed the
opened door correctly from the front. Opening and menus rendered; the original
load menu restored the save cross and the save menu wrote a new temporary slot.
The replacement browser build also completed an original-menu save/load across
a page reload. These were bounded application observations, not deterministic
input replay or retail-frame comparisons; the earlier clipping captures are
not evidence for the replacement. The isolated native client exited normally
without sanitizer diagnostics; the user's desktop client and saves were untouched.
Plaque occlusion remains visible and unresolved. No depth buffer or authored
sort-bias change was added. Neither
reported defect is Z-buffer fighting, since the backend disables depth testing.
See the detailed render findings for evidence and attribution limits.

After that application run, the native projection's floating-point division and
fog approximation were replaced with integer reciprocal refinement, Q16 screen
rounding and the original signed coefficient calculation. Linux, WASM and
sanitizer builds linked (`build/retail-projection-{linux,wasm,sanitize}.log`).
No new application run or game-input replay was performed for this arithmetic
change. A calculation over the already-recorded plaque camera vertices left all
16 screen coordinates unchanged; it does not resolve the plaque discrepancy.

The subsequent lighting correction replaces floating-point color interpolation
with integer matrix stages, intermediate saturation and signed fog inputs.
Original map producers still quantize lighting to a byte color before their
separate per-vertex fog call; combined normal/fog producers keep their combined
calculation. No animation timing, gameplay decisions, shaders or face ordering
changed. Build results and the retail call evidence are recorded in
[the lighting arithmetic findings](docs/port-findings.md#lighting-arithmetic-correction).
This is not a claim that the plaque or reported flame speed is fixed.
The flame report was later closed after the user's Linux/retail comparison
confirmed analogous behavior; no flame-specific slowdown was needed.

The following pixel change adds RGB5 writeback, dithering and integer blending
to the owned shaders, with native raw/modulated and flat/Gouraud metadata.
Linux, WASM and sanitizer builds linked; both actual shaders compiled and linked
in an isolated blank WebGL2 page with no messages or GL error. No game, assets or
draw commands were run for that check. Both reviews checked the arithmetic,
producer metadata, texture ownership and ordered framebuffer snapshots.
Native-driver rendering, visual correctness and snapshot cost remain unverified;
see [the pixel arithmetic findings](docs/port-findings.md#pixel-arithmetic-correction).

Port findings that may warrant reconstruction follow-up are recorded in
[docs/port-findings.md](docs/port-findings.md). They are not automatic decomp
corrections; the floor-item initial-frame issue still needs retail attribution.

The fixed-point sine samples in `src/lib/fixed_math.cpp` come from the supplied
Release 2.5 `LIBGTE.LIB` / `GEO.OBJ` quarter-wave data, also identified in both
original game/opening images. Its 2048-byte little-endian payload SHA-256 is
`74743e361fc4d78cbd41bd99b2acf5c75c9b5b0268ccd753ed282ec4394fb25a`.
Do not regenerate these samples with host trigonometric functions or merge the
XYZ constructor with the game's separate Y-X-Z rotation helper.
