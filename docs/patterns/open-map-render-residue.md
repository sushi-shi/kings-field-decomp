# OPEN map rendering: ownership and setup residues

## Camera narrowing control at `4199d60`

The six semantic views, fresh focused compile, exact traversal caller,
adjacent sprite, SDK transform bodies/headers and source history were refreshed
for OPEN `80018bbc`/464 bytes, strict 87.922420%. The complete owner already
recovers the matrix-base chain; the candidate remains 472 bytes, with nine
calls and nine address pairs. No signature, grid or orientation-CFG change
is proposed.

The three camera coordinates belong to a real SDK VECTOR, but the destination
is SVECTOR. Test removing only the explicit `(u16)` casts, allowing the actual
halfword assignments to narrow the differences. The focused disassembly and
ordered relocation listing are identical, including all three `lhu` loads,
the unmatched constant-one placement and coordinate schedule. Thus these
loads alone do not establish explicit source casts. Restore the original
wrapped-coordinate spelling and actually recompile it; the exact traversal
is preserved. This control supplies no new exact function or bank entry.

## Function Match Plan: orientation fall-through (`eaf83a3`)

OPEN `80018bbc` remains 464 retail/472 probe bytes, strict 87.922420%,
with 81 unequal aligned words. The six semantic views, full CFG, exact
320-byte traversal caller, preceding sprite, vertex selector, shared layouts,
GAME source homolog, history and SDK boundary evidence were refreshed.
Nine calls, nine address pairs, one internal jump, no strings/candidates,
the 80-byte frame and the widths/constants below retain their contract.

At `80018cdc..80018cec`, orientation 2 adjusts X and falls into the Z
adjustment shared with orientation 1; orientation 3 adjusts only X. Test
expressing that common tail as a switch with case 2 falling through to case 1
and a separate case 3. Preserve all coordinate calculations, stores, SDK
calls and the caller; this is a control-flow hypothesis, not a case-order
or register permutation search.

The switch grows the body to 516 bytes. It compares orientation 2 first,
introduces a signed less-than split and extra jumps, and reloads position Z
from the stack before adjustment. Retail has none of these operations.
All nine calls and numeric data targets agree, and the traversal remains
raw exact, but the CFG contradicts the proposed switch. The canonical
if/else body is left unchanged; do not build further trials on this rejected
control shape. There is no new exact result or bank entry.

## Function Match Plan: relative-coordinate values (`6434803`)

OPEN `80018bbc` remains 464 retail/472 probe bytes, strict 87.922420%.
The six semantic views, full CFG, exact traversal caller, preceding sprite
body, vertex selector, shared types, GAME homolog, source history and SDK
matrix prototypes/provider evidence were refreshed. The 80-byte frame,
nine calls, nine address pairs, internal orientation join, no strings or
candidate references, argument widths and constants retain the contract below.
The traversal bounds both full-width coordinates to 0..99; its visibility
argument and this body's wrapped object selection are unsigned bytes.
The surrounding grid/object policy is game-owned, not an SDK body.

The first non-target-address difference remains the constant-one branch slot.
In the coordinate body, retail retains the full relative X and Z subtraction
results in `a2`/`a1`, stores their low halves, and later adds orientation
offsets to those same full values (`80018cdc..80018cf8`). Current C instead
expresses the later additions as reads of the narrowed SVECTOR members.
Test explicit signed word-sized relative X/Z values feeding both the initial
stores and orientation additions. This is distinct from the rejected world-
product staging and in-place world-position trials: no extra world stores,
fake carriers, SDK object, padding or changed declaration order is involved.
Preserve the initial X/Z/Y store order and all orientation branches. Compare
the complete raw body/referents and keep the 320-byte traversal exact.

The explicit relative values emit the identical 472-byte candidate: 81
unequal aligned words, nine ordered calls and nine address pairs. The
320-byte traversal remains raw exact. Remove the extra locals; this trial
does not explain the constant-one placement or coordinate instruction order.

## Function Match Plan: halfword coordinate construction (`d1d9562`)

OPEN `render_map_cell`, `80018bbc`/464 bytes, is now strict 87.922420% under
`probe-gcc257-o2-g0`, with 472 probe bytes. The complete graphics owner has
already recovered the quadrant-to-light-matrix base; the older separate-owner
discussion below is historical. All six semantic queries, full body/CFG, sole
exact traversal caller, adjacent sprite boundary, SDK signatures and provider
ledger, source history, shared grid types and GAME homolog were inspected.
Nine direct calls, nine address pairs and one internal jump have validated or
proven referents; no strings or candidate outgoing references occur. The
80-byte frame and SDK local objects remain correct. Retail's first difference
is the hoisted constant one in the object-bounds branch slot; later coordinate
loads/arithmetic have a different dependency schedule.

The caller bounds row/col to 0..99 and supplies an unsigned visibility byte.
Attribute selection wraps in a byte before checking 99, orientation is a
word, height-grid loads are unsigned bytes, and the local SDK position is
three signed halfwords. Retail computes both X/Z products before loading
their camera halfwords. A previous experiment with separate full-width world
locals emitted identical code. Test constructing world coordinates in the
existing SVECTOR and then subtracting the unsigned camera low halfwords in
place. This checks the actual halfword object update, not new full-width
carriers; no padding lane is initialized and no extra object is introduced.
Keep the orientation CFG, data owners, SDK calls and exact traversal unchanged.
Do not change GAME, declarations or compiler flags to compensate for results.

The in-place halfword construction keeps three preliminary world-coordinate
stack stores that retail never makes, then stores all three relative results
again. The candidate grows to 484 bytes and changes argument retention from
+0x14; all nine numeric call targets and address destinations remain correct,
but data-reference order changes. The 320-byte traversal remains raw exact.
The trial is removed: the earlier combined expressions remain the supported
source, and no new exact result is claimed from this experiment.

Witnesses: OPEN `render_map_cell`, `0x80018bbc`/`0x1d0` bytes, and
`opening_render_map_cells`, `0x80018d8c`/`0x140`, under the existing
`probe-gcc257-o2-g0` profile. These observations do not identify the
historical compiler or prove an optimizer mechanism.

## Visibility-window ownership

`render_initialize` passes `0x800439d8` to `cd_file_load_into` for
`B0\\RTBL.`. The file contains exactly `0xcc0` bytes, SHA256
`b085bf1fbe30831d084f21d0ba52af1609ee6f721a6f9a1937a305f97cb61e72`.
Sixteen records of `0xcc` bytes each contain four little-endian halfwords
and 196 visibility bytes. All widths/heights are 14; origins vary with yaw;
cell values are 0, 1, or 2. The selector computes
`(15 - (view_rotation.vy >> 8)) * 204`. This supports the existing GAME
`KfCellWindow` layout, now declared in the shared render-type header.

The map pass publishes `const KfCellWindow *active_cell_window` at
`0x8006e1c8`; both map traversal and entity/item culling read it. The file
proves the logical table extent, not an original C file boundary.

## Conflicting matrix-base evidence

The per-cell renderer establishes `s0 = 0x8006e0c8` at
`0x80018d2c/0x80018d30`: `render_state.quadrant_matrices`. At `0x80018d54`
it adds `0x80` to that saved base. The `SetLightMatrix` call at
`0x80018d58` adds orientation times 32 in its delay slot, reaching
`0x8006e148 + orientation * 32`, the light-quadrant array.

The separate-array source emits another HI16/LO16 pair for that address
and uses different saved registers. This is a referent-expression and
ownership discrepancy before any explanation of register selection.
Numeric targets and the complete call set agree.

A controlled experiment extended `KfRenderStateOpen` by the four light
matrices at offset `0x100`, updating both the cell renderer and initializer
to use that one field. It reproduced the cell renderer's saved-base chain.
However, the exact initializer then replaced four independent HI16/LO16
pairs with offsets `192`, `224`, `256`, and `288` from its light-matrix
base and changed call delay slots. The aggregate did not jointly explain
both witnesses under this probe and was reverted.

The split storage model remains provisional. Do not introduce incompatible
per-file views, cross-object pointer arithmetic or aliases to reproduce
one witness. Further progress needs source-ownership or compiler/linker
evidence explaining both forms. Retained per-cell C is 84.094826%, with
coordinate temporary/load scheduling also different from retail. This is
not a proven register or scheduler wall.

## Traversal setup

Retail stores the selected pointer at `0x80018de4`, derives its cell
pointer at `0x80018de8`, then reloads the selected pointer at
`0x80018dec/0x80018df0` before loading view-cell coordinates and subtracting
window origins.

The probe eliminates that reload and hoists the two view-cell loads above
window-index arithmetic. Moving the C cell-pointer initialization before
coordinate initialization, following retail order, produces the same
instructions. Compiled code is eight bytes shorter. The row/column loops,
masks, skipped-row stride, nonzero-cell test and calls agree after the
shifted setup. Strict objdiff is 84.062500%.

This setup is an unattributed codegen residue. No volatile pointer,
redundant operation or forced call was added. Both functions remain
unbanked; their individual verdicts are in
`config/evidence/open_semantic_map_render.tsv`.

## Coordinate construction follow-up plan

At `811213b`, the six image-qualified queries were refreshed for both
functions, with their sole callers, adjacent boundaries, current C/history,
shared `KfCellWindow` definition and GAME sibling checked. Scores remain
84.094826% (cell) and 84.062500% (traversal). The cell body has a frame of
80 bytes, nine direct calls, nine reviewed address pairs, one internal jump,
no strings and no candidate references. Its col/row arguments are full-width;
the visibility and wrapped object selector are bytes, and the constructed
SDK SVECTOR uses low-halfword camera coordinates. Grid policy and the
separately identified matrix services remain the vendor negative control.

The GAME traversal's selected-pointer reload follows a real pitch-dependent
branch which OPEN lacks. Adding such a branch would not recover OPEN source.
No traversal edit is planned. The unresolved matrix-owner conflict above is
also left explicit rather than altered to improve one consumer.

The focused cell hypothesis is staged world-coordinate construction: retail
finishes both multiplications by 2000 at `80018c68/6c`, then reads the X/Z
camera halfwords at `80018c70..7c`. The current C combines each multiplication
with its camera subtraction. Test two real s32 world-coordinate values before
the existing SVECTOR assignments, retaining row/col for the height-grid
lookup. This introduces no extra storage requirement or fake carrier: both
values feed their actual coordinate differences. Compare the first divergence,
coordinate instructions, nine calls and ordered numeric targets after a real
focused compile; reject the hypothesis if it supplies no explanatory change.

The staged s32 coordinate values emitted the same instruction and relocation
stream as the original expressions, including the camera-load ordering and
matrix-base differences. Strict scores stayed 84.094826% and 84.062500%.
The trial is removed; no source, type, owner or baseline change is retained.
This rules out that staging hypothesis under the current probe, not the
possibility of another evidenced source correction.
