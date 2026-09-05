# GAME floor-trigger results and shared warp paths

## Function Match Plan

GAME `80036af0 player_warp_trigger_update`, 588 bytes, starts at strict
68.326530% on master `2cdd1b6`, in `game.player_warp` using the pinned
`probe-gcc257-o2-g0`. Hash-validated retail, all six semantic views, current
raw source/target, sole caller, both neighboring functions, globals, RODATA,
and source history were inspected before editing. This is custom floor/cell
policy calling only the two game warp helpers, absent from SDK/FID inventories.
There are no strings and no reconstructed library bodies.

The 24-byte retail frame saves only ra at +16. An unsigned floor byte selects
one of five entries after floor-1/range4; the actual table at 12c14 contains
36b38,36b9c,36bc0,36bfc,36c4c. Preserve its existing 20-byte RODATA claim and
the candidate status of the five inventory pointer rows. The indirect jump is
not promoted merely by filtering the navigator's currently unreachable arms.
Each arm loads the word at player_state+c8 and masks its upper halfword.
Two reads of map_runtime_state+1cbf select the completion flag. No signature,
object extent, relocation target, switch table or evidence tier change is
needed to correct the observed source behavior.

The return contract has two completion paths, not just the one recorded in
the old identity note:

- Floor1, key0f020000, nonzero completion flag: the jump at36b94 owns the
  `li v0,1` delay slot at36b98 and reaches the common return.
- Floor5, key272f0000, nonzero flag: `bnez v0,36d2c` at36cd0 owns
  `li v0,1` at36cd4. The taken branch retains that value through return.
  A zero flag instead calls same-floor warp(2,5,25) and returns zero.
- All other paths return zero, including unsupported floors and successful
  warps. The caller at1491c passes no arguments, branches on v0 at14924, and
  selects game_exit_code=fe and the transition sequence on a nonzero result.

First correct only the missing floor-five result, retaining the existing warp
arguments and comparisons. This is a behavior correction independent of score.
Then compare the first real divergence before testing shared warp ownership:
retail's one change-floor call is at36b58, reached with floors1..5, variant0
except floor5/variant1. Five same-floor calls remain, including the shared
z=37 call at36d20. Current C emits two change-floor sites and places its common
call after floor5's first comparison. Do not force additional calls or change
argument values simply to alter register allocation.

The preceding same-floor helper ends at36ae8/ec; the exact actor transition
starts at36d3c. Preserve both it and the newly exact shimmer. Require strict
100%, raw words and ordered relocation addends for closure; no loose banking.
Verify each focused compile, full build, existing lint/tests and diff check.

## Focused reconstruction

The return-only correction reaches strict 71.489800%, from 68.326530%, with
the floor-five nonzero branch now retaining `v0=1` in its delay slot. All other
483 function-score rows are unchanged after integration on master `14eed27`.

Next hypothesis: model the shared change-floor operation with real destination
arguments (`s32` floor and `u8` variant), defaulting the variant to zero before
dispatch. Anchor its one call in the first floor-one warp branch, and have the
other floor-changing branches join it after selecting their arguments. Retail
prepares zero in a1 before the switch and changes it only for floor5/variant1;
its first call at36b58 is a backward join from later arms. Leave same-floor
operations and the corrected return paths unchanged in this experiment.

The shared-call form recovers the entire retail entry schedule, one common
change-floor call, both completion-result schedules and all same-floor call
arguments. It reaches strict 98.163270% but is eight bytes too long: the
floor-five transition to floor4 still has a separate destination assignment/jump,
and floor-one/floor-three checks
target that extra block instead of retail's floor-three join at+104. The next
CFG correction makes that floor4 destination block explicitly shared by those
three floor arms. No instruction-selection or data change is indicated.

That explicit floor4 join reaches strict 100%: all 147 raw instruction words
and all 36 ordered relocation rows agree. The aligned word at player_state+c8
contains the existing `motion_state.pitch_step` halfword followed by
`map_cell.z/x` at ca/cb; the mask discards pitch. Spell its address through the
known pitch-step member instead of subtracting two from the cell pointer.
This preserves the directly observed four-byte load and existing shared object
layout, without introducing an overlapping global or speculative union owner.

## Final verdict and verification

The named-member access preserves the exact result. Final strict objdiff is
100.000000000%, up from 68.326530000%; the compiled body shrinks from 644 to
the retail 588 bytes. All 147 raw words and 36 ordered relocation rows are
identical in the final source/target objects, including all immediate constants,
delay slots and internal jump addends. The whole 20-byte switch contribution
also agrees: module-relative entries are 520,584,5a8,5e4,634 (hex), corresponding
to the five retail addresses in the plan. No candidate evidence was promoted.

The only changed score among all 484 function rows is this dispatcher.
GAME advances from 284/362 to 285/362 strict exact, leaving 77 partial and no
unstarted functions. OPEN remains 97/108 and PSX 1/1; overall is 383/471 exact.
The warp unit now has three strict-exact functions. Both adjacent warp helpers
retain their previous non-exact entry schedules; the shimmer and actor
transition stay exact. No library body is reconstructed or newly counted.

The final source was really rebuilt by `kf try --unit game.player_warp` and
the full `kf build -j4`. Ruff and all 612 existing local tests pass, with no
skips. The full build still exits nonzero on existing data ownership and section
placement failures: source data 5/60, SDK data 4/4, target relinking 110/116,
six conflicting section bases, zero artifact failures and no banked-function
regressions. No tooling/flake changes or new test campaign were needed.
Selectively banked only GAME `80036af0`; the entire GAME 100% goal remains open.
