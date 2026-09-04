# OPEN projected sprite rendering

`OPEN.EXE:0x800189a0` (`0x21c` bytes) is `render_enqueue_sprite`.
The complete C reconstruction reaches strict objdiff 100% with
`probe-gcc257-o2-g0`; raw instruction and ordered relocation comparison agrees.
This does not prove the historical compiler or original translation unit.

## Retail evidence and data ownership

The only decoded caller is `render_floor_item` at `0x800191f8`, passing a
12-byte `KfSpriteQuad`, a depth bias of 150 or 200, and flag 1. The callee's
halfword sign extension establishes `s16` bias independently of those values.
It projects the origin for depth, projects four signed-short corner vectors,
allocates a 40-byte SDK `POLY_FT4`, shades it, and inserts it into the ordering
table only when biased depth is at least five. Flag 1 adds half the signed
perspective term before depth cueing. The table index is masked to 14 bits.

The SDK types explain packed word stores to the four GPU coordinate pairs.
The unused origin-screen output is a `long`, as required by `RotTransPers`;
there is no evidence requiring a separate `DVECTOR` view of that output.

The eight-byte `KfSpriteMaterial` prefix at `0x8006da28` is now owned by
`floor_item_state.material`; the later
[traversal pass](open-entity-traversal-residue.md) proved the pool-base
relationship and expanded the original standalone material working model:

| Offset | Type | Evidence |
| --- | --- | --- |
| 0 | `u16 clut` | Halfword copied to `POLY_FT4.clut` at packet offset 14. |
| 2 | `u16 tpage` | Halfword copied to packet offset 22. |
| 4 | `CVECTOR color` | The CLUT base in `$a1` advances by four in the branch delay slot at `0x80018b40`, then reaches `NormalColorDpq`. |

The RGB bytes are written together by `opening_render_entities_and_items`
(`0x80019360..0x80019374`, value 180), cleared by `func_80014e28`
(`0x80015160..0x80015174`), and incremented together toward 255 by that same
ending function (`0x80015438..0x80015470`). The sprite renderer copies the
initialized primitive command byte into color offset 3. This supports the
record beyond mere address adjacency; six separate field identities are
replaced by one owner. Fourteen reviewed material reference pairs retain
their actual interior target addresses and resolve as owner plus addend.

The other owned datum, `render_sprite_light_normal` at `0x800372fc`, is the
eight retail bytes `00 00 00 00 00 10 00 00`, an SDK `SVECTOR` initialized to
`{0, 0, 0x1000, 0}`. Its sole reference is this shading call. No neighboring
color, padding, or later texture state is absorbed into either owner.

The function is game-owned. Its GTE callees have separate LIBGTE SMP/CMB
archive-order and header evidence; `SetPolyFT4` and `AddPrim` lie in the exact
LIBGPU PRIM archive span. GAME's `0x8001e230` has the same sprite semantics
but different inline primitive allocation. Neither that similarity nor the
single-function reconstruction proves an original object-file boundary.

## Focused source comparisons

The first complete body scored 90.525925% in strict objdiff. Calls, ordered
referents, and CFG agreed, but it retained biased depth in an extra saved
register and scheduled texture loads/copies differently.

1. Keep the projected depth unchanged and use `otz + depth_bias` in the
   threshold and index expressions. The retail likewise uses a transient
   argument register for the sum. This removes the extra save and restores
   the 136-byte frame and projection-call schedule.
2. Assign CLUT and texture-page selectors together before copying packed
   coordinates. Retail reads the page before its first coordinate store,
   although it delays the page store. This ordering reproduces every
   remaining instruction, including its load and branch delay slots.

These are ordinary complete source expressions, not forced carriers or
instruction padding. No unresolved codegen residue remains for this function.

## Checks and unchanged cleanup controls

`tests/fixtures/open_sprite_material_layout.c` checks the record and authentic
SDK layouts with the pinned compiler. An incorrect expected size is a negative
control. Inventory tests cover the complete ordered body referents, all
material references, the retail base-plus-four chain, and the normal bytes.

Five unnecessary TIM pointer casts were removed from already-exact OPEN code:
three conversions from existing `void *` values in `opening_run`, and one
upload conversion in each of the scene-3 and ending resource loaders. The
latter locals now use the `u_long *` type required by their sole consumer,
`tim_upload_images` / SDK `OpenTIM`. Their generic allocator boundary remains
explicit. Whole rebuilt objects have identical before/after hashes, recorded
with each function's verdict in `open_semantic_sprite_render.tsv`. All three
remain strict 100%; the existing pointer-cast ceiling is unchanged.
