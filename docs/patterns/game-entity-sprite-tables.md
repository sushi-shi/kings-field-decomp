# GAME floor-item and effect sprite descriptors

## Function Match Plan

At `5b5db21`, replace the unclassified `DAT_80055afc` byte view with the
two descriptor families its consumers actually address. Review the full
image-qualified six-view dossiers, calls, adjacent functions, source history,
shared `KfSpriteQuad` layout and the matching OPEN data. These are game
placement/animation/render policies; matrix and GPU calls remain authentic
SDK operations, separately attributed in the vendor inventory.

| Image / function | Hex extent; initial strict match | Evidence and first source hypothesis |
| --- | --- | --- |
| GAME `8001ed90 render_floor_item` | `14c`; 98.79518% | Typed placement pointer from `render_entities`; 80-byte frame, nine calls, six address pairs and one internal jump. Unsigned item ID plus byte frame selects a 12-byte descriptor at `80055b00`. Replace the cast/offset with typed indexing; name the facing/count fields and signed depth bias without changing arithmetic. |
| GAME `8001eedc render_actor_sprite` | `1e8`; 100% | Effect render view from the same sweep; 112-byte frame, twenty calls, six address pairs and two jumps. Mode ff selects a byte-indexed descriptor at `80055b54`; other modes select model ID plus 30. Preserve its banked body, signed scale halfwords, cache arguments and every return/delay slot. |
| OPEN `800190f4 render_floor_item` | `14c`; 98.79518% | Same placement, facing/count and depth-bias protocol; nine calls, six pairs and one jump. Share the new literal names, retaining OPEN's existing seven-record owner and separate image addresses. |
| GAME `8001e230 render_enqueue_sprite` | `250`; 93.97298% | Five callers and five SDK calls; seven address pairs. Rename the signed-halfword argument from `screen_scale` to `depth_bias` through implementation, header and identity. It is added to anchor depth before the signed threshold and masked OT index; it does not scale corners. |

Use the unchanged GCC 2.5.7 probe. Force the affected compiles, compare every
function score and all unrelated objects, and resolve complete words and
ordered calls/referents. The missing extra facing-byte mask in both floor
renderers and the sprite enqueuer's graphics-owner residue are existing
non-exact results, not reasons to add artificial locals or alter types.
Require the actor-sprite function to remain strict 100%. Compare every
initialized byte and section placement before committing; do not bank names.

## Descriptor owners and boundary evidence

`floor_item_sprites` at GAME `80055b00` contains seven twelve-byte records,
84 bytes identical to OPEN `800358e0`. Its first four frames use U
`90,b0,d0,b0`, V zero and a 1024-by-1024 local rectangle. The last three
use U `90,b0,d0`, V `20` and a 1024-by-1280 rectangle.

The five GAME `B1..B5/MIXA.DAT` placement chunks contain 22, 16, 51, 2 and
26 live records respectively. Floors 2..5 use ID zero/count four. Floor 1
also uses ID four with packed bytes `13` and `23`: its last three frames
are used with fixed yaw zero and a quarter turn. This establishes both
normal animation families. The exact seven-record logical extent combines
the independent OPEN copy, all complete record shapes, and these selectors;
it is not proof of the original array declaration.

`effect_billboard_sprites` starts at GAME `80055b54`. Twenty-two complete
records occupy `108` bytes through `80055c5c`, where the different fourteen-byte
HUD-record layout begins. Constructor/dispatch evidence supports the last
records as well as the first: kind `29` (hex) seeds billboard base 19 and
dispatch kind 32 advances/wraps three frames, reaching indices 19,20,21.
Fire Ball uses indices 0..4, Wind Cutter uses 5, and Lightning Bolt uses
6/7; the alternate lightning seed 17 alternates 17/18. Other descriptor
identities remain tied to their constructor selectors without invented lore.

The old gap began four bytes before either real table, at `80055afc`.
Those four zero bytes have no reviewed consumer and remain unclassified;
they are not invented structure padding. Five old byte-pattern "strings"
inside the effect table are texture-coordinate bytes followed by rectangle
halfwords. Reclassify them with their actual record owner. The two arrays
belong to the existing entity-rendering module as a WIP ownership model:
each has a direct consumer there and they form a contiguous data run.
Numeric retail relocation sites and destinations stay unchanged.

## Initial-frame limitation

The loader's retail `80020ca4..80020cc8` loads the full packed byte,
multiplies it by `rand()`, shifts right fifteen and stores the byte frame.
It does **not** mask the low nibble first. For a packed value `23`, a
15-bit random result can therefore seed frame 34; ID four then selects
index 38 on its first draw, beyond the seven logical floor descriptors.
The renderer draws before wrapping the incremented frame against the low
nibble. This is a potential original out-of-range access, not a justification
for making the table artificially larger or silently changing the random
calculation. No claim is made that every initial index stays within the
logical table; the typed source retains the existing indexing operation.

## Named constants and retained literals

The high nibble mask `f0` selects facing, and the low mask `0f` selects frame
count. Packed facing zero uses the view's pitch matrix. Packed `10` is the
first fixed facing (yaw zero); subtracting it and shifting six converts each
successive packed step `10` into 1024 angle units, a quarter turn. Thus
`10/20/30/40` mean 0/90/180/270 degrees. Larger high-nibble values retain
the original arithmetic and angle wrapping; no new validation is imposed.

The fixed-facing and billboard biases 150/200 are added to projected anchor
depth for ordering-table selection. The effect-model bias is 100. Their
sorting role is evidenced by the callees; the original tuning rationale
for these particular values is unknown. `depth_bias` replaces the misleading
`screen_scale` local/API argument name without changing signed storage.

Array extents 7/22 and all 232 descriptor components remain literal authored
data: four unsigned texture coordinates/spans followed by four halfwords
for local X/Y and width/height. Hex negative X/Y patterns are narrowed to
signed corner coordinates by the existing renderer; do not normalize or
round the asymmetric rectangles. The descriptor stride of twelve disappears
from executable source arithmetic through typed indexing, as do the fake
blob offsets four and `58`.

Other remaining literals in the two entity-renderer functions have specific
roles: zero is the no-facing predicate, animation reset, first TMD object,
zero depth bias or unboosted depth-cue flag; one increments the animation
counter or enables the existing depth-cue boost. Byte mask `ff` preserves
wrapping before the frame-count comparison. These mathematical/boolean/API
origins stay literal. SDK vector/matrix casts outside the table view remain
separate boundary work; this campaign does not claim those types are closed.

## Final verification

| Function | Final strict match | Before/after raw comparison |
| --- | --- | --- |
| GAME `render_floor_item` | 98.79518% unchanged | All 82 source words, nine calls and six address pairs agree; retail still has the extra facing-byte mask. |
| GAME `render_actor_sprite` | 100% unchanged | All 122 retail words, twenty calls and six address pairs agree, including both internal jumps and the restoring return slot. |
| OPEN `render_floor_item` | 98.79518% unchanged | All 82 source words, nine calls and six address pairs agree. |
| GAME `render_enqueue_sprite` | 93.97298% unchanged | The complete object is byte-identical; renaming the bias does not affect generated code. |

Both new arrays reproduce all 348 retail bytes. The independent strict data
check also accepts their extents, linkage, relocation state and section
placement. This adds one fully matching data-owning unit: GAME now has 9/42,
with the same 33 divergent units. The preceding four zero bytes remain
unclassified; no array overlap or fabricated padding is introduced.

All 112 objects and 484 scores were checked against `5b5db21`. This batch
changes GAME entity-renderer data/symbols/REL addends and OPEN debug lines.
The concurrent `735e0b0` actor change separately improves GAME `8002fa88`
from 98.93894% to 100%. An independent compile of the literal old actor source
under the new headers preserves its code, data, runtime symbols and their
relocations; only debug/file metadata differs. Every other score is unchanged.

All 657 tests pass without skips, as do inventory, Ruff and whitespace checks.
The inventory has 2,954 data identities, 254 named. The existing count control
is updated for the two new owners, and two current evidence signatures follow
the corrected bias parameter name. No tests or static size assertions are
added. The modern compiler reports the same multiset of 320 errors, with
64/112 source/image variants passing.

The full build remains incomplete on the existing ownership/placement work:
PSX data checks 0/1, GAME 9/42 and OPEN 2/19; target relinks 1/1, 75/77 and
34/38; six section-base conflicts and no artifact failures. GAME's reached
data changes from 94 source-owned / 343 config-only ranges to 96 / 342.
Neither the new data match nor the table names establish safe initial-frame
bounds or historical compiler/TU attribution. No new function is banked by
this campaign.
