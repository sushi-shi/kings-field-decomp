# Typed storage and cast-reduction campaign

The objective is to model every supported object through shared types and
reduce explicit casts throughout `src/` and `include/`. Generic allocation,
serialized byte streams and real numeric conversions must remain distinguishable
from pointer views used to compensate for incomplete fields or interfaces.
The initial pylibclang audit counted 1,049 written casts, including 806 pointer
casts in C files. Its per-site ledger and the generated comparison artifacts
are under the isolated worktree's ignored `build/cast-model/` directory.

Work proceeds by related object families: GPU/TMD packets, graphics state and
projection storage, menu prefixes, world/actor/effect fields, animation and
resource interfaces, and the remaining numeric/SDK/ABI boundaries. A family
is complete only after its callers and consumers use a coherent shared model;
moving casts into unchecked helper macros is not a solution.

## Function Match Plan: GAME packet views

Baseline is `c3700f8`, with a fresh retail-validated full build. That build
reports existing data-owner, reference-closure and target-section-placement
failures. It nevertheless compiles all 112 units and records the function
scores below; the failed closure checks are retained as baseline evidence.

The four functions are the contiguous `game.render_enqueue` source module.
Their complete retail disassembly, CFG/delay slots, xrefs, callees, strings,
identities and fresh match state were captured before editing. The source
history, existing GAME ownership dossiers, OPEN packet implementation, supplied
LIBGPU/LIBGTE declarations and shared packet layout controls were reviewed.
These are game packet traversal/allocation policies around vendored APIs, not
SDK implementations. No outgoing candidate references or strings occur.

| Image / address / body | Function | Before | Evidence and intended change |
| --- | --- | ---: | --- |
| GAME `8001c7f8 / f38` | `render_enqueue_tmd` | 99.314170% | Seven callers constrain unsigned object ID and signed halfword bias. Twelve packet modes, 88-byte retail frame, 45 direct calls plus switch transfer, 29-word table, per-packet allocation-overflow returns, packed XY words/UV halfwords/CVECTOR colors. Replace individual field casts with existing `KfGpu*` and `KfTmdPrimitive` members; retain every guard, call, constant, mode and loop. |
| GAME `8001d730 / 6e8` | `render_enqueue_model` | 100% | Actor caller, 96-byte frame, four textured modes and 15 direct calls. Shared material CLUT/page fields and model color are retained; use matching packed packet views without changing primitive/count/depth widths. |
| GAME `8001de18 / 418` | `render_enqueue_map` | 100% | Map-cell caller, 80-byte frame, two textured modes, projection plus 16 direct calls, depth bias 200 and per-vertex fog. Use the existing typed TMD texture/color and GPU packet fields; preserve depth arithmetic and allocation exits. |
| GAME `8001e230 / 250` | `render_enqueue_sprite` | 100% | Five callers, projected anchor/four corners, signed bias and word depth-cue flag, five SDK calls. Replace four packed XY pointer stores and destination color pointer with `KfGpuFT4` fields; address the separate source-color owner in the graphics-state family. |

`gpu_packets.h` already pairs all eight real SDK packet types with matching
packed members, and `tmd.h` owns the discriminated primitive union. Existing
GCC layout fixtures check packet sizes, alignment, offsets and a negative
wrong-offset control. Allocation cursors and prepared byte-offset streams remain
generic storage at their decoding boundaries. The initialized color at
GAME `80057b58`, switch claim and all physical referents remain unchanged.

Each focused build will be compared against the saved original object and
retail from the first actual divergence. Preserve all three banked functions;
do not attribute new residues to a compiler mechanism without evidence.
Per-function final verdicts and broader campaign coverage will be appended
after real compilation and checks, not inferred from cast-count movement.

## Function Match Plan: GAME active colour

Retail `8001e3f0..8001e42c` copies the GPU command byte to graphics offset
`241c7`, derives the four-byte colour at `241c4`, and passes it to the SDK
`NormalColorDpq` CVECTOR argument. The complete graphics owner remains the
same `249cc` bytes; only its four separate colour bytes become one authentic
`CVECTOR active_render_color`. All consumers use its r/g/b/cd members.
No narrower global or material owner is introduced. The prior separate-owner
and narrow-material negative controls remain relevant.

The sprite's evidence snapshot above covers the address-taking consumer.
The following additional functions have refreshed disassembly/CFG, call and
data xrefs, strings, identities and strict match snapshots before this edit.
They implement game scene/HUD policy around the vendored SDK. None has an
outgoing candidate relocation or a string reference affecting this change.

| GAME address / body | Function | Before | Evidence and retained behavior |
| --- | --- | ---: | --- |
| `8001e480 / 16c` | `render_screen_sprite` | 100% | HUD caller passes a complete `KfSpriteQuad`; 32-byte frame, two SDK calls, one capacity branch and one return. Three lbu/sb pairs copy the active RGB bytes. |
| `8001f218 / 580` | `render_entities` | 96.667610% | Frame caller, 48-byte frame, nine direct calls and five pool walks. Three byte stores set floor-item brightness to 180 at `241c6..241c4`; existing culling and unrelated effect views remain for their own campaign. |
| `8001fde4 / 518` | `render_frame` | 100% | Main-loop caller, shared HUD/notification material setup and render dispatch. Byte stores copy HUD brightness, then set notification brightness. Preserve all SDK/game calls, branches and referents. |

Rebuild all units affected by the shared header and compare against the fresh
baseline. Layout checks must cover the complete CVECTOR and each member offset.

## Packet and active-colour result

The retained source removes 190 C-file pointer casts. The GPU union's SDK
member remains the authentic API argument; packed fields express the actual
word XY, halfword UV and CVECTOR accesses. TMD modes select the existing
primitive union's corresponding packet, texture and colour fields. The active
colour is one CVECTOR within the original complete graphics owner, with checks
for its size and all four physical component offsets.

Fresh strict objdiff and independently linked raw-word comparison give:

| GAME function | Before / after % | Raw result |
| --- | --- | --- |
| `render_enqueue_tmd` | 99.314170 / 99.314170 | All 3,900 compiled bytes unchanged; existing non-exact result remains open. |
| `render_enqueue_model` | 100 / 100 | All 1,768 bytes and ordered references unchanged and retail-exact. |
| `render_enqueue_map` | 100 / 100 | All 1,048 bytes and ordered references unchanged and retail-exact. |
| `render_enqueue_sprite` | 100 / 100 | All 592 bytes and ordered references unchanged and retail-exact. Four XY casts remain pending. |
| `render_screen_sprite` | 100 / 100 | All 364 bytes and ordered references unchanged and retail-exact. |
| `render_entities` | 96.667610 / 96.667610 | All 1,416 compiled bytes unchanged; existing non-exact result remains open. |
| `render_frame` | 100 / 100 | All 1,304 bytes and ordered references unchanged and retail-exact. |

The four sprite XY conversions are a recorded residue of the wider campaign,
rechecked with the OPEN sibling in [projected addresses](typed-projected-addresses.md).
Replacing those scalar-output stores with direct packed members
preloads all four outputs and emits 584 bytes instead of 592; objdiff falls to
93.770270%. Calls, physical referents and CFG successor lists agree. The first
branch displacement changes at `+118`, with the first changed material-base
register at `+128`; the new schedule begins at the XY loads. Using the same
packed view for CLUT/page or a nested XY union leaves that result unchanged.
A real four-element output array changes the frame positions and emits 604
bytes. These forms are not retained. No compiler mechanism is attributed to
the residue, and no helper, fake local or volatile carrier conceals the casts.

The existing historical ownership fixtures continue to reconstruct their old
byte-field declarations explicitly. The production declaration and consumers
use the new CVECTOR; the negative controls still test the prior separate and
narrower ownership hypotheses.

The refreshed AST census covers all 112 image variants, 111 C files and 53
headers with zero parse errors or unlocated casts. Written casts fall from
1,049 to 859: pointer targets from 817 to 627, integer targets unchanged at
232. The C-file pointer subset falls from 806 to 616. This is a stage result,
not a declaration that the remaining casts are necessary or fully modelled.

Verification ran all 713 repository tests (9 skips). Two inventory assertions
still expected four separate colour fields; both were updated to the complete
CVECTOR and the field census reduced by three, then rerun. The ownership
positive/negative controls and packed SDK layout checks pass. Ruff and
`git diff --check` pass. A final full build recompiles all 35 affected GAME
units and retains all 439 exact functions across the three images. It still
fails the same pre-existing data ownership/reference closure and target
placement gates; no complete-build pass is claimed.

The next completed family is [shared menu records](typed-menu-records.md):
25 additional pointer casts and one redundant scalar cast are removed through
actual point, glyph-row and list-title members. All 36 functions in those
units retain their complete linked instructions. The cumulative C-file pointer
count is 591, down 215 from 806. Packed map fields, vector/angle interfaces,
effect records, allocation/resource APIs and the documented exact-code
residues remain part of the active repository-wide modelling objective.

The [shared effect record stage](typed-effect-records.md) then removes the
duplicate rendering layout and models SDK/Euler rotation and direction
mutation through the owning types. Eight more pointer casts and one redundant
scalar cast disappear. All 21 affected-unit functions retain their linked
words. C-file pointer casts now total 583, down 223 from 806; the remaining
vector-prefix interfaces and other families above are still open.


The [world-transform stage](typed-world-transforms.md) completes map-object
and event vectors, shares actor/effect SDK rotations and removes incompatible
angle-prefix interfaces. Its 26 pointer-cast removals bring the C-file count
to 557 (249 removed overall). All 129 affected-unit functions retain their
reference-resolved instruction streams. Packed map fields, remaining vector
interfaces, animation/projection views and exact-code residues remain open.

The [map link stage](typed-map-links.md) adds whole-record byte/word views,
gold amounts, behavior-dependent spawn values and bounded container item
arrays. It removes another 18 C pointer casts while preserving all 45 compared
function bodies. The running reduction is 267 of the initial 806 C pointer
casts; 539 remain for boundary and model review.

The [weapon render stage](typed-weapon-render.md) types the proven projection,
translation and SDK rotation fields, identifies the loader's mirrored Y angle,
and removes nine more C pointer casts. All 20 compared bodies are unchanged;
the C pointer count is 530, down 276 from the original count.

The [position-interface stage](typed-position-interfaces.md) removes the
remaining duplicate three-word position type and eighteen C pointer casts,
while retaining genuine six-byte vector types at their separate boundaries.
Map-emitter SDK inputs now have complete SDK extents. All 107 compared bodies
are unchanged; 512 C pointer casts remain, down 294 from the initial count.

The [screen-coordinate stage](typed-screen-coordinates.md) gives shared
projected vertices real packed-word and SDK halfword views, removes four
pointer reinterpretations and three redundant MATRIX casts, and preserves
all 41 compared bodies. The running C pointer count is 505, down 301.

The [packed-state stage](typed-packed-state.md) models player motion/cell and
live dialogue word views across every producer and consumer. It removes ten
more pointer casts and preserves all 71 compared functions. C pointer casts
now total 495, down 311 from the initial count; the resource, animation and
remaining renderer boundary review continues.

The [resource-boundary stage](typed-resource-boundaries.md) makes thirteen
allocator outputs use actual generic pointer storage and removes the last
incompatible-pointer diagnostic. All 25 compared functions are unchanged;
482 C pointer casts remain, down 324 from the starting count. Fixed table
copy views and the other animation/renderer families still need review.

The [record-table stage](typed-record-tables.md) gives weapon, armor and magic
storage complete entry/word views and propagates their table input types.
Seven more pointer casts disappear, with all 92 compared functions unchanged.
The cumulative reduction is 331 C pointer casts; 475 remain for the continuing
animation, SDK, generic-storage and renderer-boundary audit.


The resource pointer follow-up preserves the allocator's generic output while
providing byte/TIM cursor views of the same four-byte slot. It removes the
new editor-mode void-pointer errors and restores three explicit serialized
table conversions. All 25 compared functions remain unchanged; 478 C pointer
casts remain, 328 fewer than the starting count. The editor's older SDK
interface diagnostics are tracked separately from the clean target-C census.

The [matrix stage](typed-matrix-elements.md) replaces three whole-MATRIX
reinterpretations with the SDK's element pointers. Both units remain exact;
475 C pointer casts remain, down 331 from the start.

The [animation-vertex stage](typed-animation-vertices.md) removes nine more
casts using complete SDK/word vertex views and a record-pointer binder
result. All 51 compared functions retain their instructions; 466 C pointer
casts remain, down 340 from the starting count.

The [map-table stage](typed-map-tables.md) models five complete grids, both
resource definition tables and saved-world byte initialization. It removes
fourteen C pointer casts and one header cast, keeping all 177 compared
functions unchanged. C pointer casts now total 452, down 354 from the start.

The [TMD-header stage](typed-tmd-headers.md) types stored assets and the
registration interface. Four header/object casts disappear; eight explicit
resource-boundary conversions are added. All 76 compared functions remain
unchanged. C pointer casts total 456, down 350 from the original 806; the
net increase preserves real serialized decoding boundaries.

The [projected-address stage](typed-projected-addresses.md) removes one OPEN
sprite color cast through the shared SDK packet union, preserving all seven
compared bodies. Its controlled probes record the remaining byte-address,
container-recovery and sprite XY residues. The C pointer total is 455, down
351; allocator ownership and the final boundary audit remain active.

The [allocator-owner stage](typed-memory-owner.md) replaces the opening-only
layout with shared arena/allocation types and one 80-byte owner per image.
All 33 compared bodies remain exact; C pointer casts total 454, down 352.
The controller takes the typed allocation subobject directly, while its
container recovery retains the documented exact-instruction residue.

The [redundant-cast cleanup](typed-redundant-casts.md) removes one pointer
cast and six redundant scalar casts left by earlier type recovery. All 27
compared bodies are unchanged. C pointer casts total 453, down 353; the
remaining complete workspace boundaries are reviewed next.

The [ordering-table stage](typed-ordering-index.md) replaces byte scaling
with typed element indexing. Both pointer casts disappear and all sixteen
compared bodies remain unchanged; 451 C pointer casts remain, down 355.
