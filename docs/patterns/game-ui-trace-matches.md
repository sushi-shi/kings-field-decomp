# GAME UI matching with GCC 2.5.7 traces

## Function Match Plan

Continue from `1dcfcce` with hash-verified retail and the unchanged pinned
`probe-gcc257-o2-g0` profile. Read the six semantic views, complete retail
bodies/CFGs, callers, adjacent boundaries, shared types, source history and
provider evidence before editing. These are game UI functions; PadRead and
AddPrim retain their separately recorded LIBETC/PAD and LIBGPU/PRIM attribution.

| GAME function | Baseline and preserved evidence | First hypothesis |
| --- | --- | --- |
| `8002589c menu_config_panel` | 1284 bytes, 99.859810%; 168-byte frame, nineteen direct calls, nine address pairs, seven internal jumps, no strings/indirect transfers; sole no-argument caller in menu_root. Four signed-word state copies and two 24-byte by-value labels retain their ABI, positive input priority, cues 0/1/2, redraw/release sequence and publication. Only nine s5/s6 operands differ. | Retail's confirm-exit and cancel arms join at one phase=-1 assignment (`80025c84`). Express that shared assignment as a common source label reached from confirm after setting its flag; cancel keeps its own sound before falling through. |
| `80025da0 menu_config_panel_draw` | Exact 408-byte control, 32-byte frame, nine calls, eleven pairs and one internal jump. Both by-value label arguments and the third state pointer are confirmed by both caller sites. | No body or signature change; retain every linked retail word. |
| `8001fafc notify_effect_update` | 99.385475%; 712 compiled versus 716 retail bytes, 56-byte frame, five calls, 36 retail address pairs and five internal jumps. The sole render_frame caller supplies no arguments. Preserve phase 0/2/3, byte IDs/cursors, 15-update hold, narrowed angle increment 128/clamp 512, four digit reads and duplicate non-gold dequeue. | Trace the missing independent initial-tail address before selecting a source correction. The exact enqueue, digit helper and buffer flip remain controls. |

Native and instrumented baseline compiles have identical whole ELF bytes for
both units. The configuration trace identifies phase as pseudo 75 and the
loop's state-array address as pseudo 223. At global allocation they have
respectively 9/8 weighted references and live lengths 230/206; both cross
nineteen calls. The probe's `global.c:allocno_compare` ranks them 1173/1165
using `floor(log2(refs))*refs/live_length*10000`, with one-register size.
Phase therefore receives s5 before the array receives s6. Both phase=-1
assignments still exist at allocation; the final jump pass merges them.

The shared source join tests a directly decoded CFG fact and predicts a
different allocation priority by removing the duplicate assignment before
allocation. It adds no local, alias, padding, forced register or call. Compile
one axis in disposable source, require native/traced parity, compare the first
raw divergence and all numeric referents, then keep only humane source which
preserves the exact helper. A candidate must reach canonical strict objdiff
100%, pass focused and full verification, and be banked before it counts as a
closed function. Probes and explanations alone leave the campaign open.

## Notification tail snapshot hypothesis

The baseline's initial tail read is a word `zero_extend(mem QI)` directly
inside the array index (UID 354). CSE1 changes its address to phase minus two;
CSE2 leaves that nested memory address relative. The earlier idle-case read
first loads a byte local and retains the independently materialized address.
The rotation-exit arm likewise snapshots a tail once before its mutable dequeue
loop, but the current source embeds the load directly in the ID expression.

Test an explicit `u8 tail` snapshot in that arm, assigned immediately before
the initial ID lookup. This represents the decoded byte load and its single
index consumer, matching the idle arm's existing source shape; it adds no
memory read, pointer alias or invented storage extent. Keep the control binding,
all six sprite clears, and the loop's subsequent independent tail reloads in
their current order. Require the raw initial-tail pair and the full strict
result, with all exact siblings preserved.

## Configuration result

The single shared-exit candidate reaches strict **100%**, up from 99.859810%.
Phase has seven weighted references and live length 228; the array address has
eight references and length 205. Their allocation priorities become 614 and
1170 respectively, so the array receives s5 and phase receives s6. Both still
cross nineteen calls. One source assignment survives to allocation, instead
of two assignments merged after allocation. Native and instrumented whole
objects agree; no compiler flag or optimizer is changed.

The retained C uses the existing confirmation branch to set its flag and
jump to `close_panel`; cancellation plays its own cue and falls through to
that same phase assignment. No additional sound, frame, input or publication
occurs. Fresh `kf try` reports both listings identical, and production strict
scores are 100% for both the panel and its draw helper. The strengthened
existing ABI control independently compares all 423 retail words across the
two functions, their 28 ordered calls, twenty address pairs, all reviewed
internal jump targets and delay slots. Its wrong-callee control still fails.

GAME advances from 317/362 to 318/362 strict exact; OPEN remains 106/108.
The broader goal still has 44 GAME and two OPEN functions open.

## Notification verdict and verification

The explicit byte snapshot remains 99.385475%, 712 bytes. Its byte load and
zero-extension stay separate through CSE2, but the address still uses phase
minus two; combine then reunites them. Thus a nested zero-extension alone
does not explain why the retail tail load retains its independent address.
Complete unit text and all 88 ordered text relocations remain identical to
the baseline, as do the other relocation sections except the debug-line file
reference. The candidate is rejected and production notification C is unchanged.

The focused configuration ABI/jump controls pass, as do Ruff, all 703 repository
tests (99.181 seconds, nine optional skips) and whitespace checks. Full
`kf build` was run after the affected production unit actually rebuilt. It
retains the existing data ownership/placement and target-relink failures, with
no artifact failures or exact-function regression. GAME/OPEN target relinks
remain 75/77 and 34/38; no comparison gate is weakened. Generated traces and
verification logs stay under `build/gcc257/ui-baselines/`.

`kf bank --unit game.menu_config_panel` records only these two verified exact
rows: the newly closed panel and the unchanged exact helper under the new unit
source hash. No notification or graphics-owner experiment is banked.
