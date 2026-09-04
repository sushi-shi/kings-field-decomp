# OPEN visible entity/item traversal

`OPEN.EXE:0x80019240` is the 664-byte `opening_render_entities_and_items`.
Its complete C reconstruction is 92.867470% under `probe-gcc257-o2-g0`.
The original compiler and translation-unit boundaries remain unproved; this
is a documented non-exact result, not a proven compiler wall.

## Semantics and contiguous module

The sole decoded caller is the scene-0 frame wrapper at `0x80019500`, after
map traversal has selected `active_cell_window`. The function computes a
world-cell origin from the view cell minus the window origin, selects TMD
slot 1, then walks all 32 opening entity records. IDs 32 and above are skipped.
Each survivor's halfword cell coordinates are subtracted from that origin
with unsigned 16-bit wrap, bounded by window height/width, and checked against
the row-major nonzero visibility byte before `opening_entity_render` is called.

It then installs a dedicated light matrix, sets active material RGB to 180,
copies the floor-item texture selectors, and traverses the loaded item count.
Here signed world X/Z positions are divided by 2000 before the same wrapped
visibility tests. Both division sites retain the checked MIPS `div` expansion.
The loop counter is signed 16-bit with a minus-one terminator, not a clamped
unsigned count. The source adds no bounds, allocation, or rendering policy.

The module now contains the contiguous entity renderer, floor-item renderer,
and their direct traversal caller (`0x80018ecc..0x800194d8`). Their shared call
graph and object families support grouping, but not a historical file name.
The first remains 100%; the second retains its 98.795180% facing-mask residue.

## Wider floor-item state owner

The decisive chain is in the traversal's material setup:

| Retail site | Operation | Reached data |
| --- | --- | --- |
| `0x80019378/0x8001937c` | `$a0 = 0x8006da2a` | Active material texture page. |
| `0x800193b8` | `$s4 = $a0 + 22` | First item at `0x8006da40`. |
| `0x800193c0` | `$s0 = $a0 + 26` | First item's world X at `0x8006da44`. |
| `0x800194ac` | Advance item by 24 | Next `KfFloorItem`. |

`KfFloorItemStateOpen` therefore expands the earlier eight-byte material owner
into one supported `0x618`-byte BSS model at `0x8006da28`. The item module owns
it; sprite rendering and initialization use the same declaration. The fields
are material at +0, unknown six bytes at +8, source CLUT at +14, source texture
page at +16, count at +18, unknown four bytes at +20, and 64 items at +24.
The previously curated pool extent ends exactly before `DAT_8006e040`.
The two gaps remain opaque; their contents and the original full declaration
are not inferred from the match. No interior global or alternate per-file
view remains. Twenty-two reviewed references retain their numeric targets
and resolve through this owner.

The initializer proves the source texture fields: `GetTPage(1,0,0x340,0)` and
CLUT `0x7a00`. It still matches 100%, as does the sprite renderer with its
unchanged `KfSpriteMaterial` prefix. The placement loader remains 97.731480%,
with its existing first-pass temporary-register and branch-offset differences.

The independent initialized `floor_item_light_matrix` occupies
`0x800359e4..0x80035a04`, with rows `{0,0,4096}`, `{0,0,4096}`, `{0,0,0}` and
zero translation. Its SDK `MATRIX` layout is 32 bytes. The former 160-byte
coverage gap also contained LIBCD state beginning at `0x80035a04`; that tail
is deliberately excluded. The apparent second matrix xref was really an
interior reference to that later storage in the old coarse coverage model.

## First differences and bounded comparisons

The first complete body scored 88.710846%. The 48-byte frame, calls and all
physical data destinations agreed. The probe used `$s0` for the entity pointer
and `$s1` for its coordinate cursor, opposite retail, and computed visibility
origins directly in saved registers rather than through retail's temporaries.

Combining the BGR stores into one chained RGB assignment follows their
right-to-left retail order. Loading the count into the signed loop variable
and using a predecrement loop reproduces retail's `lhu $s3` / `addiu -1`
sequence instead of a temporary `0xffff` addition. Those source forms are kept.
A separate window-origin load/subtract form changed the entry register use
without reproducing retail and was discarded in favor of direct expressions.

In the retained body, the probe anchors the material/pool base at color R
(owner +4), rather than the texture page (owner +2). Its +20/+24 offsets still
reach the correct item and position fields, but an extra absolute texture-page
store and a different load schedule remain. Entry setup is eight bytes shorter;
the item-row division result also has different temporary/load ordering.
The complete function is four bytes shorter overall. Branch destinations shift
accordingly, without changing the culling or loop topology. No forced pointer,
volatile carrier, alias, dead statement or assembler bytes were introduced.

Pinned layout tests verify every state field and reject an incorrect total
size. Inventory and retail controls verify the base chain, item stride, signed
division sites, independent light-matrix extent, and all body references.
