# OPEN unlit triangle emission

OPEN `render_enqueue_unlit_triangles` at `0x80018344` has a complete C body
under `probe-gcc257-o2-g0`, matching **93.455620000%** in strict objdiff.
The probe emits 692 bytes against 676 retail bytes. This is neither banked
exact code nor a proven compiler wall. The per-function evidence and exact
caller control are in `config/evidence/open_semantic_unlit_triangles.tsv`.

## Supported behavior

The sole caller is `opening_entity_render`: object cases 26 and 27 select
depth biases 1000 and 10000 after projecting the selected object's vertices.
The renderer walks a word-sized primitive count using a postdecrement loop.
It supports TMD modes `0x20` (F3) and `0x24` (FT3), rejecting nonpositive
`NormalClip` results before allocating a GPU packet.

FT3 takes texture coordinates, CLUT and texture page from the prepared TMD
packet, but RGB from `floor_item_state.material.color`. F3 takes packet RGB.
Both paths copy projected coordinates as packed words; FT3 also copies each
UV pair as a halfword. The source uses authentic SDK `POLY_F3` and `POLY_FT3`
types, with pinned layout checks and an intentionally wrong-size negative
control in `tests/test_open_unlit_triangle_inventory.py`.

The shared tail sums signed halfword depths, performs checked signed division
by three, shifts right by two, and adds the signed halfword bias. It enqueues
only depths at least five, indexing the ordering table with `depth & 0x3fff`.
It has neither an upper-depth rejection nor an allocation-null check. After
consuming the four-byte header, the next packet advances by
`(header >> 6) & 0x3fc`, including skipped modes and rejected triangles.

There are eight direct calls, all accounted for. NormalClip and the LIBGPU
packet helpers have separate SDK attribution; the surrounding TMD/material
policy is game-owned. This is not GAME's `render_enqueue_model`: that renderer
has additional shading and mode behavior. No lighting or fog calls belong
in this OPEN body.

## Shared-base ownership remains unresolved

Retail materializes the current-asset slot at `0x80069b60`, then adds 32 to
obtain the projected-vertex base. That same base also addresses:

| Relative address | Retail identity |
| --- | --- |
| `0x80069b80` | Projected vertices |
| `+16044..16046` | Material RGB at `0x8006da2c..0x8006da2e` |
| `-276` | Ordering-table pointer at `0x80069a6c` |

These physical relationships support investigating a wider original graphics
object, but do not establish its complete extent, declaration, or original TU.
The retained source keeps the existing supported typed owners. It does not
invent out-of-bounds aliases to force the shared-base instruction sequence.

Consequently, the source object has 17 text relocations versus 11 in the target:
three additional absolute HI16/LO16 pairs materialize the projected vertices,
material, and ordering-table owners independently. The target has eight calls,
one internal jump, and one HI16/LO16 pair. Base-relative accesses do not justify
inventing extra retail relocation entries.

## First comparison and retained differences

Both versions use a 64-byte frame. The first divergence is the incoming bias:
retail retains it in a saved register, while the probe stores a halfword on
the stack and later reloads/sign-extends it. The independently materialized
global bases also change saved-register use and instruction order; vertex
pointer registers and commutative addition operand order differ.

Calls, packet dispatch, field widths, depth policy, and numeric referents are
accounted for, but address formation is not yet equivalent. Classification
therefore starts with unresolved wider data ownership/shared-base formation,
followed by unattributed register/order symptoms. It is not a demonstrated
compiler mechanism, and no permutation experiments were used.

## Cast-only controls

The new renderer requires 34 pointer casts for byte-offset vertex indices and
packed GPU/TMD field accesses. Thirty redundant casts were removed from five
existing GAME render consumers whose arguments already had the exact SDK
`MATRIX *` or `SVECTOR *` type. All three reconstructed objects were byte-identical
before and after this mechanical cleanup at parent `24d4c7a`; hashes and
unchanged scores are recorded in `render_typed_matrix_cast_cleanup.tsv`.

That comparison left 881 pointer casts, below the existing limit of 882,
without changing the gate or hiding casts in helper macros. The controls are
not new GAME matching progress.
