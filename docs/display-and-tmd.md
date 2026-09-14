# Display and TMD state

GAME and OPEN contain the same display-frame and TMD-access source shapes at a
constant `0x5280` address delta. This is cross-overlay lineage, not a claim
that the functions belonged to one original translation unit. The complete
per-function dossiers are in
`config/evidence/game_semantic_display_tmd.tsv`; `kf lineage` checks all 541
instructions admitted by this campaign.

GAME's reconstructed `game.render` module now owns the filled contiguous run
`0x8001b7b0..0x8001c7f8`.  The former `game.display` split ended immediately
before `render_initialize`, which `display_initialize` calls directly, and
both halves operate on the same graphics state family with the same compiler
profile.  That is enough evidence to consolidate the reconstruction unit; the
module boundary remains WIP rather than a claim about the historical filename.
The projection tail now shares this owner: the preceding accessors set its
input vertex cursor, and callers use their object counts before projection.
See [the projection ownership evidence](patterns/tmd-projection.md).

## Function identities

| Action | GAME | OPEN | Contract |
| --- | ---: | ---: | --- |
| renderer initialization | `0x8001bce0` | `0x80016908` | `void render_initialize(void)` |
| begin frame | `0x8001bfb8` | `0x80016d38` | `void display_begin_frame(void)` |
| present frame | `0x8001c050` | `0x80016dd0` | `void display_present_frame(void)` |
| select TMD slot | `0x8001c0e8` | `0x80016e68` | `void tmd_select(u16 slot)` |
| get object record | `0x8001c114` | `0x80016e94` | `KfTmdObject *tmd_get_object(u16 object_index)` |
| set vertex cursor | `0x8001c138` | `0x80016eb8` | `void tmd_set_current_vertices(SVECTOR *vertices)` |
| select object vertices | `0x8001c148` | `0x80016ec8` | `void tmd_select_object_vertices(u16 object_index)` |
| set view transform | `0x8001c184` | `0x80016f04` | `void render_set_view_transform(const VECTOR *, const SVECTOR *)` |
| prepare primitive indices | `0x8001c2b0` | `0x80017030` | `void tmd_prepare_primitive_indices(void)` |
| register TMD | `0x8001c5b0` | `0x80017330` | `void tmd_register(u16 slot, u8 *tmd)` |
| release last allocation | `0x8001c5ec` | `0x8001736c` | `void tmd_release_last_allocation(s32 slot)` |

`display_begin_frame` toggles a byte selector, selects a 0x0c-byte primitive
buffer, derives its ordering-table pointer, calls `ClearOTagR` for 0x4000
entries, and resets the record's cursor to its start. `display_present_frame`
then calls `DrawSync(0)`, `VSync(0)`, selects `DRAWENV` and `DISPENV` records
with exact strides `0x5c` and `0x14`, and submits `ordering_table - 4` through
`DrawOTag`.

The view function treats position and rotation as independently optional. It
copies a 16-byte `VECTOR` and an eight-byte `SVECTOR`, derives the two signed
view-cell coordinates, and calls `RotMatrix` twice.

## Complete structures

```c
typedef struct KfPrimitiveBuffer {
    u8 *start;   /* +0x00 */
    u8 *end;     /* +0x04 */
    u8 *cursor;  /* +0x08 */
} KfPrimitiveBuffer; /* 0x0c */

typedef struct KfTmdObject {
    u32 vertex_offset;     /* +0x00 */
    u32 vertex_count;      /* +0x04 */
    u32 normal_offset;     /* +0x08 */
    u32 normal_count;      /* +0x0c */
    u32 primitive_offset;  /* +0x10 */
    u32 primitive_count;   /* +0x14 */
    s32 scale;             /* +0x18 */
} KfTmdObject; /* 0x1c */
```

`KfPrimitiveBuffer` is shared by the two overlays through
`include/kf/lib/render_types.h`. `KfTmdObject`, the TMD header and primitive packet
bodies, and the shared projected-vertex record are checked definitions in
`include/kf/lib/tmd.h`, not inferred names in prose. Overlay-specific display,
TMD-registration, and render-state aggregates live in `game/render.h` and
`open/render.h`.
`config/retail/structures.tsv` records the complete sizes and
`structure_fields.tsv` records every field offset, size, type, confidence, and
evidence.

`KfPrimitiveBuffer` is supported by more than adjacency. Each initializer
writes two complete `start/end` ranges, and frame begin selects a record with
`index * 0x0c` before copying `start` to `cursor`. The buffer byte spans differ
by overlay (`0x19640` in GAME and `0x26160` in OPEN), while the record layout is
identical.

`KfTmdObject` follows the standard PlayStation TMD object-table layout. The
lookup function skips the 0x0c-byte TMD header and multiplies the object index
by `0x1c`. Vertex selection reads `vertex_offset` at `+0x00`. The preparation
pass reads `primitive_offset` and `primitive_count` at `+0x10/+0x14`, walks
variable-size primitive packets by their header length, and shifts selected
16-bit vertex/normal indices left by three so they become offsets into
eight-byte vector arrays. Its indirect jump table remains explicitly modeled;
no callee is invented for that `jr` dispatch.

## Image-qualified state

| Object | GAME | OPEN | Type |
| --- | ---: | ---: | --- |
| display selector | `0x80070e98` | `0x80049a48` | `u8` |
| primitive buffers | `0x80070ea0` | `0x80049a50` | `KfPrimitiveBuffer[2]` |
| current primitive buffer | `0x80070eb8` | `0x80049a68` | `KfPrimitiveBuffer *` |
| ordering table | `0x80090ebc` | `0x80069a6c` | `u32 *` |
| draw environments | `0x80090ec0` | `0x80069a70` | `DRAWENV[2]` |
| display environments | `0x80090f78` | `0x80069b28` | `DISPENV[2]` |
| TMD slots | `0x80090fa8` | `0x80069b58` | `u8 *[8]` / `u8 *[2]` |
| current asset/TMD payload | `0x80090fc8` | `0x80069b60` | `void *` |
| current TMD vertices | `0x800910bc` | `0x80069b68` | `SVECTOR *` |
| view position | `0x80095744` | `0x8006e0ac` | `VECTOR` |
| view rotation | `0x80095754` | `0x8006e0bc` | `SVECTOR` |

The TMD slot count is genuinely overlay-specific. GAME has eight pointers
before the current payload, whereas OPEN has two. The shared payload retains
the shared `tmd_state.current_asset` field, also used by the reconstructed
asset registry; the TMD interpretation does not introduce an overlapping
global or a separate payload owner.

GAME's only caller passes slot `4` to `tmd_release_last_allocation`, but the
wrapper itself never reads `$a0`; OPEN has no confirmed caller. The parameter
is therefore retained as a candidate `s32 slot` call contract rather than
erased from the signature or assigned a stronger semantic role.

## `VSync` exclusion boundary

The function at GAME `0x800555e0` / OPEN `0x800352c4` implements the public
mode-dependent `VSync(int mode)` contract and is followed by a private
0x1bc-byte hardware/root-counter worker. Both pairs preserve every instruction
shape at delta `0x2031c`. They lie after the exact `LIBGPU/SPRINTF` contribution
and three small libc members, contain the embedded `vsync.c` revision string,
match the Release 2.5 `LIBGPU.H` API semantics, and agree with the Release 2.5
`VSYNC` FID corpus's object family and order. Their bytes and boundaries differ
from the pinned Release 2.5 object, so all four rows use
`sdk-lineage-supported`, not an exact confidence. They are excluded from the
game denominator and are not reconstruction targets.

## Remaining questions

- Prove the exact SDK revision that produced the two-function retail VSYNC
  shape.
- Recover historical TU boundaries beyond the supported reconstruction
  grouping; adjacency and overlay lineage alone are insufficient.
- Refine the selected payload identity if later asset-registry work proves a
  narrower TMD-specific owner without creating overlapping globals.
- Reconstruct a representative TMD function batch before attributing any
  recurring compiler scheduling residue.
