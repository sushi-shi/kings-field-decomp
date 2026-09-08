# GAME enqueuer stack-slot order

## Function Match Plan at `4a01223`

The completed graphics owner leaves two related GAME polygon emitters with
only swapped stack operands. Refresh retail hashes, the six image-qualified
views, complete callers/callees, shared packet and SDK types, adjacent TMD and
sprite functions, and the histories recorded in the model/map reconstruction
notes before editing. The existing rendering policies are game code; their
LIBGPU/LIBGTE calls retain the curated vendor attribution and authentic ABI.

| GAME function | Retail extent; strict baseline | Complete raw discrepancy |
| --- | --- | --- |
| `8001d730 render_enqueue_model` | 1768 bytes; 99.984160% | Seven operands exchange header/count slots: source 40/32, retail 32/40. Frame 96, fifteen calls, ten address pairs, five internal jumps and every other word agree. |
| `8001de18 render_enqueue_map` | 1048 bytes; 99.980920% | Five operands exchange header/normal-base slots: source 32/24, retail 24/32. Frame 80, sixteen calls, six address pairs, one internal jump and every other word agree. |

Model retains `void(u16, s16)`, map `void(u16)`. Counts and header words are
32-bit; prepared normal/vertex offsets are unsigned halfwords; projected depth
and fog are signed halfwords. Preserve the immediate four-byte header advance,
postdecrement count loop, packet-body length mask, every dispatch/clip/depth
predicate, all allocation-overflow returns, RGB/UV writes, ordered physical
referents, division traps and all transfer delay slots. Neither has strings
or unresolved indirect transfers. The exact adjacent sprite is a control.

The baseline trace maps model count/header to pseudos 78/79, and map
normal-base/header to 74/77. `reload1.c` first calls `alter_reg(i, -1)` in
ascending pseudo order. Its unallocated, referenced, non-equivalent values
receive slots through `assign_stack_local`; observed slots are eight bytes
apart in this probe. Validate this relationship with a small independent
source control before applying it to game code. Stack-allocation events alone
have no source-value owner, so corroborate them with source pseudos,
allocation results and the post-reload RTL/actual load-store value chains.

The bounded source hypothesis is that the existing packet header was declared
before the other spilled values. Test that one predicted order in temporary
copies, independently for model/map, and then compose only if the complete
retail bodies agree. Retain all real variables and their types; do not add
carriers, padding, volatile, assembly, forced calls or compiler options. The
previous packet-local header-scope experiment was identical and is not repeated.
This tests a concrete source declaration under the pinned GCC 2.5.7 probe;
it makes no claim about the historical compiler or a universal allocation rule.

Require strict objdiff 100%, complete raw-word equality and unchanged sibling
bodies before adopting a result. Verify native/debug/traced compiler parity and
trace repeatability, rebuild the affected unit and full project, run lint/tests,
inspect the diff and bank only exact rows. Preserve unresolved data placement
checks and the two pre-existing documentation changes outside this campaign.

## Result

Both predicted source changes are strict 100% on their first independent trial,
and they compose without changing either exact result. Model changes only the
order of the existing header/count declarations. Map declares the existing
header immediately after the object pointer. All expressions, object sizes,
control flow, calls, data ownership and compiler options remain unchanged.

The independent synthetic control reads twelve input values before an external
call and consumes each afterward. Three values lack hard registers. Reversing
the declarations of values 9 and 10 exchanges their pseudos 81/82 and their
stack homes 16/24; value 11 stays at 32. The same two input loads/stores and two
return loads change only their stack offsets. Trace allocation, decoded input
to stack value chains and complete native/traced objects corroborate the result.
The retained fixture and smoke runner test this alongside all existing controls.

| Function | Source value | Baseline pseudo / slot | Retained pseudo / retail slot |
| --- | --- | --- | --- |
| Model | header | 79 / 40 | 78 / 32 |
| Model | remaining | 78 / 32 | 79 / 40 |
| Map | header | 77 / 32 | 74 / 24 |
| Map | normals | 74 / 24 | 75 / 32 |

Weighted references, call crossings and live lengths are unchanged: model
header `[6,17,364]`, count `[10,17,386]`; map header `[6,15,206]`, normal base
`[5,16,224]`. Each still has no hard register after global allocation. The
verified pinned `reload1.c:644` ascending scan and `alter_reg` allocation explain
these particular slots. This is a demonstrated mechanism in the productive
probe, not historical compiler attribution or a general register-order rule.

Canonical source produces identical complete objects with native, debug,
instrumented and repeated instrumented GCC. Repeated function traces are
identical: model 33,622 events, map 16,988. The control corpus produces five
identical objects with tracing disabled/enabled and 48,848 deterministic events.
No instrumentation hook or production compiler/profile was changed.

Every word in the 1768-byte model and 1048-byte map bodies matches retail,
including all calls, addresses, branch/jump targets and delay slots. The exact
sprite sibling remains identical, and the TMD emitter keeps its prior partial
body. Production owner tests now require 154 exact bodies across the 173-function
campaign. Their separate-owner and earlier declaration-order controls still
reproduce the exact old seven/five-word mismatches.

GAME advances 327/362 -> 329/362; OPEN remains 106/108 and PSX 1/1. Thirty-three
GAME and two OPEN functions remain. The full build retains GAME 10/41, OPEN
3/19 and PSX 0/1 data comparisons, target relinking 75/77, 34/38 and 1/1,
zero artifact failures and no banked regression. Its data/placement failures
remain unresolved. Ruff and all 705 repository tests pass (9 skips); the
focused owner/overflow suite passes all 13 tests. The compiler smoke runner
validates all controlled allocation, reload, CSE, cross-jump, stack and
scheduling facts. `nix flake check -L` passes. Only the two new exact rows
and the unchanged exact sprite sibling are banked for this unit input.

A read-only trace refresh of the adjacent map-cell caller confirms its different
problem: the retail frame has an unaccessed 32-byte interval after MATRIX.
The SVECTOR, flag and saves are shifted by that interval. Declaration order
cannot explain the missing extent, and no padding or unused object is added.
That function remains open under its existing source/storage evidence.

Generated evidence and traces are under `build/gcc257/enqueue-spills/` and are
not committed. The two pre-existing documentation changes are outside this
campaign.
