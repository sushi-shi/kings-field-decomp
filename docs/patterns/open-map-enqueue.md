# OPEN map polygon emission

OPEN `render_enqueue_map` at `0x800185e8` has a complete C reconstruction
under `probe-gcc257-o2-g0`, with **97.449580000% strict objdiff**. It emits
960 bytes against 952 retail bytes. It is not banked and is not a proven
compiler wall. Its sole caller, `render_map_cell`, remains at 84.094826000%.
Per-function evidence is in `open_semantic_map_enqueue.tsv`.

## Supported behavior and owners

The routine gets one TMD object, computes its normal-data base, projects its
word-sized vertex count, and then reloads the selected asset to locate the
primitive stream. A word-sized postdecrement count controls traversal. Modes
`0x24` and `0x2c` emit authentic SDK `POLY_GT3` and `POLY_GT4` packets of
40 and 52 bytes respectively; other modes are skipped. Prepared halfword
indices are byte offsets into eight-byte projected-vertex records.

Both paths reject nonpositive `NormalClip` results before calling OPEN's
primitive allocator. There is no allocation-null check or GAME-style inline
cursor/end test in this body. Packed coordinates are copied as words and UV
pairs as halfwords. Each face normal drives `NormalColorCol`, followed by
three or four `DpqColor` calls using signed per-vertex `p2` halfwords.

The colour input is one initialized `CVECTOR` at `0x800372f8`, containing
`80 80 80 00`. Retail materializes its command byte at `0x800372fb`, writes
the GPU code there, and passes that pointer minus three to `NormalColorCol`.
The former one-byte interior identity is now the full four-byte owner,
`map_textured_primitive_color`; the encoded relocation still targets owner
plus three. The preceding four-byte unclassified interval and following
sprite normal remain separate. The claimed colour bytes match exactly;
aligned object-section padding is not an additional retail ownership claim.

Triangle depth is `((sz0 + sz1 + sz2) / 3) >> 2`; quad depth is
`(sz0 + sz1 + sz2 + sz3) >> 4`. Both add 200, compare signed depth with
16384, and mask accepted depths with `0x3fff`. There is no lower bound.
Triangle division retains the MIPS checked-division sequence. After the
four-byte header, all paths advance by `(header >> 6) & 0x3fc`.

All 18 direct calls are accounted for. The GTE helpers have separately
documented LIBGTE SMP lineage; packet helpers have LIBGPU PRIM evidence.
The map traversal, allocation policy and depth rules are game-owned, not
vendored reconstruction progress. The matching GAME interface is a behavioral
homolog, not proof of an original common translation unit.

## Comparison and remaining work

The first complete source used a single post-switch depth check and reached
95.894960000%. Retail instead checks depth inside each mode and joins at the
ordering-table load. Expressing the per-mode checks and insertions recovers
that CFG, including one emitted AddPrim tail, and reaches 97.449580000%.

The first raw difference is the saved normal pointer: retail uses stack
offset 32, the probe 24; the saved header occupies the other slot. Both
frames are 80 bytes. Immediately afterward, the probe independently
materializes the projected-array address, whereas retail derives it from
the current-asset slot plus 32. Retail later loads the ordering-table pointer
from that projected base minus 276; the source uses its supported separate
global. These two additional HI16/LO16 pairs explain 27 text relocations in
the source versus 23 in the target. No synthetic retail pairs were added.

The colour owner plus-three pair, current-asset plus-eight pair, ordered call
set, mode branches, clipping/depth guards, packed widths, numeric constants,
and return delay slot agree. Remaining symptoms include counter/base register
roles, commutative addition operands, triangle packet register identity and
stack-slot placement. Wider graphics ownership/address formation is still
unresolved, followed by unattributed codegen differences. Neither the exact
historical compiler nor a backend mechanism has been proved; no declaration
permutation or fabricated aggregate is justified by this score.

## Shared packed-coordinate correction

The existing shared `KfScreenVertex.sxy` field was a `DVECTOR`, while every
perspective writer passed its address as SDK `long *` and renderers read it
as a packed word. The canonical field is now `long`, preserving the authentic
SDK interface and signed halfwords at offsets four and six. The two
non-perspective paths retain independent halfword stores through a `DVECTOR`
view. Pinned layout tests check target long size/alignment, record offsets,
scalar reads/writes, GPU/TMD layouts, and an intentionally wrong packet size.
The inventory checker now understands target O32 `long`, independent of the
host ABI.

All eleven affected existing functions retain their scores, including all
five exact functions. Before adding this map renderer, the four reconstructed
objects were byte-identical after a real rebuild; instructions, symbols and
relocations are therefore unchanged. Baseline `db05496` and after hashes:

| Object | SHA-256 before and after |
| --- | --- |
| OPEN `80016d38_render.o` | `587e6c2f908d581607cd2de0dd9c6968ce9d4ee47165acc62262c76f082e88e3` |
| OPEN `80018344_render_unlit.o` | `b066d85149519f0d22b7622878023e3fb21ced905a2c32368d6f9a3f47a2a2d4` |
| GAME `8001b7b0_render.o` | `ac318ec0fc3908796b1c445d47c16afc3c2182fe8a56b68d17ada771ed69b7d4` |
| GAME `8001c7f8_render_enqueue.o` | `58e9f0d38105c776d3c7a79b95e43618341c5a1015a7c46f7d54b2802903b3a4` |

`game_open_semantic_packed_projection.tsv` records individual snapshots and
verdicts. This removes 117 pointer casts net from existing consumers; the
map renderer adds 43. The total falls from 881 to 807 without changing a
cleanliness floor. These controls are not new GAME matching progress.

## Verification checkpoint

The full three-image build passes with all 59 data-owning units exact and
all 348 previously exact functions retained. OPEN strict checking passes;
its started count moves from 100 to 101 of 122 eligible functions, with
87 exact. Repository lint and 348 tests pass (seven optional tests skipped).
Only the five exact projection controls are re-banked, not the map renderer
or its partial caller.

The all-image historical strict audit still flags four pre-existing GAME
partial rows: `render_enqueue_model`, `render_enqueue_map`, `render_entities`
and `notify_effect_update`. Every existing function's size and score was
compared with the pre-edit reports and is unchanged; these historical
high-water discrepancies were not introduced or re-banked by this campaign.
