# GAME world-collision source recovery

## Function Match Plan at `a0ffa9b`

GAME `8001a5ac collision_query_world`, 1284 bytes (`0x504`), is the sole
function in `game.collision`. Strict baseline is 93.202490%; the current
compiled body is 1300 bytes. Hash-identical retail, all six semantic views,
complete retail disassembly/CFG and initial focused comparison were read,
along with all sixteen external call-site argument windows, the five callee
contracts, adjacent occupancy update/checked allocator, shared types, source
history and both prior collision-data/relocation dossiers.

The six O32 arguments remain signed word X/Y/Z/radius/height plus unsigned
word flags; callers supply height and flags at sp+16/+20. Height includes
zero, 1700 and unsigned definition halfwords; Y also uses the word sentinel
65535. Return is an encoded word or -1. The 64-byte frame saves s0..s7/ra,
the eight internal J instructions reach the common return at `8001aa84`,
and `jr` at `8001aaa8` owns the final stack restoration. All 21 HI/LO pairs
and eight internal jumps are already validated. There are no strings,
switch tables or unresolved indirect transfers. Five direct calls are the
floor-height, player-distance, actor-overlap, map-object-near-point and
map-event-overlap helpers. This is game-owned grid/class policy, absent
from the vendor census, not a Sony/GNU library-body reconstruction.

Retain terrain-first rejection, player/actor/object/event priority, all skip
bits and optional 0x800 output writes. Do not change the 255 signed height
initializers, complete VECTOR/SVECTOR copies, source claims, shared layouts
or compiler profile. The optional output's last six bytes remain untouched.
The first raw mismatch is the saved X register at +4, but more specific
source facts must be resolved before attributing that symptom:

- Retail reads actor definition ID at `8001a868`, and object ID at
  `8001a948`, before either transform copy. The current source reads each
  ID after both copies. Their unsigned byte values select the final radius.
- Retail reaches the actor radius with the already materialized pool base
  plus ID*152-1702, and the map-object radius with pool base plus ID*8-1278.
  The source's late ID reads instead emit extra definition HI/LO pairs.
  The existing aggregate owners already include each definition table and
  pool; the old source comment claiming otherwise is stale.
- The grid rejection masks shifted query flags before intersecting the
  cell byte. The candidate instead masks the cell byte first. Record this
  separate source-expression question without changing masks or widths to
  force a particular register.

First recover the two real unsigned ID snapshots before publishing position
and rotation. Compile and inspect the first divergence and both radius
referent chains. If the definition address still has a different lifetime,
test a typed selected-definition pointer within each guarded arm. No fake
carriers, volatile, forced registers, inline assembly, new struct variants
or compiler changes are admitted. Strict 100% plus all raw words/ordered
referents is the closure gate; lower intermediate scores need a separately
evidenced improvement to justify retention.

## Focused source trials

The two early ID snapshots give 94.806854% and restore both ID loads before
the transform stores. The byte local introduces an extra `andi 0xff` after
the actor copy, absent from retail; the loaded IDs are used as word-sized
array indices. Keep the actual fields unsigned bytes but promote their
snapshots to u32, matching retail's unmasked index arithmetic. The separate
definition address pairs still need investigation; the initial coordinate
register difference remains the first raw mismatch.

Promoted word indices give 95.118380% and remove the unsupported extra
narrowing. Both definition lookups still materialize a separate base, so
test the planned typed selected-definition pointers at the same early
binding point. Their radius is consumed only after both full transform
copies, as in retail. Do not replace the existing owners with offset views
or introduce a fabricated table bias to remove a real reference.

The typed definition pointers give 97.772580%. Both optional actor/object
transform-and-radius blocks now reproduce the retail instruction sequences,
including the pool-relative -1702/-1278 loads; the two extra definition
HI/LO pairs disappear. The body is now 1276 bytes, eight shorter than retail.
The first register mismatch and early flags load remain, along with the
terrain/rejection scratch values and operand order of the flag intersection.

Retail uses one word candidate-hit value (t1) for terrain kind, rejected
flag bits and pool-result indices. The source currently splits those into
three independently named values. Model that ordered query's candidate as
the existing signed word `hit` throughout; terrain byte and intersected
flags remain nonnegative, so their returns and shift retain exactly the same
values. Keep the remaining flag expression unchanged for this first trial.

The shared candidate-hit value gives 97.881620% and restores terrain kind,
rejected flags and pool indices to retail's t1. The eight-byte entry deficit
and cell/query mask order remain. Next express the selected query class as
`((flags & 0xf000) >> 8) & cell_flags`: this is the same documented query
field aligned with the cell byte, without introducing a synthetic carrier.
Compare the actual shift/mask/intersection sequence after a focused rebuild.

Masking before the shift gives 97.320870%: the cell byte now stays in v1,
but the emitted `andi 0xf000; srl 8` reverses retail's `srl 8; andi 0xf0`.
Keep the query field as the left operand while spelling that observed order:
`((flags >> 8) & 0xf0) & cell_flags`. The early flags-load/coordinate mismatch
is unchanged; the preceding trial is not kept as a preferred final form.

The shift-first left operand returns to 97.881620%, but still masks the cell
byte rather than the query word and rotates the scratch operands. Restore
the initial expression. Retail consumes the loaded cell flags as a word in
two independent intersections, without preserving a narrowed byte local;
test a promoted u32 snapshot while retaining the grid's actual u8 elements.
This isolates local value width from query-field association.

The promoted cell flags are byte-identical to the 97.881620% control; retain
the existing u8 snapshot rather than claim a recovered wider local. All
observable uses of the flat cell take its low 16 bits. A final bounded type
control will put that u16 conversion at the cell definition, retaining the
signed Z quotient and existing use-site casts. Reject it if the compiler
collapses the two separately observed truncations across the floor call.

The u16 cell control is also byte-identical; restore the existing word cell
with explicit use-site truncations. Neither local-width trial attributes
the entry difference. One remaining source-expression control follows the
retail operations as two statements: first compute the query mask into the
existing candidate `hit`, then intersect it with `cell_flags`. This tests
the observed intermediate mask before the intersection, without adding a
new local or changing the mask, result, branches or inputs.

The two-statement form gives 97.912770% and restores the observed shift,
query-mask and intersection order. It puts the mask into hit/t1 early,
whereas retail forms the mask in v0 and defines hit/t1 only at the final
intersection. Give that actual intermediate its own `u32 rejection_mask`
identity; it is consumed once by the intersection and has no artificial
later use. This tests distinct mask/result ownership, not a forced register.

## Kept source and raw verification

The distinct rejection mask gives **97.943924%**, up from 93.202490% at
`a0ffa9b`. It reproduces `lbu v1; srl v0; andi v0,0xf0; and t1,v1,v0`,
apart from the saved flags input being s5 rather than retail s0. Keep the
early typed definition pointers, shared candidate-hit value and two-stage
mask/intersection. The u8/u32 cell-flag and s32/u16 cell controls were flat
and are not retained as type discoveries. The source remains ordinary C;
no ABI, owner, claim, initialized value or compiler profile changed.

A temporary fresh-source compile uses the manifest's exact
`probe-gcc257-o2-g0` profile and produces the same sections as the production
object. Relocating every text reference to its actual reviewed GAME address
restores all 321 retail target words exactly. The reconstructed body remains
319 words / 1276 bytes versus retail's 321 words / 1284 bytes, with its first
different word at +4 (`sw s0,24(sp)` versus `sw s2,32(sp)`). Both objects have
55 physical text relocations: 21 ordered HI/LO pairs, five direct calls and
eight internal jumps. All ordered materialized addresses and call targets
agree; no missing target relocation is inferred. Both initialized height
payloads are exactly the same 510 retail bytes.

The per-function paired graph check compares block-numbered edge destinations
and ordered control mnemonics, not just branch/return counts. Both sides have
49 blocks and those sequences agree, including every shared-return edge;
there are no unresolved indirect transfers in this function. This does not
claim identical branch-condition dataflow or equal delay-slot instructions.
The mask stages and all four optional transform/radius blocks are also read
in the unmasked instruction comparison.

The remaining entry differs in flags loading/copying, saved coordinate/flag
registers and delay-slot scheduling: retail loads flags through v1 at +0xb4,
waits one instruction, then copies to s0; the candidate loads s5 earlier and
omits that separate copy and wait. Later control destinations move by eight
bytes. No independent source fact presently explains this entry, and it is
an **unattributed codegen residue**, not an exact result or a compiler wall.
No partial result is banked.

The four optional publication blocks are word-for-word equal after resolving
their real references: retail-relative ranges 0x1f4..0x260 (player),
0x2a4..0x33c (actor), 0x37c..0x40c (map object) and 0x44c..0x4c8 (event)
correspond to candidate ranges eight bytes earlier. These checks include
the complete vector copies and radius loads/stores, not merely their masks.

Ruff and `git diff --check` pass, and all **649 repository tests pass**
(94.081 seconds). The full `kf build` was run after the fresh focused build;
its existing data/ownership/placement failures remain: 6/60 source-data units
exact, 4/4 independent SDK/config contributions exact, 110/116 target units
relinked, six conflicting-section-base findings and zero artifact failures.
The world-query height data still has its pre-existing section-alignment
issue despite its exact initialized bytes.

All 484 reported function rows were compared with the `a0ffa9b` snapshot.
Only `collision_query_world` changes. Exact counts remain GAME 293/362,
OPEN 98/108 and PSX 1/1, or 392/471 eligible functions; all thirteen vendored
verification functions remain exact. The failed effect-helper return-form
trial was restored to the committed 97.452630% source. No baseline changed,
no banked function regressed, and no general CFG tooling was added.
