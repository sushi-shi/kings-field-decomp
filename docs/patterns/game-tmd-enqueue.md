# GAME TMD mode entries and shared lighting tails

## Function Match Plan (2026-09-06)

GAME `8001c7f8` / `0xf38` (3896 bytes), `render_enqueue_tmd`, starts
at strict 52.039013% on `9d076d6` under `probe-gcc257-o2-g0`. Retail
hash validation, all six semantic views, the complete body/CFG and switch
bytes, all seven ABI-constraining call sites, adjacent projection/model
functions, shared TMD/SDK types, source history and OPEN's behavioral
homolog were inspected before editing. The candidate signature remains
`void(u16 object_index, s16 depth_bias)`: retail masks a0 and sign-extends
a1; callers pass zero or halfword object selections, and depth biases zero,
15, 100, 1000 or a signed weapon-position calculation. Counts and headers
are words; prepared offsets are unsigned halfwords and depth/fog fields
are signed halfwords. No caller consumes a result.

Retail has an 88-byte frame, 45 direct calls plus one indirect switch,
35 validated HI16/LO16 pairs, eleven candidate internal jumps and one
return with its stack-restoring delay slot. The 29-word switch is this
unit's existing `RODATA(8001222c, 74)` claim, with twelve mode targets
and one default. Its decoded value chain is header shift 24, mode minus
32, unsigned bound 28, four-byte indexing, table load/load delay, `jr v0`
and its delay slot. The navigator's unresolved-switch reachability flags
do not prove these case bodies unreachable.

Physical mode order is 24, 28, 30, 38, 34, 3c, 32, 2c, 20, 3a, 22, 2a.
Each has its own positive-clipping and allocation/overflow path. All
twelve overflow branches reach `8001d700`; default, clipping rejection
and depth rejection reach packet advance at `8001d6d4`. Source currently
combines 20/22 and 28/2a and places the remaining arms in numeric order.
Recover the separate mode policies; do not introduce forced calls.
The actual SDK packet sizes in physical order are 32, 24, 28, 36, 40,
52, 28, 40, 20, 36, 20, 24. Modes 22/2a/32/3a enable semitransparency;
32 uses NormalColorCol3 without fog and 3a uses four NormalColorDpq calls.
All Gouraud fog calls use vertex zero's p2, including the fourth vertex.

The SDK negative control is LIBGPU PRIM (packet constructors, AddPrim and
SetSemiTrans, exact Release 2.5 archive evidence) and LIBGTE SMP
(clip/lighting, supported earlier SDK lineage, exact revision unresolved).
No vendored body is reconstructed. Keep authentic SDK signatures, TMD
packet colors/texture selectors, neutral textured color, triangle signed
division by three then shift two, quad shift four, signed biased depth
greater than four, OT mask 3fff and payload stride mask 3fc.

First review the eleven literal J destinations and all 29 table words
against their bounded value chain, then admit only these forty relocation
rows. This repairs the target object without altering source bytes. Next
remove the 29 obsolete standalone pointer identities/data rows inside the
unit's existing RODATA claim; they are switch entries, not separate globals.
Then test the retail asset-offset expression, immediate four-byte header
advance and word postdecrement. Reconstruct the twelve switch arms in
decoded order. Retail forms projected storage only after the entry guard,
reads quad vertex three before publishing the allocation cursor, and
publishes textured command bytes after UV writes. Retail shares F4/2a
emission, FT4/F4 fog, all quad depth, and final biased-depth insertion.
Compare shared vertex/packet lifetimes where these joins require them.

The first raw discrepancy is the 96-byte source versus 88-byte retail
frame, followed by asset/projection setup and dispatch. Do not pad a frame
or force registers. The broader GAME graphics owner remains unresolved;
keep separate honest globals while recording the common-base evidence.
Rebuild after each focused source cause and compare referents, calls, CFG
and widths. Preserve model 93.126690%, map 98.770996% and every exact.
Run focused matches, lint/tests, diff checks and full build before commit;
bank only strict 100% and record remaining evidence-based questions.

## Focused reconstruction

The forty reviewed rows pass the unchanged shared validator; the eleven
internal jumps are now validated references, not range-only candidates.
The 29 obsolete pointer identities are removed from the already unit-owned
switch range. Their census rows are consolidated into one 116-byte table
range without a separate source identity. With source unchanged, strict
match is 52.059547%.
Offset-before-base setup reaches 52.074947%; immediate header advance
reaches 54.473305% and removes the extra source frame slot, recovering the
88-byte retail frame without padding or forced storage. Word postdecrement
reaches 55.728954%. Twelve separate, correctly ordered mode entries reach
77.516426%; all allocation failures remain function returns.

Guarded projected-base initialization reaches 77.366530%, retaining the
observed lifetime despite the small intermediate score drop. The remaining
entry difference is the separate asset/projected addressing, then the
count/header spill exchange. Source still has duplicated quad tails and
case-local vertex values; use one shared set of vertex pointers for these
real shared calculations. Preserve each case's independently decoded
packet offsets, clipping arguments, lighting API and output packet type.

Shared vertex values recover the common depth tails and reach 94.640656%.
Publishing the four textured command bytes after UV writes reaches
97.351130%; reading each fourth vertex before the allocation-cursor store
reaches 98.413760%. FT4 fog in decoded vertex 0/1/2/3 order then reaches
98.932236%, recovering its shared F4 fog/lighting tail. These changes keep
the authentic packet types and twelve independently clipped allocation
paths. No forced register, fabricated local or handwritten instruction is
introduced.

## Final verdict

The focused recorded match retains strict **98.932236%**, up from
52.039013%. Compiled text is 3904 bytes versus 3896 retail, with the same
88-byte frame. A read-only audit of both raw object listings finds 45
identical ordered call targets, 55 conditional branches, eleven direct
jumps, eighteen division-check traps, one indirect switch and one return.
Both have 92 blocks when all reviewed switch destinations are included as
leaders. Ordered direct-control source/destination block indices and all
29 mode-indexed switch destinations agree; the twelve-instruction
restore/return tail is byte-identical. This is a bounded control-flow audit,
not a general equivalence proof or a substitute for strict 100%.

All twelve allocation-overflow guards reach the full epilogue, not packet
advance. The focused regression control checks the literal guards in retail
and compiled code and rejects redirecting each retail guard to packet
advance. It also checks the forty reviewed relocation rows against literal
targets, the bounded switch value chain and absence of superseded standalone
switch identities. No relocation validation rule is relaxed.

The first raw difference is at +3c: retail begins forming the selected-asset
address after loading the primitive count; source emits a load-delay NOP
and later loads the selected field directly. Retail derives projected
storage from that field's address plus 488 and the ordering-table pointer
from projected storage minus 756. Source retains their honest separate
identities and has 37 HI16/LO16 pairs versus retail's 35. The broader GAME
graphics owner remains provisional; the existing complete-clear pilot must
be checked across remaining consumers before migrating its production
claims. Do not remove either real reference to accommodate a target object.

Normals/projected bases exchange s7/s6, and count/header spills exchange
40/32. Vertex additions also retain opposite operand order. These are
observed, unattributed symptoms, not proved compiler mechanisms. No compiler
profile change or operand permutation is kept. The first twelve case
entries are four bytes later and the packet/return tail eight bytes later.

Only this function changes among all 484 native score rows. Model
93.126690%, map 98.770996%, sprite 88.263510% and every banked function are
preserved. GAME remains 288/362 exact and OPEN 97/108, with no OPEN source
changes. GAME aggregate fuzzy rises from 94.81% to 95.97%; overall fuzzy
rises from 95.52% to 96.49%. This is a partial reconstruction, not a new exact
or bank entry.

Focused recompilation after the census correction and full `kf build -j4`
retain these scores. Ruff, all 639 repository tests (72.596s, no skips),
the complete three-image payload census and `git diff --check` pass.
The first test run caught missing census coverage after removing the 29
pointer rows; the final single table range restores byte accounting without
restoring any false identity or weakening validation. Full build exits 1 on
the existing data/ownership/placement gates: source data 5/60, config-backed
SDK data 4/4, target relink 110/116, six conflicting section bases and zero
artifact failures. No partial baseline is updated or banked.
