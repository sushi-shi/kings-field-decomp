# OPEN map polygon emission

## Function Match Plan: case-local vertex lifetimes (`d1d9562`)

OPEN `800185e8` starts this follow-up at strict 99.663864%, 952 bytes under
`probe-gcc257-o2-g0`. The six semantic queries and full retail CFG, sole
map-cell caller, adjacent unlit/sprite boundaries, accessor and allocator,
shared layouts, SDK declarations/provider ledger and source history were
rechecked. Eighteen direct calls, two validated address pairs, one internal
jump, no strings/candidate references, the 80-byte frame and every delay slot
retain the contract described below. The caller masks its object to eight
bits before the supported unsigned-halfword API; prepared vertex indices are
unsigned halfword byte offsets into the shared projected array. This remains
game-owned packet traversal around separately attributed SDK operations.

All vertex pointers die within the selected polygon case, before the shared
packet-advance label. The fourth exists only for the quad path and is formed
after clipping; the current C nevertheless declares all four function-wide.
Test narrowing each pointer to its actual case scope, keeping the existing
case-local depth and SDK packet pointers. This is a source-lifetime correction,
not a declaration-order search. Do not change statements, call order, packet
types, common-tail spelling, shared owners, compiler options or GAME. Compare
the earliest normal-pointer spill difference and the triangle packet register
after a real focused build; retain only a supported source improvement.

Separating all vertex locals grows the body to 960 bytes: both clipping paths
lose retail's paired index loads, adding a load-delay nop per case, and use
different vertex registers. All 18 numeric calls and two address pairs remain,
but this does not reproduce the retail dependency schedule. Restore the three
shared clipping vertices. The fourth vertex is a distinct lifetime: quad-only,
computed after the clipping call and consumed through the fourth fog/depth
lane. Test only that narrower scope while retaining the shared first three.

The fourth-only scope emits the original 952-byte candidate and exactly the
same 20 unequal words. Both scope trials are removed. The retained source,
strict 99.663864% result and numeric referents are unchanged; this follow-up
does not supply a new exact match or justify further declaration permutations.

Current result: **99.663864% strict objdiff**, 952 compiled/retail bytes under
`probe-gcc257-o2-g0`. Twenty aligned words remain different; this function is
not exact and is not banked. The current caller is 87.922420% and unchanged by
this pass. The lifetime corrections below supersede the original results.

## Function Match Plan: loop-local projected base (master `7e3bb27`)

OPEN `0x800185e8`, 952 retail bytes, starts this pass at strict 97.890755%
with a 956-byte candidate under `probe-gcc257-o2-g0`. The complete graphics
owner already recovers current-asset/projected-array address sharing. The six
semantic views, full retail CFG, sole caller `render_map_cell`, neighboring
unlit/sprite bodies, accessor/allocator boundaries, SDK declarations and source
history were inspected again. There are no string references or candidate
outgoing relocations. All 18 calls, the two validated data address pairs and
the validated internal jump have concrete referents. LIBGTE SMP and LIBGPU
PRIM provider evidence excludes the SDK callees, not this game-owned emitter.

The caller masks the selected object to eight bits; this function and the
accessor preserve their `u16` boundary. The packet count/header are words;
prepared indices are unsigned halfword byte offsets; depth/fog inputs are
signed halfwords. Both mode branches, clipping/depth guards, checked signed
division, postdecrement loop and 80-byte-frame return delay slot are modeled.
The first raw divergence is the normal-pointer stack slot (32 retail versus
24 candidate). The candidate then forms the projected base before loading
the count, whereas retail forms it only after the nonempty guard at
`0x80018664`. It also retains one extra ordering-table HI16/LO16 pair.

First hypothesis: make the projected-array pointer local to the packet loop,
its actual use scope, without changing packet expressions or other locals.
Compare the first raw divergence and ordered numeric referents after a real
focused compile, then obtain the strict report. Any unresolved frame/register
or address-sharing symptoms remain unattributed; do not fabricate padding or
permute declarations to imitate them.

The loop-local projected base recovers the nonempty-path initialization,
retail counter/base register roles and relative ordering-table load. The
candidate is now 952 bytes with the same 18 numeric call targets and two
numeric data targets; 23 aligned words differ. Remaining symptoms are the
normal/header spill slots, quad depth result register and triangle packet
register. Next inspect header lifetime: retail reads it afresh each iteration
and uses it only through that iteration's advance, so test a loop-local
initialized header instead of the function-wide temporary.

That header-scope trial emits the same instructions and referents and is
removed. The next independently bounded lifetime is depth: each mode defines
and consumes its own depth before leaving the case, with no live depth at the
shared packet-advance label. Test case-local depth variables, preserving the
separate quad and triangle arithmetic and their existing per-mode guards.

Case-local depth recovers all three quad-depth register differences, leaving
20 aligned words. Keep that narrower live range. Retail has one insertion
tail, entered after each mode's own depth guard and masked index, with both
packet forms using the same packet register. Test spelling that shared tail
explicitly: each case produces the accepted packet and ordering index, and
one `AddPrim` consumes them after the switch. Unlike the rejected original
post-switch-depth-check source, clipping and depth rejection stay in each
mode, and rejected/unsupported packets still jump directly to advancement.

The explicit shared-tail trial recovers the triangle packet register but
reintroduces the extra ordering-table address pair, changes the guard delay
slots and grows back to 956 bytes (49 aligned differences). It is removed.
The retained changes are only loop-local vertices and case-local depth:
952 bytes, 20 aligned differences, unchanged numeric calls/data referents.

The strict report confirms 97.890755% -> 99.663864%. The retained normal/header
spill-slot swap accounts for five differing words; the triangle packet's
`s0` versus retail `s1` accounts for the other fifteen. All 18 calls, two
HI16/LO16 address pairs and the internal jump resolve to the retail numeric
targets. The frame, per-mode depth arithmetic, branch destinations and return
delay slot agree. These last stack/register symptoms are unattributed, not
proof of a compiler mechanism or an exact reconstruction.

Verification for this map/TMD lifetime pass: affected objects were rebuilt
and strict reports refreshed. OPEN remains 95/108 exact (13 partial), with
no previously exact regression and overall fuzzy coverage 99.086%. No bank
operation is appropriate. All 401 existing tests, Ruff and `git diff --check`
pass; no test or tooling code is added. Full `kf build` still reports only the
known OPEN TMD default-addend mismatch, thirteen GAME data-addend mismatches
and four GAME historical-best deficits. All 13 vendor-source controls remain
exact. GAME source, shared headers and retail ownership/relocations are unchanged.

## Historical first reconstruction

The initial reconstruction of OPEN `render_enqueue_map` at `0x800185e8`
reached 97.449580000% strict objdiff under the
`probe-gcc257-o2-g0`, emitting 960 bytes against 952 retail bytes. It was not
banked. Its sole caller, `render_map_cell`, then stood at 84.094826000%.
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
