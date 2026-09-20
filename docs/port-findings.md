# Source-port findings requiring reconstruction follow-up

For priorities, next actions and completion criteria, use the
[remaining-work list](port-status.md). This document is the evidence/history,
not the work queue.

These are findings from executing the portable application, not automatic
corrections to the retail reconstruction. Keep fixes in the port until the
relevant image, function, data interpretation and original call contracts have
been checked. An aggregate match percentage is not a verdict on a particular
function; even an exact body does not establish every surrounding data model.

## Match confidence and backport policy

"Almost 100%" is not exact, and aggregate progress does not give the match state
of the implicated function. An exact object establishes the compared machine
output, not a unique source spelling or proof of native C/C++ safety. Different
types can produce the same instructions under the original ABI. Pointer width,
layout, neighboring storage and initialization can change when porting.

The native failure is a reason to investigate, not proof that either the retail
game or the reconstruction is wrong. Keep three possibilities distinct: a
reconstruction mistake, an original unsafe operation exposed on the host, and an
incorrect port assumption. Record which evidence distinguishes them before
attributing a bug.

Carry back independently supported field meanings, widths, declarations and
ownership corrections through the reconstruction workflow, checking callers and
the affected image's exact match. Keep host-only layout changes, defensive checks
and behavior changes in the port unless retail/source evidence supports them.
Do not sacrifice a verified exact function for a plausible native fix, or retain
an unsupported model merely to protect a fuzzy score.

## Cast cleanup is a modelling task

The port policy is in [PORTING.md](../PORTING.md#types-and-casts). Existing source
still contains C-style casts; no repository-wide cleanup is claimed.

- Remove unnecessary casts such as pointer-to-`void` conversions around `memcpy`
  when the declarations already permit the call.
- Fix inconsistent declarations and use typed runtime objects instead of casting
  incompatible views into agreement. Split packed fields at the loading boundary.
- Retain explicit, justified numeric conversion points for narrowing, signedness
  and overflow intent. A `static_cast` is not a range check.
- Decode packed bytes rather than dereferencing cast native-structure pointers.
  Renaming a cast to `reinterpret_cast` does not fix alignment, aliasing, lifetime,
  extent or pointer-width errors. Some API conventions still require an explicit
  reinterpretation, such as OpenGL's buffer-offset parameter.

For decomp follow-up, upstream the supported type/ownership fact, not a blanket
cast rewrite or a host runtime representation. Respect that branch's source
language and pinned compiler; C++ named-cast syntax is not a prescription for
reconstructed C. No cast-cleanup backport or matching campaign has been performed.

### TMD resource extents for the enqueuer cleanup

The scheduled GAME render-enqueuer cast batch first needed real bounds. Previously,
`tmd_register` and the module slot/current-selection fields retained only pointers.
Menu item TMDs are standalone files, while animated assets use a separate registry
that can replace the selected TMD without registering a slot. Inferring a length
from a preceding MIX chunk header would therefore be incorrect.

The port now carries a borrowed `KfTmdResource` pointer/size pair through slots,
registry selection and the current selection. Actual loaded file lengths feed
the MIX/COM chunk views, alternate CHR archive, weapon buffer and menu item loader;
destination capacity is not substituted for the loaded extent. The archive loader
checks its count and each embedded asset's declared `byte_size` and `tmd_data_offset`
against the containing bytes before registering that asset's TMD view. The view's
extent is the remainder of the containing asset, not proof of an independently
delimited TMD subregion.

Registration checks the 12-byte TMD header and complete 28-byte object table.
Object lookup checks its index; selecting an object's vertex array checks its
offset/count and alignment. Header counts/sizes are read as little-endian bytes;
legacy typed serialized consumers still require little-endian targets, now stated
explicitly in the code. Slot release clears its resource record and clears the
selected view/vertices when they refer to that resource. Existing arena ownership,
rewind timing and whole-module resets remain in charge; no resource copies,
alternative gameplay or separate memory manager were introduced.

This prerequisite established extents, not completed packet decoding. At that
stage, primitive/normal streams and individual vector indices still needed bounded
access; the following batch addresses those reads in the three GAME enqueuers.
The prerequisite's GAME/OPEN renderer edits only accessed the selected record's
data member. It did not change dispatch, indices, winding, lighting, sorting or
draw calculations, nor establish animation metadata safety.

Before the loader/accessor edits, hash-initialized GAME/OPEN retail disassembly,
callers/callees, strings, data references, adjacent code, current match reports and
source history were inspected. Existing reconstruction findings for TMD headers
and asset animation layouts supplied the format model. These host extent checks
are not a newly matched function or an automatic decomp backport. Ignored evidence
is retained in `build/tmd-boundary-{game,open}-*.txt`.

Linux and WASM rebuilt the original sources and linked successfully
(`build/tmd-boundary-{linux,wasm}.log`). Separate no-input Linux observers reused
the shipping objects and closed only their diagnostic clients, with isolated saves:

- GAME reached 120 original presentation calls, including its initial entry
  effect, with two populated TMD slots and 33 populated model-registry records.
  The selected TMD extent was 274,592 bytes (`build/tmd-boundary-game-run.log`).
- OPEN scene0 returned naturally after 508 camera steps and 507 presentations,
  with final input action zero (`build/tmd-boundary-opening-run.log`). Point 1→15
  took 22.272821 active seconds. Active time excludes focus pauses; this resource
  check does not replace or overwrite the earlier timing reference.

Both processes exited successfully. These checks do not exercise all weapon,
menu, alternate-CHR or later opening/ending paths, and observer shutdown is not a
normal application-exit/re-entry check. Reviewer and holistic code reviews found
no must-fix issue. No new unit-test campaign or synthetic gameplay input was used.

### GAME enqueuer byte decoding and cast removal

The finite follow-up converts `render_enqueue_tmd`, `render_enqueue_model` and
`render_enqueue_map` in place. Their object records are now decoded values, with
checked byte addressing shared with the legacy object accessor. A borrowed packet
cursor checks the declared count, four-byte header and `input_length * 4` body
extent. It advances by that original stride even for modes the caller ignores.
Each original accepted case decodes a plain face record containing indices,
UVs, texture page/palette and color, rather than reading an inactive packet-union
member or dereferencing a cast into serialized storage.

The decoder distinguishes all eight base polygon layouts. Flat faces have one
normal followed by vertex indices; Gouraud faces have interleaved normal/vertex
pairs. Texture UV/control words precede the indices on textured faces; the
untextured prefix is RGB/code. Fields are little-endian byte reads after a body
length check. Vertex indices are checked against both the object's vertex count
and the native projected-array capacity. Normal arrays are checked against their
containing resource and each consumed normal index against that array; signed
components are constructed by exact-width bit interpretation, not pointer casts.

The existing mode switches remain authoritative: 12 cases in the general TMD
enqueuer, four in the material-overridden model enqueuer, and two in the map
enqueuer. Unsupported modes still skip their packet. All winding, vertex order,
flat/Gouraud selection, material overrides, semitransparency, per-case fog inputs,
depth arithmetic/biases and submission order are retained. No gameplay logic,
resource ownership or persistent state is added. No C-style casts remain in
`src/game/render_enqueuers.cpp`; legacy packet unions remain for OPEN consumers.

The static GAME evidence snapshots are
`build/tmd-enqueuer-game-render_enqueue_{tmd,model,map}.txt`. The inspected existing
match reports say 100% for all three reconstruction functions; this port change
is not a new matching claim or backport. Each function's verdict is a direct
bounded-read adaptation with its original rendering decisions retained.

Linux/WASM rebuilt and linked (`build/tmd-enqueuer-{linux,wasm}.log` and the final
shared-accessor rebuilds `build/tmd-enqueuer-final-{linux,wasm}.log`). The existing
isolated, no-input Linux GAME observer was rebuilt against those shipping objects
and exited successfully after 120 original presentation calls, including the entry
effect (`build/tmd-enqueuer-game-run.log`). It reported two populated slots and 33
model records. That catches a starting-room decoder failure; it does not exercise
every mode/model, user interaction or natural shutdown/re-entry. No new unit-test
campaign or synthetic input was introduced. Reviewer and holistic reviews found
no must-fix code issue, including the final direct-byte object accessor.

This closes only the scheduled GAME family. It does not establish complete
animation metadata safety, upstream projection-source lifetime safety, OPEN
packet decoding or full rendering equivalence, and does not schedule another
cast family implicitly.

## Floor-item initial animation frame includes facing bits

Status: observed native failure, port safety fix applied; retail attribution
unresolved. Candidate for decomp investigation, not an approved backport.

### Source and runtime evidence

The port base commit `b5f0e4a6f87facebb6cc268a3295d349fcab781a` already contains
this expression in `src/game/item.cpp:item_load_floor_placements`:

```cpp
(rand() * kf_enum_encode<u8>(item->facing_and_frame_count))
    >> KF_FLOOR_ITEM_INITIAL_FRAME_RANDOM_BITS
```

The current shared loader is `src/lib/floor_item_load.inc`. Before the fix,
its equivalent expression used `kf::random_next()` but still multiplied by the
entire packed byte. Thus the unmasked expression predates the shared port loader;
this does not establish that it accurately reconstructs the retail function.

The resource model assigns the high nibble to facing and the low nibble to frame
count. Before the runtime field split, rendering extracted the facing nibble and
frame advancement already masked the count with `0x0f`. The seven-entry
`floor_item_sprites` table contains
four frames starting at index 0 and three starting at index 4.

Inspection of the original extracted `KF/B1/MIXA.DAT` floor-item chunk found:

| Base sprite | Appearance byte | Number of placements |
| --- | --- | --- |
| 4 | 0x23 | 1 |
| 4 | 0x13 | 1 |
| 0 | 0x04 | 20 |

For a 15-bit random result, multiplying by `0x23` selects initial offsets
0–34, whereas the frame-count interpretation requires 0–2. Adding base index 4
can therefore exceed the seven-entry table. Other inspected B2–B5 floor-item
placements use base 0 / appearance 0x04, where the distinction is hidden.

During a native ASan/UBSan run, moving forward from the starting room caused a
global-buffer-overflow in `render_enqueue_sprite`, reached through
`render_floor_item` and the original game loop. The reported access was four
bytes after `floor_item_sprites`. The generated diagnostic is
`build/native-gameplay-run.log`; generated logs/screenshots are not committed.

### Port correction and modelling follow-up

The initial safety fix masked the initial frame count to the low nibble. It kept
the same random call count, initial-frame scaling and subsequent advancement.
A later actual-app run exercised movement, the original root menu and a sword
attack without a sanitizer report (`build/native-gameplay2-run.log`). This is
bounded execution evidence, not complete gameplay verification.

The former `KfFloorItemAppearance` named the packed representation but permitted
converting all its bits to an integer. That was not sufficient runtime modelling.
The port now gives `KfFloorItem` separate `facing` and
`frame_count` fields, decoded once by the shared placement loader. The loader
checks placement extent, capacity, sprite range and map-cell coordinates. Plain
structures and a facing enum suffice. The split builds and links on Linux/WASM.
A subsequent native diagnostic run exercised movement, the root menu and a sword
swing without a sanitizer report (`build/native-floor-model.log`); the opening
also ran for 12 seconds (`build/native-floor-model-opening.log`). Both source
reviews found no remaining issue in this bounded change. These checks do not
establish that all surrounding MIX resource parsing is bounded or attribute the
original unmasked expression to retail.

### Before changing the decomp branch

- Establish the exact match state of the GAME loader function and independently
  inspect the OPEN counterpart; the shared port helper does not prove identical
  original implementation or ownership.
- Check retail instructions for field width, nibble masking, multiplication,
  shift and store, together with the placement bytes and sprite-table extent.
- Check the original random-call contract and initialization. The port uses a
  deliberate seed-one startup policy, not a proved retail boot seed.
- Determine whether this is a reconstruction/model error, a retail out-of-table
  read exposed by native memory rules, or another surrounding-state assumption.
  None of these explanations is established by the native crash alone.
- If the function and surrounding interpretation are exact, keep the safety
  change port-only unless source evidence justifies a decomp correction.
  Do not trade away a verified exact body merely to make native execution safe.

No retail-function comparison or decomp-branch edit was performed for this finding.

## Starting-room plaque occlusion and behind-camera doors

Status (2026-09-20): plaque report closed after the user reproduced the edge in
the verified retail-disc emulator session. Keep this behavior unchanged; it is
not a port-only artifact. The evidence is retail code in the pinned emulator,
not original-hardware confirmation or pixel-exact equivalence. The separate
door intrusion received the retail-supported screen-space/rasterizer correction
below; door interaction and placement logic are unchanged.

The user supplied three screenshots on 2026-09-20 showing a plaque edge through
stone and an open door appearing at particular view angles. A separate sanitizer
client reproduced both by ordinary movement, opening the door, and turning.
Diagnostic wrappers only logged the existing camera coordinates and submitted
faces; they did not move objects, alter gameplay, or change rendering commands.
The diagnostic client exited normally without sanitizer reports. The desktop
client and user saves were not involved.

### Plaque edge overdraw

The user confirmed the plaque edge is present in retail. The live emulator
command was checked: pinned PCSX-Redux `b745534e`, the supplied OpenBIOS, and
`-iso build/emulator/retail.cue`, launched by `kf-run-retail`, not
`kf-run-candidate`. That CUE points directly to the original `King's Field
(Japan).bin`. A fresh SHA-256 check returned
`ae74beba377d686bfaa292ea40df8ade4454ec3139c2b5152364e02aac90b3d9`, matching the
pinned retail digest. The candidate-building branch of the launcher was not
used; no replacement GAME/OPEN executables or observer scripts were loaded.
The isolated session log is `/tmp/kf-plaque-retail.HmhTHy/client.log`.

This user comparison supplies the missing retail reproduction for the report.
The ordering trace below explains how the bevel draws over the covering wall;
it does not justify changing retail's depth bias or introducing a depth buffer.

At camera `(25988, -11563, 19040)`, yaw `3872`, the plaque's front triangles are
backface-culled; four bevel triangles remain. The thin strip is not a complete
plaque with a partially transparent texture. At native pixel `(175.5, 124.5)`,
the submitted wall face (map mesh 30) has sort depth `725`, followed by plaque
bevel faces (object 130) at depths `637` and `635`. This sample lies on their shared
edge: the inclusive geometric check lists both, while rasterization selects the
covering triangle. All these faces are opaque; this is not double blending.
The wall's sort depth includes `KF_MAP_OT_DEPTH_BIAS = 200`; larger depths draw
first, so the bevel is painted over the wall. Thus the observed strip is edge
overdraw, not proof that the sign should be fully visible from that side.

Local evidence: `build/plaque-centered.png`, `build/plaque-centered-frame.txt`,
and the one-off `build/plaque-analyze.mjs` point-coverage inspection. These are
ignored diagnostic artifacts, not product features or a new test suite.

The subsequent resource/transform trace uses the later saved pose
`(25994, -11453, 19345)`, yaw `3900`, pitch zero, in
`build/retail-raster-plaque-frame.txt`. `KF/B1/MIXA.DAT` places object 130 in
cell `(13,10)`, with local `(1000,-1500,1700)`, height byte 100 and yaw 1024:
world position `(27000,-11500,21700)`. That cell's attribute 31 selects near
map mesh 30; orientation byte 1 leaves it unrotated at `(26000,-10000,20000)`.
The placement-width/sign and rotation-call evidence is in GAME.EXE
`map_object_pool_load` (`0x80031008`), `render_map_cell` (`0x8001e5ec`),
`render_map_object` (`0x8001ebb8`) and `matrix_set_rotation_y` (`0x80014bec`).

Evaluating this bounded pose with the reviewed Q12 arithmetic gives view rows
`(3912,0,-1207)`, `(0,4096,0)`, `(1207,0,3912)`. It reproduces all 16 captured
plaque camera coordinates, plaque translation `(266,-47,2545)` and the captured
wall translation `(-188,1453,627)`. Decoding the original TMD indices gives:

| Faces (zero-based resource ordinals) | Camera Z values | Sort depth |
| --- | --- | --- |
| Plaque 8 | 2296, 2327, 2232 | 571 |
| Plaque 9 | 2232, 2182, 2296 | 559 |
| Plaque 16 | 2182, 2232, 2327 | 561 |
| Plaque 17 | 2327, 2296, 2182 | 567 |
| Wall 24 and 25 | 1728, 1876, 1728, 1876 | 650 (including +200) |

The covering wall faces are the perpendicular return at world Z `21000`,
spanning X `26498..27000`, not the wall carrying the plaque. The plaque's
bevel/back edge is at X `27000`, with its front at X `27040`; the visible bevel
faces lie behind that return from this camera. Their smaller sort depths still
paint them after it. No placement or depth discrepancy was found in this
calculation; changing the wall bias would contradict the inspected retail rule.

`build/plaque-resource-inputs.mjs` and its output record the decoded bytes and
calculation. This is data-flow consistency for one saved native pose using
reviewed arithmetic, not an independently executed retail frame or proof of
pixel coverage. The later user-driven retail reproduction above, not these
numbers alone, closes the report.

A subsequent read-only geometric check of that saved native trace found 80
pixel centres strictly inside both a plaque triangle and a covering wall
triangle. The plaque is later in the recorded painter ordering at those points;
21 centres are more than half a pixel from every edge of their plaque triangle.
The overlapping region lies within the wall, not merely along its outer edge.
This supports face-order overdraw rather than a gap between wall faces; it is
not a PS1 rasterizer or a texel-accurate comparison. Local one-off artifacts:
`build/plaque-overlap.mjs` and `build/plaque-overlap.txt`. No product code changed.

### Door faces behind the camera

At the same position, yaw `2556`, all 60 vertices of the opened door object 120
have negative camera-space Z, ranging from `-3036` to `-1417`.
The original port path in `src/renderer/projection.cpp:render_project_point`
clamped these depths to zero and used the maximum projection scale.
`render_enqueue_tmd` then added the door's
positive sort bias `15`; this exceeds the minimum submission depth `5`, admitting
faces despite the whole object being behind the camera. Several projected quads
span the viewport and were painted over the visible scene. The door had not
closed again. The initial diagnosis stopped at this projection/submission path;
the subsequent retail check below identifies missing rasterizer rejection.

Local evidence: `build/door-behind-camera.png`,
`build/door-behind-camera-frame.txt`, and `build/plaque-trace-run.log`.
The backend explicitly disables depth testing and sorts whole faces, so neither
reported symptom is Z-buffer fighting. No equivalent retail frame has been
captured; static contract evidence and its limits are recorded below.

### Initial near-plane correction (superseded)

The first correction carried signed camera coordinates through `DrawFace`, used
camera-space facing and clipped at half the selected projection distance. Final
perspective projection also stopped saturating coordinates. It fixed the captured
door failure in an isolated native run, but changed rendering policy beyond
retail's projected-coordinate decisions. The user requested retail behavior,
not a different occlusion model. This correction and its extra runtime metadata
have therefore been removed.

Its historical build/run evidence remains in `build/near-clip-*.log` and
`build/near-clip-*.png`: Linux/WASM/sanitizer builds, native opening/menu/sword/door
interaction and turning, and browser menu save/load across a page reload. The
plaque still leaked in that run. Both reviews checked implementation consistency,
not retail fidelity. These artifacts do not verify the replacement below.

### Retail evidence and native rasterizer correction

`kf init --retail-dir` verified the supplied retail executables before querying
GAME.EXE. Static disassembly, callers/callees, strings, existing match reports,
source history and adjacent projection/enqueue functions were inspected. The
existing reports list the four game-owned functions below as exact; no fresh
matching build, banking or decomp-branch edit is claimed. Raw instructions, not
that recorded percentage, establish these narrower contracts:

| GAME.EXE function | Observed contract |
| --- | --- |
| `tmd_project_vertices`, `0x8001c60c` | Calls `RotTransPers`, then stores full current depth via `ReadSZ2`; the GTE flags are not used for rejection. |
| `render_enqueue_map`, `0x8001de18` | Rejects nonpositive projected winding; quad depth is signed sum shifted by four, plus 200. Triangle depth is signed sum divided by three, shifted by two, plus 200. |
| `render_enqueue_tmd`, `0x8001c7f8` | Projected winding precedes submission; averaged depth plus signed bias must reach five before masking to 14 bits. |
| `render_map_object`, `0x8001ebb8` | Supplies bias 15 for the hinged-door behaviors, 180 for the lift-door pair, otherwise zero. |

The SDK routines are evidence dependencies, not game reconstruction targets.
`ReadSZ2` at `0x8004cb58` stores register 19 through its first argument only;
there is no previous-vertex depth dependency. `RotTransPers` at `0x8004da4c`
returns that depth shifted by two, but this loop obtains the full value through
`ReadSZ2`. `NormalClip` at `0x8004de4c` consumes projected screen coordinates.
`AddPrim` at `0x80054290` inserts at the head of the selected linked list, matching
the native equal-depth LIFO rule. Local disassemblies of the four game-owned
functions are `build/retail-*.asm`.

The missing backend rule is rasterizer size rejection. The documented maximum
vertex separation is 1023 horizontally and 511 vertically, checked before
clipping to the drawing area. Quads split along the existing diagonal; each
triangle can be rejected independently. See the
[PSX-SPX rendering attributes](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/#vertex-parameter-for-polygon-line-rectangle-commands)
and the independent per-half handling in
[DuckStation's polygon command implementation](https://github.com/stenzek/duckstation/blob/master/src/core/gpu.cpp).
These references establish rasterizer rules, not a captured retail game frame.
No emulator code was copied into the port.

In `build/door-behind-camera-frame.txt`, object 120 emits 12 quads. Sixteen of
their 24 triangles have Y spans of 1000 or 777, exceeding 511. The other eight
have 270-pixel Y spans but lie above the viewport. Thus none should contribute
pixels under this rule for the captured inputs. The old backend instead let
OpenGL clip and draw the oversized triangles, producing the intrusion. This is
a concrete native rasterizer omission; it does not require changing door logic,
sort biases or introducing a depth buffer.

The native producers again use projected-coordinate winding and submit those
same coordinates. The backend sorts whole faces first, retains the original
quad halves, applies their independent size limits and draws surviving triangles
without new camera-space clipping. Texture/blend state still follows the original
face order even for rejected geometry. At this stage projection retained host
division; the later arithmetic correction is recorded below. OpenGL pixel
coverage is not claimed bit-exact.

Linux, WASM and sanitizer builds linked with this replacement
(`build/retail-raster-{linux,wasm,sanitize}.log`). A subsequent isolated sanitizer
client rendered the opening and original menus. At camera
`(25994, -11453, 19345)`, yaw `2584`, the room stayed visible with the opened door
behind the camera; at yaw `868` the door appeared correctly open from the front.
The nearby plaque strip remained visible at yaw `3900`. These are nearby
reproduction angles, not identical captured inputs or a retail-frame comparison.
Evidence: `build/retail-raster-{door-behind,door-front,plaque}.png` and their
`-frame.txt` companions. No rendering or gameplay state was injected.

The original native load menu restored the first save cross from a seeded slot
one; its save menu wrote an 8,964-byte slot two in
`/tmp/kf-raster-saves.bmwRgS`. The save menu returned to the rendered cross.
This run did not load the newly written native slot two. The isolated client
exited with status zero and `build/retail-raster-run.log` contained no sanitizer
diagnostics. The user's desktop client and saves were not used.

The replacement browser build imported the disc locally, used a seeded native
slot one only to reach the save point, and wrote its own 8,964-byte slot two
through the original save menu. After a page reload it restored the verified
resource cache and loaded that slot through the original load menu; saved bytes
were unchanged. See `build/retail-raster-browser.log` and
`build/retail-raster-browser-save-reloaded-position.png`. No application stop or
browser exception was reported. Audio context activity is not a listening check;
the displayed persistent-storage warning still applies. This verifies page
reload, not browser-process restart, eviction survival or power-loss durability.

These historical runs used timed key input, not a deterministic record/replay
mode. Following the user's clarification, further interactive gameplay checks
are left to the user unless a deterministic mode is separately agreed.

The plaque's four bevel faces do not exceed these size limits, so this rule is
not a plaque fix. Their ordering is established for the port's captured inputs,
not proved to reproduce retail's complete frame. The bounded placement trace
above found no mismatch at this pose; the next evidence needed is a comparable
retail view or a specific rasterization discrepancy. Do not assume the plaque
leak existed on the console or adjust the wall bias to hide it.

### Projection arithmetic correction

The previous native projection evaluated an ordinary floating-point quotient,
bounded by exactly two, and an unquantized fog formula. Those are not the
arithmetic rules used by the original rendering boundary. The native
`render_project_point` now uses an integer reciprocal with its documented seed
and refinement steps, a maximum Q16 scale of `0x1ffff`, signed screen-coordinate
rounding and saturation, and the original fog coefficient truncation. The
projection distance is unsigned 16-bit; the fog reference remains fixed at 200
when weapon rendering changes the projection distance.

Static evidence was gathered after verifying the retail image hashes. GAME.EXE
`RotTransPers` at `0x8004da4c` executes the Q12-shifted projection instruction and
returns the projected XY and fog results. `SetGeomOffset` at `0x8004caf0` supplies
integer screen centers shifted by 16. The existing exact report for the unchanged
GAME consumer `tmd_project_vertices` is not a fresh match or a proof of native
arithmetic. Its disassembly confirms that the returned flags are not consumed.

GAME.EXE `SetFogNear` at `0x8004f4b4` computes a wrapped 32-bit `-320 * near`,
divides it as a signed integer by the reference distance, and passes the result
to the signed 16-bit coefficient setter. Its intercept is `0x01400000`.
The display initializer, both fog setters and weapon-nine lighting caller use
reference distance 200. OPEN.EXE `SetFogNear` at `0x8002f288` has the same
arithmetic. These routines are already identified as vendored in
`functions_vendored.tsv`; they are dependencies, not newly reconstructed game
functions. No SDK API, register bank, command dispatcher or FIFO was added.

The reciprocal and saturation rules are described in
[PSX-SPX's GTE arithmetic specification](https://psx-spx.consoledev.net/geometrytransformationenginegte/#gte-division-inaccuracy).
The native integer outputs are also consistent with the unenhanced coordinate
and fog path in
[DuckStation's GTE implementation](https://github.com/stenzek/duckstation/blob/master/src/core/gte.cpp).
These sources establish arithmetic rules, not an equivalent retail frame.
For example, camera X=500 and Z=5000 at projection distance 200 produce a Q16
scale of 2621: with screen center 160, X becomes 179, not the previous formula's
180. Keeping ordinary division would retain a concrete discrepancy.

Linux, WASM and sanitizer builds linked the changed projection and its original
callers (`build/retail-projection-{linux,wasm,sanitize}.log`). No game was run for
this step and no new test suite was added. Recalculating the 16 saved object-130
camera vertices in `build/retail-raster-plaque-frame.txt` changed none of their
screen positions. This does not explain the plaque strip, establish the wall's
projection, or verify the complete rendering chain. Earlier runtime captures
precede this arithmetic change.

### Lighting arithmetic correction

The previous native lighting used floating-point interpolation and clamped the
fog factor to 0..4096. The existing value-based `render_light_normal` and
`render_fog_color` now preserve the integer stages: Q12 matrix products, shifted
32-bit results and positive 16-bit saturation, then base-color multiplication.
Fog first quantizes and saturates the color difference to signed 16-bit before
interpolating. The input factor narrows to signed 16-bit; it is not clamped to
0..4096. Ambient and far-color scaling retains the original wrapped 32-bit shift.
The final result is quantized and saturated to RGB8, with its code byte retained.

Hash-verified GAME.EXE `NormalColorDpq` at `0x8004dd14` executes `0xe80413`
(NCDS, Q12 shift, positive intermediate saturation); its triple variant at
`0x8004dd40` executes `0xf80416`. `NormalColorCol` at `0x8004dd8c` uses
`0x108041b` without fog, and `DpqColor` at `0x8004dbc4` uses `0x780010`.
The fog wrappers write the caller's factor directly. `SetBackColor` at
`0x8004ca3c` and `SetFarColor` at `0x8004ca5c` shift each input left by four.
OPEN.EXE homologs at `0x8002dae8`, `0x8002db60`, `0x8002d998`, `0x8002c090`
and `0x8002c0b0` confirm the same single-color and setter operations. These are
already attributed vendored dependencies, not newly reconstructed game bodies.

The stage formulas are documented in
[PSX-SPX's color calculation specification](https://psx-spx.consoledev.net/geometrytransformationenginegte/#gte-color-calculation-commands).
[DuckStation's GTE implementation](https://github.com/stenzek/duckstation/blob/master/src/core/gte.cpp)
also corroborates factor sign extension, narrowing before intermediate saturation,
and final color extraction. No emulator code, register bank, FIFO or SDK API was
added to the port; only the returned color calculation changed.

Original map producers still call lighting to obtain a byte color, then fog that
color separately per vertex. Combining those calls would lose their intermediate
quantization. Original boosted fog inputs can exceed 4096, and the ending uses
non-black fog. For example, a byte channel of 64, far color 128 and factor 8192
produce 192, whereas the previous clamped blend produced 128. This arithmetic
example does not establish that a particular captured frame contains those values.

Linux, WASM and sanitizer builds recompiled `lighting.cpp` and linked the
clients (`build/retail-lighting-{linux,wasm,sanitize}.log`), without new compiler
warnings. No game execution, animation-rate adjustment or new test suite
accompanies this step. Full scene inputs, shader interpolation, pixel precision
and frame history remained separate verification gaps at this point; this does
not resolve the plaque or flame reports. The subsequent pixel change follows.

### Pixel arithmetic correction

Later feedback (2026-09-20): the user reports that Linux rendering looks fine,
closing the general visual check. This is not a pixel-exact comparison or
performance benchmark. The separate plaque-occlusion report was subsequently
closed after the user reproduced it in the verified retail-disc emulator session.

The prior shaders tinted decoded textures and used fixed-function blending into
an RGBA8 target, without dithering or five-bit writeback. Original display setup
enables dithering in both buffers: GAME.EXE `display_initialize` at `0x8001bb94`
stores one to the two `dtd` fields at `0x8001bc34` and `0x8001bc40`; OPEN.EXE's
initializer at `0x80016adc` does so at `0x80016ba8` and `0x80016bb4`. Hash-verified
disassemblies are retained in `build/retail-pixel-{game,open}-display.asm`.
Both buffers start at X=0 and Y=0 or 240, preserving the same four-pixel dither
phase when represented by one native 320x240 target. No original function or
curated reconstruction inventory was changed for this evidence check.

Native faces now retain flat/Gouraud shading and raw/modulated texture modes as
separate scoped enums. STAT.DAT decoding preserves the raw-texture bit; original
render submissions preserve their shading choice. The inspected HUD, menu and
floor-item "sprite" producers emit polygons, not hardware rectangle commands.
The shader dithers modulated textured polygons and untextured Gouraud polygons;
raw textures, flat untextured polygons and clears remain undithered. The four-by-four
offset pattern precedes RGB5 quantization, following the documented
[polygon and dither rules](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/#24bit-rgb-to-15bit-rgb-dithering-enabled-in-texpage-attribute).

The fragment shader now performs integer texture modulation and saturated RGB5
blending. In five-bit channel values, the four blend operations are
`(B + F) >> 1`, `B + F`, `B - F`, and `B + (F >> 2)`.
Zero texture words remain transparent; STP black remains visible, and a blended
textured face still blends only its STP texels. Quantized channels are bit-expanded
into the RGBA8 storage target after each write, including undithered clears.
This is per-write arithmetic, not a final-screen filter.

Before each blended triangle, the backend copies its clipped framebuffer bounds
to a separately owned texture. The shader reads that snapshot; it never samples
the attached color target. This avoids the
[framebuffer feedback restriction](https://registry.khronos.org/webgl/specs/latest/1.0/)
while preserving the existing whole-face order and quad-half order. Opaque
triangles need no snapshot. Copy cost and resulting completed-frame cadence have
not been measured; unchanged timing code is not a performance guarantee.

The modulation equation uses eight-bit vertex shading, consistent with the
[later GPU revision](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/#shaded-textures).
The launch-model GPU's earlier truncation is not implemented. OpenGL still owns
attribute interpolation and pixel coverage; full frame inputs/history remain
unverified. These changes therefore do not establish pixel-exact retail output,
nor do they resolve the plaque or flame reports.

Linux, WASM and sanitizer clients linked after the producer/header/shader changes
(`build/retail-pixel-{linux,wasm,sanitize}.log`); pre-existing gameplay warnings
remain. The actual two source shaders also compiled and linked with no messages
and no GL error in an isolated Chromium WebGL2 context
(`build/retail-pixel-shader.log`, which records the source hash). That one-off
check used a blank page, without the application, assets, drawing or input.
It verifies shader compilation/linking, not native-driver execution, rendering
correctness, performance or gameplay. No new test suite was added.

### Retained-frame ownership and presentation

The inspected retained-image callers do not alternate independent accumulated
backgrounds. GAME.EXE `display_show_system_screen` at `0x8001b7b0` selects the
buffer opposite `display_state.buffer_index`, disables its clear policy, and
submits one overlay before waiting for input. `screen_show_image_until_input`
at `0x8002c794` selects that buffer once, then repeatedly submits its image while
ramping brightness from 32 toward 127. Neither path swaps display environments
or begins a new scene frame inside the modal wait. The existing native
`FrameClear::Retain` submissions likewise accumulate on one target. Window
resize/expose only presents its stored pixels and does not repeat the blend.

This supports the single-target accumulation model for these callers, not full
retail frame-history equivalence. GAME.EXE `display_present_frame` at
`0x8001c050` installs matching-index draw/display environments before submitting
the draw list; their initialized Y origins are opposite. It therefore displays
the other buffer while drawing the next image. The native backend instead
draws and presents its new target in one call. A modal background can consequently
come from a different generated scene frame. No extra display delay or second
retained history was introduced during this audit; exact presentation chronology
remains unverified. The modal brightness ramp's portable yielding wait is also
not a measurement of retail's GPU-completion-paced loop.

Evidence is retained in `build/retail-history-game-display_present_frame.txt`,
`build/retail-history-game-display_show_system_screen.txt` and
`build/retail-history-game-screen_show_image_until_input.txt`, including raw
instructions, references and the inspected existing match reports. Those reports
are not new matches or verification of the native renderer. No product code,
gameplay timing, application run or test suite changed for this audit.

### Rendering model coverage

The model is a set of data-flow and arithmetic contracts for the existing render
path, not another gameplay world or a replacement scene implementation. Reusing
the original producers alone does not establish the complete chain. Each row
needs its own evidence; a correct downstream rule cannot validate its inputs.

| Stage | Current implementation and evidence | Remaining gap |
| --- | --- | --- |
| Resources, animation and visibility | Original loaders, pose and visible-cell/entity decisions feed the renderer. | Portable decoding, matrix inputs and all producers are not yet audited as one retail-equivalent frame. |
| Camera/model transforms | Explicit fixed-point matrices and copied vertices; a bounded resource/Q12 calculation reproduces one saved plaque pose and its wall translation/depth. | No independent retail frame establishes that pose's full inputs; other placements/compositions remain outside this bounded check. |
| Projection and fog factor | Integer reciprocal and coefficient rules above replace the floating-point approximation. | Rule-based implementation and builds are not an end-to-end retail-frame comparison. |
| Facing and sort depth | Inspected GAME producers use projected winding, signed depth arithmetic and authored biases. | Evidence covers the named functions, not every render producer and every input path. |
| Face ordering and splitting | Descending whole-face order, equal-depth head insertion and original quad diagonals. | Does not establish that all submitted inputs and draw state are correct. |
| Rasterizer rejection | Oversized triangles rejected independently before viewport clipping. | Other pixel-coverage rules remain unaudited; no equivalent retail plaque frame is available. |
| Lighting | Integer lighting/fog stages above, retaining the original intermediate byte colors. | Full lighting inputs are not established for a retail-equivalent frame. |
| Textures and blending | Owned TIM/palette data, native raw/shading metadata, RGB5 writeback, dithering and integer blending. | Launch-model modulation differs; OpenGL interpolation/coverage, full frame inputs and snapshot performance remain unverified. |
| Presentation | Fixed-target modal accumulation, retained pixels on expose/resize, clipping and original callers. | Immediate native presentation differs from retail's opposite-buffer chronology; complete frame-history and menu/effect behavior are not established as exact. |

Continue at the unresolved boundary, retaining evidence of any discrepancy.
Do not replace this audit with depth buffering, new authored sort biases, or an
assumption that the plaque artifact must also exist in retail.

## Flame animation rate and frame-dependent logic

Status (2026-09-20): report closed after the user compared the Linux client with
the pinned retail emulator and confirmed that the flame flickers analogously
in both: this was the original behavior. No flame-specific animation slowdown
was added or is needed. This is a visual confirmation of the reported effect,
not an exact timing measurement or proof about all other animations.

The initial report was that flame/torch frames looked correct but animated too
quickly, not that room brightness flickered. The static investigation below
made no timing change or application run; the later user comparison resolves
the report independently of the scripted-world pacing correction.

GAME.EXE `render_floor_item` at `0x8001ed90` increments the frame byte after each
sprite submission and wraps at the packed low-nibble count. Instructions at
`0x8001ee9c..0x8001eec0` contain no separate timer or frame-repeat divisor.
The port's `floor_item_advance_frame` retains that progression for decoded valid
frames. `render_entities` calls the renderer once per visibility-eligible item;
its retail call is at `0x8001f560`. Re-reading the original `KF/B1/MIXA.DAT`
placements confirmed the earlier table: two base-four, three-frame sequences and
twenty base-zero, four-frame sequences. The sprite definitions are unchanged.

GAME.EXE `frame_pacer_wait` at `0x800149f4` waits until the VBlank counter exceeds
the previous value plus two, then records the current count. The callback at
`0x800149d4` increments it once; the main loop registers that callback on the
VBlank event. The native normal loop similarly waits for three ticks on its
60-Hz absolute clock, with one scene render per game update. Its intended steady
rate is therefore about 20 updates per second, not the monitor refresh rate.
At that rate the three- and four-frame sequences repeat in 150 and 200 ms.
These are derived rates, not measurements of the user's client or retail
performance. Slower completed frames would also slow these animations; no retail
slowdown has been established here.

The port's other inspected frame dependencies include:

- Player movement/keyboard turning, charge accumulation and status timers in
  `player_update.cpp`; actor and object behavior, effect updates and map-event
  progression are called from the same game loop.
- Flame/floor-sprite animation and notification effects advance from rendering.
  Extra calls to original scene render producers can advance state without a
  corresponding normal gameplay update. Retained-frame presentation does not
  call those producers.
- Menus and blocking fades/scripts have their own loops and waits; the normal
  gameplay pacer alone does not establish every loop's rate.
- Audio playback and sequence scheduling use elapsed sample time through the
  shared clock, although game decisions to start sounds occur in game updates.

Static disassemblies are saved in `build/retail-render-floor-item.asm`,
`build/retail-render-entities.asm` and `build/retail-frame-pacer.asm`. Existing
exact reports for the two queried game functions were inspected, not rebuilt
or newly banked. No missing animation divider or unbounded normal-loop FPS was
found in the examined code. This is not proof that the reported speed matches
retail; do not add an arbitrary flame delay or change the game tick to hide it.

### Opening cutscene speed report

The user also reports that the opening cutscenes feel too fast in the latest
Linux run. This is a separate pacing path from the normal gameplay limiter.
OPEN.EXE `opening_scene0_run` (`0x80014268`) steps the camera once at
`0x800142cc` and renders once at `0x800143ac`. Its renderer (`0x800194d8`)
calls `display_present_frame` (`0x80016dd0`), which waits for GPU completion
and calls `VSync(0)` at `0x80016de4`. The SDK wrapper (`0x800352c4`) calls
its wait worker once for mode zero; it does not apply GAME's three-tick limiter.
The original camera step (`0x80014100`) advances Q4 position and rotation once
per call, as does the port.

The native opening similarly has one next-tick wait on the host's 60-Hz clock
per ordinary rendered frame, not the normal gameplay loop's three-tick wait.
Thus 60 rendered updates per second is possible when native work finishes in
time; the later native measurement below covers this path, not retail. The removed
console CPU/GPU work could have consumed additional display intervals. Retaining
the original per-frame calculations therefore does not, by itself, preserve
cutscene duration. No omitted explicit divider was found in this inspected
fly-through chain, and no new rate or animation divisor has been selected.
The later pinned-emulator measurement below establishes a comparison for scene0,
not original-hardware timing. Other opening paths include extra waits, so a
blanket change to all waits would also change title holds, fades and transitions.

Static evidence: `build/retail-opening-cadence.txt` and
`build/retail-opening-vblank-worker.txt`. SDK bodies were inspected only to
understand their timing boundary, not reconstructed or counted as game progress.

For the native measurement, `build/opening-timing.cpp` and
`build/opening-timing-build.mjs` build a separate, non-shipping client using
the existing Linux objects and linker wrappers. It observes the original scene0,
camera-step and presentation calls: segment/step counts, presentation intervals,
active-clock duration and wall duration are logged. It changes neither waits nor
inputs and exits its own process after scene0 returns. The final input action
distinguishes a skipped scene; focus pauses are excluded from the active clock.
After explicit user authorization, it completed one fly-through with no supplied
input and exited with status zero. The final input action was zero and the path
reached point 15. There were 508 camera-step calls and 507 presentation calls;
the final loop step exits without rendering. From the first camera step to the
last presentation return took 8.506395 active seconds. The 506 intervals between
presentation returns spanned 8.484701 seconds, or 59.636749 calls/second; their
minimum/maximum intervals were 8.680/31.579 ms. The whole scene call, including
resource loading and the final audio fade, took 10.963448 wall seconds and
10.963423 active seconds. These are presentation-call timings, not GPU-completion
or physical-refresh measurements, and include small unquantified observer overhead.

The observation is recorded in `build/opening-timing-observation.txt`. This
establishes approximately 60 native opening updates/second for that run, not
retail speed, flame timing, a normal module exit or re-entry.

### Pinned-emulator opening measurement

With explicit user approval, `kf-run-retail` ran the hash-verified original disc
in the repository's pinned PCSX-Redux `b745534e`, using its supplied OpenBIOS
(`c6c55240f02b447fddec85144184fdcaf4b496a3`). A temporary portable profile under
`/tmp/kf-retail-opening.jDm1sJ` isolated settings and memory cards. `-safe` selected
defaults: dynamic recompiler, software GPU, scaler 100, automatic NTSC selection,
no fast boot and no debugger. No game inputs, memory/register writes, patches,
save states or speed enhancements were supplied.

The non-shipping `build/retail-opening-observe.lua` reads the camera-path state
on `GPU::Vsync`, guarded by OPEN.EXE instruction signatures and the scene0 path
pointer. This keeps the default CPU core, without execution breakpoints. It
records the emulator's CPU-cycle counter, monotonic host time and VBlank count.
The [emulator API](https://pcsx-redux.consoledev.net/Lua/redux-basics/) defines
cycle time as the cycle count divided by `PCSX.CONSTS.CPU.CLOCKSPEED`; this run
reported 33,868,800 cycles/second. Point changes are first-observed-on-VBlank
timestamps, not exact camera-call timestamps; each endpoint can lag its state
change by up to one VBlank. The state is not an atomic pose snapshot.

The comparable interval is **camera point 1 to first reaching point 15**, not
the native first-step-to-last-presentation span above:

| Observation | Point 1 to point 15 |
| --- | --- |
| Native camera-step observer, active host time | 8.223520 s |
| Retail executable in pinned emulator, cycle-derived time | 22.313425276 s |
| Same emulator interval, monotonic host time | 22.295302697 s |

The emulator interval spans 1,339 VBlanks and 755,728,938 CPU cycles. Its duration
is about 2.71 times the native interval, far beyond the sampling uncertainty.
The close host/cycle times show this was not a host-overhead slowdown masquerading
as game timing. Across the whole observed path there were 508 distinct sampled
countdown states; the 507 intervals between them were 169 two-VBlank, 300
three-VBlank and 38 four-VBlank intervals. These are sampled camera-state changes,
not GPU-completion or physical-display measurements. They demonstrate variable
cadence in this emulator run, not a missing fixed three-tick wait in the source.

All 1,520 samples of the active scene0 state had zero `opening_input_action`.
The observer requested exit 180 VBlanks after first observing point 15; its
process exited with status zero. This cleanup is not a natural module-exit check.
Evidence: `build/retail-opening-observation.txt` and the read-only comparison
`build/retail-opening-compare.mjs`. Earlier setup attempts failed before producing
a timing observation; the successful run used the pinned build's preloaded
`luv` library.

This confirmed that the then-current scene0 was substantially faster in the port
**relative to this emulator reference**. The supplied BIOS and the emulator's
CPU/GPU/CD timing are not original hardware, and this run does not establish
other cutscenes' rates or flame timing. No production timing code changed during
the measurement. A single fixed rate cannot reproduce the observed variable
cadence; the subsequent user-approved policy below deliberately chooses stable
updates instead of baking this one emulator trace into gameplay.

### Stable update policy and loop boundaries

The user chose stable, retail-informed update rates while preserving original
per-step logic. This is a broader timing policy, not a presentation-only fix:
frame-driven calculations must run at the owning loop's rate, regardless of how
quickly the native renderer completes. It does not authorize multiplying every
gameplay calculation by elapsed time or changing all loops to one frequency.

The focused caller trace found these boundaries at the time of the scene0 fix.
The subsequent [GAME correction](#game-world-cadence-boundary) moves its wait into
world rendering and closes the unpaced GAME paths listed here:

| Owner/path | Advancement and existing timing boundary |
| --- | --- |
| `game_main_loop` | Player, actors, objects, effects and map events advance once; one world render; `frame_pacer_wait` enforces a three-host-tick minimum, about a 20-update/s ceiling. |
| GAME `render_frame` → `render_floor_item` / `notify_effect_update` | Eligible floor sprites and notifications advance from scene production, even when a caller is outside the normal player/AI loop. No separate animation clock. |
| GAME warp shimmer, reveal fade and lighting transition | Their nested world-render loops also call the three-tick pacer; they do not run the whole normal player/AI update chain. |
| GAME `map_floor5_transition_cutscene` | Its camera loop and subsequent transformation/effect loop call world rendering without the gameplay pacer. The presentation's next-host-tick wait alone limits them; this is a concrete dependency outside OPEN. |
| OPEN scene0 | Camera, entity rotation and lighting fade advance together once per loop. The new loop-local deadline is 22 updates/s. |
| OPEN scene1 hold / cylinder transition | The hold explicitly waits one host tick without rendering; the cylinder loop renders and then explicitly waits another tick. Neither inherits the scene0 deadline. |
| Menu / input-release / retained-frame paths | Menu presentation and explicit input waits yield on the host clock. Re-presenting retained contents does not call world-render producers or advance player/AI/flame state. |

These are source-level timing dependencies, not measured rates for every row or
a complete gameplay-speed audit. Sources: `src/game/{game,render_frame,
render_scene,map_scripts,player_warp,lighting,menu_runtime}.cpp`,
`src/lib/floor_item_render.inc`, `src/open/opening_scenes.cpp`, and
`src/platform/host.cpp`. Audio mixing and sequence scheduling remain based on
elapsed sample time; game-loop sound-trigger decisions can still move in time
when their owning update rate changes.

For scene0, the equivalent point-1→15 interval contains 490 update intervals.
Dividing by 22.313425276 emulated seconds gives approximately 21.96 updates/s;
the selected fixed rate is 22. Its nominal interval duration is therefore
490/22 = 22.272727 seconds, about 0.18% shorter than this emulator observation.
This preserves the aggregate duration approximately, not each segment's original
slowdowns or timing against music. The original camera, rotation, fade and
input/exit calculations are unchanged. Scene0's own fade, entity rotation and
render-driven floor sprites all adopt the new loop rate; only other scenes'
fade/title/script paths are unchanged.

The portable `UpdatePacer` is a local plain structure initialized after scene
resource loading and camera setup. `host_wait_update` advances an absolute
nanosecond deadline, carrying division remainders. The original render wait
consumes part of that interval rather than adding another complete interval.
It uses the existing focus-pause-aware clock and SDL/Asyncify yielding path. A
stall of a whole extra update interval rebases the deadline instead of building
a catch-up backlog. It does not skip, multiply or inject game steps, and there
is no module-owned pacer state to leak into a later scene0 invocation. The
existing presentation wait still quantizes submissions to the 60-Hz host grid,
so stable update deadlines do not imply evenly spaced presentation returns.

Linux and WASM linked the change (`build/opening-fixed-cadence-{linux,wasm}.log`).
The original scene0 evidence pass was refreshed after `kf init`, including its
caller, callees, empty string references, CFG and existing match report in
`build/opening-fixed-cadence-evidence.txt`; no reconstruction source or match
claim was changed. At this stage, other opening sequences, GAME scripted loops
and the reported flame speed remained pending; this single implemented deadline
did not establish their timing. The later GAME correction and user flame
comparison are recorded separately.

### GAME world cadence boundary

The user asked to implement the known timing corrections without requiring
further manual scene measurements first. GAME world-frame production also
advances eligible floor sprites and notification effects. Its floor-5 camera
and transformation/effect loops, plus brief extra world redraws, previously
relied only on the next presentation tick. They could therefore advance that
state faster than GAME's ordinary three-tick cadence.

`render_frame` now calls the existing `frame_pacer_wait` after presentation and
the stale-pool sweep. All nine matching caller-side waits were removed. The
limiter's three-tick deadline, initialization before the first warp render,
late-frame rebase and module reset are unchanged. Presentation consumes part
of the deadline; it does not add a second complete game interval. Focus-paused
time is still excluded by the shared host clock, and waits do not re-enter game
logic. Original calculations, visibility decisions and per-loop step counts
remain intact; no animation divider, delta-time multiplier or catch-up loop was
introduced.

This deliberately applies the existing approximately 20-update/s GAME policy to
the formerly unpaced floor-5 loops, paired info/death/warp redraws and the final
pose of a blocking map-event animation. Those short redraw paths may now take
longer too. It is not evidence that retail floor-5 ran at exactly 20 Hz. Ordinary
gameplay and already-paced blocking loops retain one wait per world frame.
Normal-game flames already advanced at that cadence: this fixes the bypasses,
not a demonstrated cause of the user's normal-game flame-speed report, which
the subsequent user comparison closed as analogous retail behavior. OPEN's
22 Hz scene0 deadline, other OPEN waits, menu/retained-frame presentation and
the sample-based audio clock are untouched.

The matcher evidence pass used GAME explicitly after `kf init`: disassembly/CFG,
callers/callees, strings, data/referents and existing match reports for the moved
wait sites and world-frame entry were inspected, with adjacent source and
history. Retail has nine explicit calls to `frame_pacer_wait`; the floor-5
camera/effect calls at `0x80034734` and `0x80034a04` have no such wait. The port
change is a timing-boundary adaptation, not a missing-source restoration or
decomp backport. Local snapshots are `build/world-cadence-game-*.txt`.

Linux and WASM rebuilt and linked (`build/world-cadence-{linux,wasm}.log`).
Both code and holistic reviews found no must-fix issue. The holistic review's
request to document the extra redraw/final-pose delays is included above.

A brief no-input Linux observer reused the shipping objects, counted original
presentation, pacer and player-update calls, and exited after 120 world waits.
It observed 120 presentations and 70 player updates (the initial warp accounts
for the other 50 world frames). The 119 wait-return intervals took 5.949984001
active seconds: 20.000054 intervals/s, with 49.907–50.092 ms individual intervals.
The paired initial warp redraws were 50.016 ms apart. This confirms one shared
wait per observed world frame, without doubling the ordinary gameplay interval;
it does not exercise floor-5, later interactions or the user's flame appearance.
No input was sent, no controller was bypassed, and saves were isolated in
`/tmp/kf-world-cadence.GyLwLI`. The observer exited with status zero. Artifacts:
`build/world-cadence-observe{.cpp,-build.mjs}` and
`build/world-cadence-native.log`. No new tests or matching campaign were added.

### Corrected opening observation

After user authorization, the same no-input observer was rebuilt against the
corrected Linux objects and run with isolated saves. Point 1 (step 1) to point 15
(step 491) took **22.273082 active seconds**: 0.355 ms above the fixed-rate target,
and about 0.18% shorter than the pinned emulator's 22.313425276-second interval.
All camera marker step ordinals matched the earlier native run. There were again
508 camera-step calls and 507 presentation calls, with final input action zero.
The original scene0 function returned and the observer closed its own process
with exit status zero.

The whole scene took 27.776278 wall seconds and 25.496334 active seconds. The user
confirmed focus loss during this run; the approximately 2.280-second difference
is excluded by the host's focus-pause-aware clock. The camera-marker comparison
uses active time, not the longer wall duration. The observer does not record
individual focus events, so this is not a controlled skip/focus interaction check.

First camera step to last presentation return took 23.019932 active seconds.
The 506 intervals between presentation returns spanned 22.996620 seconds, averaging
22.003233 calls/second; shortest/longest intervals were 23.988/64.350 ms. Those are
call-return measurements, not GPU completion or display refresh. The existing
60-Hz presentation wait and host scheduling still allow presentation jitter.

Evidence: `build/opening-fixed-cadence-native.log` and
`build/opening-fixed-cadence-run.md`. This verifies the selected stable scene0
cadence on Linux. It does not establish identical timing within each retail
segment, music synchronization, other cutscenes, flames, browser runtime timing,
interactive skipping or natural module re-entry. No production source changed
for this observation, and no additional emulator run was needed.

### Title-card timing observation

The following automatic title phase, `opening_scene1_run`, already retains its
explicit hold and was measured separately rather than inheriting scene0's 22 Hz
rate. OPEN.EXE `0x8001455c..0x80014608` calls 33 fade-in draws (shades 0 through
128 in steps of four), waits up to 1,000 VBlanks while polling input, then calls
33 fade-out draws (128 through zero). Counter 600 stops the sequence. The static
evidence pass is in `build/title-timing-evidence.txt`; existing reconstruction
match reports were read, not rebuilt or changed.

The separate native observer calls the original scene1 function, records submitted
face shades and timestamps its existing presentation calls, and exits only after
that function returns. The first observer attempt could not intercept a call
within the same translation unit and recorded no fade markers; that attempt is
not fade-timing evidence. The corrected observer wraps the external face-enqueue,
presentation and input-poll calls without changing their arguments or waits.
It observed 66 presentations and 1,033 input polls, with the expected shade ramps
and final input action zero. The pure 1,000-wait hold, measured from the final
fade-in poll to the final hold poll, took **16.666046 active seconds**.

Retail ran subsequently, not alongside the native client, in pinned PCSX-Redux
with the same default CPU/GPU/NTSC/OpenBIOS configuration as the scene0 reference.
Its isolated profile was `/tmp/kf-title-retail.tDLIzy`; the native save directory
was `/tmp/kf-title-native.pBShaO`. No inputs, memory/register writes, save states
or debugger breakpoints were supplied. The Lua observer reads the two title FT4
packets on VBlank, guarded by OPEN instruction signatures, scene0 having reached
its fade-out point, packet extents, geometry, texture pages and matching RGB values.

Packet memory is not an atomic command event: the active buffer can briefly
contain its previous packets before rebuilding. The recorded sequence was
therefore checked before choosing endpoints. It contained exactly the expected
66 shade observations, including two separate shade-128 observations in
alternating buffers, with no extra/backward samples. Every recorded change had
PC `0x80035448`; no recognized-title sample had nonzero input. This supports this
run's phase comparison, not a general guarantee against stale-buffer sampling.

| Phase markers | Native active seconds, before presentation | Retail cycle-derived seconds | Retail VBlanks |
| --- | --- | --- | --- |
| Fade-in shade 0 → 128 | 0.533694 | 0.533256 | 32 |
| Fade-in 128 → fade-out 128 | 16.682857 | 16.680910 | 1,001 |
| Fade-out shade 128 → 0 | 0.535368 | 0.533256 | 32 |
| First → last shade marker | 17.751919 | 17.747422 | 1,065 |

The two-peak interval includes one presentation boundary as well as the hold;
it is not a direct measurement of just the 1,000 waits. Native markers are before
presentation; retail markers are first-observed-on-VBlank constructed packets.
Sampling and different marker positions allow roughly one VBlank of endpoint
uncertainty, so the approximately 4.5 ms total difference is not an exact timing
or physical-display claim. Retail host time for the same markers was 17.726272 s.
The full native function, including resource/audio preparation, took 19.700850
active seconds and 19.700957 wall seconds; focus-paused time was negligible in
this run. Active time remains the comparison clock when focus pauses occur.

Both observers exited successfully. Retail stopped at observing the last shade
packet, not at completed presentation or natural scene return; native exited
after the original scene1 return, not after normal application/module exit.
Evidence: `build/title-timing-native.log`, `build/retail-title-observation.txt`,
`build/title-timing-retail-run.log` and `build/title-timing-compare.mjs`.

This bounded observation supports keeping the title's existing approximately
60 Hz fade/hold timing. **No title timing change is warranted by these results.**
No production code changed and no additional Linux/WASM product rebuild was
needed. Other independent cutscenes, browser runtime timing and interactive
skip/focus behavior remain separate work. The flame report was subsequently
closed by the user's comparison, not by this title observation.

### Scene3 observation requires title input

The queue initially described scene3 as the next automatic opening phase. That
was incorrect. The current original-source controller in
`src/open/opening_controller.cpp` and the hash-verified OPEN.EXE agree: after
scene1 returns, action 1 branches from `0x80015848` toward the scene3 call at
`0x80015870`; action 2 takes the skip path, and action 0 jumps back to scene0 at
`0x80015858`. With no input the flythrough/title cycle repeats indefinitely.
No scene3 observer run was launched on the false assumption that it would be
reached automatically. The user was asked to drive the title confirmations in
sequential native/retail clients; timing stays unchanged pending that observation.
This does not invalidate the completed no-input scene0/title measurements.

## Pause, menu Escape and Linux keyboard remapping

Status (2026-09-20): the user confirmed the Linux inputs are verified, covering
fresh-input pause, menu Escape, Caps-to-Esc and mouse capture restoration after
modal interaction. These reported input issues are closed; browser pointer-lock
permissions are not covered by this Linux confirmation.

The user's screenshot `2026-09-20-13-22-31.png` shows `menu_use_item_panel`.
That original menu handles the Back action, but the host mapped Escape to Select
(pause) in every input context. Escape now supplies Back in menu context and
Select outside it. Backspace remains Back, and P remains Select. No menu
selection, inventory or gameplay action logic is replaced.

The original system-screen routine at GAME.EXE `0x8001b7b0` loads its image
from CD, then waits for a held button followed by release (`0x8001ba40` and
`0x8001ba50`). It does not first release the input that opened the screen.
The same sequence with ordinary fast file reads explains the reported
hold-P-only pause: the opening P is already sufficient acknowledgement.
For the pause screen only, the port now waits for release before accepting a
fresh press/release. This is an explicit input-boundary adaptation requested
by the user, not a missing retail instruction; no artificial CD delay is added.
Existing system-screen evidence is in
`build/retail-history-game-display_show_system_screen.txt`.

Keyboard bindings now use logical keycodes from the current keyboard layout,
ignoring modifiers for letter bindings. Physical scancodes only track which
press owns each release. This preserves a held logical action when two keys
map to it and clears that ownership on focus loss. The implementation uses
[SDL's layout translation](https://wiki.libsdl.org/SDL3/SDL_GetKeyFromScancode),
not a special case for Caps Lock. The user's remapped keyboard and interactive
pause/menu behavior were subsequently confirmed in the Linux client. No new
input automation or unit-test campaign was introduced.

Linux, WASM and sanitizer configurations rebuilt the changed host and original
render unit and linked successfully (`build/input-boundary-{linux,wasm,sanitize}.log`).
That build still reported the unused TMD release-slot parameter. No client was restarted
or steered for these checks.

### Restoring mouse capture after modal interaction

After the user requested a restart with the input fixes, reading the plaque and
exiting with Escape exposed a second host-boundary issue: entering Menu disabled
relative mouse mode, but returning to Gameplay never restored it. The plaque
path nests Gameplay -> Scripted (`map_interaction_dispatch`) -> Menu
(`screen_show_image_until_input`) -> Scripted -> Gameplay.

The host now remembers whether capture was active when leaving Gameplay and
restores it only on the outer return. Nested non-gameplay contexts do not
overwrite the saved request. An initially uncaptured session remains uncaptured;
focus loss or an external capture loss while in Gameplay/Scripted clears the
request. Entering Opening also clears it rather than carrying it to a new game.
Mouse deltas are still cleared at context boundaries, and no gameplay input is
synthesized. A failed native capture request is reported; browser pointer-lock
permission can still require a fresh click. This change does not alter the
plaque's rendering or the original interaction/menu logic.

The final host source rebuilt and linked in Linux, WASM and sanitizer
configurations (`build/capture-restore-{linux,wasm,sanitize}.log`), with no new
warnings. The existing user-controlled client was left untouched for that build;
the user subsequently confirmed the input fixes, including interactive capture
restoration, in the Linux client.

## MAGIC bar charge-state trace

Status (2026-09-20): deferred at the user's request while they try to reproduce
the one-off empty MAGIC bar. It remains unexplained, not fixed; no further
investigation is scheduled without a reproduction or new evidence.
The bounded source/resource trace found no demonstrated port defect. No gameplay
change, new input automation, client run or test campaign was made for this step.
Reviewer and holistic reviews found no must-fix factual or scope issue.

The charge field is distinct from MP. `player_update` admits ordinary refill when
the update reaches gameplay, body armor is not Skull Armor, no new Magic-button
edge is being handled, and a spell other than `KF_MAGIC_NONE` is selected. The
gain is `2 * (((magic * 64) / (charge_rate + 1)) + 1)`, with integer division and
a clamp to 5,000 after the original halfword assignment. The retail checks and
operations at GAME.EXE `0x8001913c`, `0x800193cc..0x80019430` and helper
`fixed6_ratio_step` at `0x80015128` support that path directly.

The extracted COM.DAT's offensive-spell charge rates are 70, 28, 45, 20 and 14
for IDs 4–8. With normal magic power 0–999, eligible gain is at least two and at
most 8,526; adding it to a normal charge of 0–5,000 cannot overflow the halfword.
Starting magic power is 20, and Light Needle gains 172 per eligible update,
reaching full charge in 30 such updates from zero. These are calculations from
the actual resource table, not measured wall-clock recharge timings. The ignored
`build/magic-charge-resource.mjs` and `.log` retain the table read and COM.DAT hash.

The surrounding paths were checked as well:

- A successful cast deducts MP and clears charge. Insufficient MP does neither;
  MP is not a gate on the later refill branch. Holding Magic does not repeatedly
  take the pressed-edge branch: the original previous-input snapshot is updated,
  and the host supplies held buttons rather than repeating key-down events.
- `player_select_magic` clears charge and updates both the ID and record pointer;
  None selects a null pointer. Its three original callers are initialization,
  load-return and confirmed equipment-menu spell selection. Canceling that menu
  does not call it. The ordinary menu-use spells (healing/status effects) spend
  MP without replacing the equipped spell or clearing this charge field.
- Save serialization explicitly stores the charge and spell ID, never the record
  pointer. Applying a validated save rebuilds the pointer from the current magic
  table. The existing load-return path then reselects the spell and clears charge;
  subsequent eligible gameplay updates refill it. The table is module-owned,
  rather than a pointer retained into a discarded menu buffer.
- Death/recovery dispatch and modal loops can defer normal gameplay updates;
  they do not introduce a separate MAGIC-only timer. The HUD recomputes its width
  as `magic_charge / 100` on every enabled-gauge render, matching retail's charge
  read/division at `0x8001ffb8..0x8001ffc4`. It has no independent refill counter.

The direct named writes to `magic_charge` are initialization, spell selection,
successful casting and refill; the port also restores whole player state on
module entry or save application. This does not rule out unrelated memory
corruption, an invalid live state or a rendering problem. None/Skull Armor are
established gates, not established explanations of the user's particular report.

Static snapshots are in `build/magic-charge-game-*.txt`. Existing match reports
read 100% for selection, the ratio helper and `render_frame`, and 99.96948% for
`player_update`; no new match or reconstruction-source change is claimed. The
next useful evidence is the failing live session: selected ID/record, charge,
magic power, body armor, current/previous input, update state and whether the HUD
width agrees. Ask the user to leave that client open if it recurs. Do not reopen
this completed static trace or reset the gauge without new evidence.

## Uninitialized gameplay inputs also present at the retail boundary

Two native compiler warnings are not explained by omitted port calculations.
The corresponding retail paths also consume values without initializing them
locally. This is a C/C++ source portability problem; the machine instructions
themselves still operate on concrete register and memory contents. The original
source spelling and intent are not established merely by matching that output.
The investigation initially left both branches unchanged. The user subsequently
approved the defined port corrections recorded below.

### Initial return-home direction

GAME.EXE `actor_update_current_action` at `0x8002fa88` dispatches action 33
through the word at `0x800125a8`, whose raw value is `0x800303cc`. Initialization
branches to `0x80030500`, where `s3` and `s2` supply the supposed home X and Z
before either has been assigned in this function. Only the subsequent
collision-clear path at `0x80030414` computes the placement coordinates, into
`s3` at `0x80030444` and `s2` at `0x80030450`.

The sole proven direct caller is `actor_pool_update`, calling at `0x8003087c`.
Its entry sets `s3=1` at `0x80030844` and `s2=65535` at `0x80030848`, for the
active-state comparison and loop decrement. The intervening actor binder and
occupancy helper do not change those registers; awareness saves and restores
them. Consequently this normal retail call path initially computes the bearing
toward `(1, 65535)`, not the actor's home. Those numbers are caller register
residue, not authored coordinates or recovered home-position constants.

The source selector can choose this action for a homebound slot; no claim is
made here about how often a particular retail scene reaches initialization.
The existing reconstruction report is 100%, but was only inspected, not rebuilt
or newly banked. Verdict: the native uninitialized locals are real; calculating
the actual home position immediately would be a deliberate behavior correction.
Preserving the demonstrated initial bearing is another possible defined port
policy, without reproducing the register machinery.

### Player-homing projectile pitch

GAME.EXE `effect_update_dispatch` at `0x80038a38` dispatches kind 20 through the
word at `0x80012d38` to `0x800396a4`. With phase greater than four and target
mode `0xfe`, the player branch calculates yaw, then loads `120(sp)` at
`0x80039760` for pitch. This path has no preceding assignment to that slot.
In the alternative actor-target branch, `0x8003977c..0x80039790` passes its
address to `actor_pool_find_target_in_cone`, which provides the distance later
read at `0x800397ec`. The player path bypasses that call. Its preceding yaw
helper does not receive a pointer to, or itself write, the slot.

The sole proven direct dispatcher caller is `effect_pool_sweep` at
`0x8003a7a4`; its loop reuses the dispatcher stack area. A stable player distance
cannot be inferred from that old stack content. The source actor-effect path
can construct player-targeted alternate homing projectiles, whose constructor
normalizes the kind to 20. This establishes a source path, not an observed
runtime frequency. The existing dispatcher report is 99.82781%, not exact and
not a fresh match. Verdict: the player pitch input is undefined in native C++;
computing the player's horizontal distance would be a deliberate correction,
not restoration of an instruction missing from the port.

Retail hashes, raw selector words, callers, relevant control flow, references,
match reports and source history were inspected. Static disassemblies are in
`build/retail-actor-action-init.asm`, `build/retail-actor-awareness-init.asm` and
`build/retail-effect-dispatch-init.asm`; the caller and helper chain is retained
in `build/retail-uninitialized-callers.asm`. That diagnostic step did not change
gameplay or run a client. Neither finding is an automatic reconstruction-branch
change.

### Approved port corrections

The user explicitly chose the actor's home coordinates and the actual player
distance rather than preserving the retail residue. The port now implements
that policy in the original functions:

- The return-home case calculates immutable X/Z coordinates with the existing
  placement helper before either branch uses them. Initialization and subsequent
  collision-clear updates therefore use the same tile/local home position.
  These locals no longer exist outside that action's scope.
- Player-targeted homing calculates horizontal distance from the player's camera
  X/Z minus the projectile's X/Z with `fixed_vector2_length`. This retains the
  game's existing downshift and sampled-square-root arithmetic; it does not
  introduce floating-point distance or a new range/cone restriction. Pitch still
  uses the original vertical offset, sign, angle mask and turn limit. The
  actor-target branch retains its own distance output from the existing search.

Other action selection, collision, animation progression, random calls, movement
speed and frame pacing are unchanged. No persistent state, gameplay replacement
or reconstruction-source edit was added. These are deliberate port behavior
corrections, not claims of recovered missing retail instructions.

Linux, WASM and sanitizer builds recompiled both affected original-source units
and linked the clients (`build/defined-inputs-{linux,wasm,sanitize}.log`). The
three uninitialized-variable warnings for `home_x`, `home_z` and
`target_distance` are absent; pre-existing enum/signedness warnings remain.
No new tests, game run or input steering were performed, so this is build and
source verification, not an observed fight or projectile-path check.
