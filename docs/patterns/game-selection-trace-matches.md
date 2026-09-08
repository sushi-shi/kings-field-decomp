# GAME selection panels with GCC 2.5.7 traces

## Function Match Plan

Continue from `1769fb3` after strict closure of both shop panels. Reinitialize
hash-identical retail, capture/read the six GAME semantic views for each target,
and inspect full retail CFGs, sole caller windows, adjacent returns, shared
menu/item/player types, source histories and SDK-provider evidence. Normal and
instrumented compilers emit identical whole baseline objects for both units.
The configured GCC 2.5.7 target flags and compiler attribution remain unchanged.

| GAME function | Snapshot | First hypothesis |
| --- | --- | --- |
| `800249a8 menu_drop_item` | 1212 bytes, 99.547850%; frame 1856 with s0..s5/ra, twenty calls, eleven internal jumps, three address pairs, no strings/candidates/indirect transfers. Sole no-argument caller `80022494` ignores the result. | Consume the signed confirmation return through the existing numeric selection channel before its pending/item-ID remapping. Independently replace the local whole-player pointer with direct references to the same seven named fields. |
| `80023e9c menu_spell_select` | 1136 bytes, 99.542250%; frame 520 with s0..s5/ra, twenty calls, eleven internal jumps, three address pairs, no strings/candidates/indirect transfers. Sole no-argument caller `800237b8` ignores the result. | Consume the signed confirmation return through the existing numeric selection channel before its pending/selected-row remapping. |

These are game inventory/UI policies. The external PadRead at GAME `8005012c`
retains LIBETC PAD.OBJ, v1.17 retail-marker and cross-overlay provider evidence;
neither panel is a vendor body. The shared widget returns signed-word result
-1 for cancellation, consumed at `80024bc4..bcc` / `80024068..70`. Each panel
then writes -99 or its accepted selection. Modern typed confirmation values
are explicitly encoded at this existing numeric boundary. Do not treat the
widget's return as an item ID, add a carrier local, or change any branch result.

Drop's byte inventory quantities exclude one equipped copy using the ordered
weapon/head/body/shield/arm/leg/accessory fields. Its list context and eighty
label, quantity and item-code rows keep their current complete extents. All
transfers and effective field addresses agree; 24 words exchange selection,
codes-base and input registers, and eight words use player root plus member
offsets instead of retail's weapon-field base. Direct named accesses are already
supported by the exact sell panel and preserve the complete KfPlayerState owner;
no scalar-member pointer arithmetic or new overlapping datum is proposed.

Spell retains learned byte == 1 for IDs 4..8, the authored none label and ID255,
twenty ten-halfword rows and twenty byte codes, and a signed selected row with
-99/-1 controls. Its 26 unequal words are solely register operands. Preserve
texture failure == 1, initial/loop-head presentation, confirmation reset after
the exit guard, active-ID writeback and both byte loads before player_select_magic.

Drop's three exact save/load siblings and spell's exact equipment sibling
remain controls, totaling 4092 retail bytes. Test the three hypotheses separately,
then compose only independently supported corrections. Preserve each early
model/texture failure's direct epilogue edge, normal release/input behavior,
constants, ordered calls/referents and every delay slot. Inspect the first raw
divergence and selected allocation states after each focused trial. Only strict
100% production matches, verified by raw retail bytes and focused/full checks,
may be banked; partial improvements leave that function open.


## Independent trial results and composition

Spell's selection-result trial is strict 100%; all 284 resolved retail words
agree. Its selection pseudo has seventeen weighted references instead of
thirteen, with length 219 and seventeen crossed calls unchanged. It receives
s2, while the loop's constant one and current input receive retail's s3/s4.
The intermediate copy from v0 does not survive; all calls and referents agree.

Drop's selection-result trial is 99.943890%, with exactly the eight original
player-base/displacement words left. First divergence is `800249fc`, which
still selects `player_state` instead of `player_state+64`. All selection,
array-base and input operands now agree; selection references rise from
thirteen to seventeen, keeping length 224 and eighteen crossed calls.

The independent direct-field trial is 99.603960%. It recovers exactly those
eight base/displacement words and all three ordered address targets, leaving
the 24 original register-operand differences. Its first divergence remains
input initialization at `800249cc`. Complete source words and references of
all exact siblings are unchanged in every trial. The two corrected word sets
are disjoint and preserve the same field identities and control flow.

Now compose only these two supported drop corrections: direct named equipment
reads and the actual selection-result channel. Retain the exact spell trial
normally. Require strict 100% for the composed drop body before accepting it;
verify full source/target retail words, unit controls and current input hashes.


## Retained source and raw verification

Both panels are canonical strict **100%** after the changed units actually
rebuild. The composed drop body combines only the two independent corrections.
It retains the selection's seventeen references and s1 assignment, the codes
base in s2 and input in s3. Direct equipment reads expose a first-field address
at CSE1 (`player_state+0x64`); that same value survives loop hoisting and CSE2.
The previous whole-object pointer instead retained `player_state` at each of
those stages. All seven effective byte-field identities are identical; only
the supported source reference form changes. No overlapping global or alternate
structure layout is introduced.

Spell retains its actual confirmation return in the numeric selection channel,
then maps cancellation to pending or acceptance to the row index. Drop maps
acceptance to an item ID instead. Both preserve their original signed control
sentinels and typed API boundaries. The full final instruction streams contain
no extra result copy. The recorded allocation changes describe this compiler
probe; neither the source spelling nor compiler identity is historically proved.

Normal and instrumented production compiles agree as whole ELF objects. An
independent source/target relocation audit reproduces all **1610 retail words**
in the six functions: 587 newly exact drop/spell words and 1023 unchanged exact
sibling words. All 185 direct calls and 21 ordered address pairs match, including
the two corrected panels' forty calls and six pairs. A deliberately wrong
confirmation-widget target breaks each panel's raw match. Frame layouts,
constants, branch destinations, load/transfer delay slots and complete return
sequences are checked by these unmasked full-word comparisons.


## Verification checkpoint

The affected `game.menu_select` and `game.menu_item_drop` units actually compile
in the focused GAME match. Ruff, whitespace checks and all 703 repository tests
pass (83.241 seconds, nine optional skips). Full `kf build` runs every image's
checks and retains the existing source-data placement/ownership, reference
coverage and target-relink failures. Artifact failures remain zero; target
relinks remain PSX 1/1, GAME 75/77 and OPEN 34/38. No banked function regresses.
Fresh reports contain no stale or missing input failures.

The strict counts are GAME **322/362**, OPEN **106/108**, and PSX **1/1**:
429/471 overall. The full goal still has forty GAME and two OPEN functions
open. Generated evidence, candidate traces, raw audits and verification logs
remain under `build/gcc257/selection-panels/`; none are committed.


`kf bank --unit game.menu_item_drop --unit game.menu_select` records only the
six verified strict-100% rows: two new closures and four unchanged siblings
under their current complete source/header/profile hashes. No OPEN trial or
other unfinished campaign is banked.
