# GAME radial damage falloff

## Function Match Plan

Target: `GAME.EXE` `0x8002d4a8`, `actor_pool_apply_radial_damage`,
0x1f8 bytes, owned by `game.actor`. Initial strict native objdiff score:
86.103170%. Fresh candidate: 508 bytes; unit baseline: 25/29 exact.

The six image-qualified semantic views, all three direct caller windows and
source, both callees, adjacent actor functions, shared layouts/constants,
and source history were inspected before editing. The original falloff
shape and `u32 damage_scale` date to `642f170`.

Evidence snapshot:

- Eleven arguments: a three-word origin pointer, unsigned 32-bit radius,
  `u16` falloff/base power, and seven incoming stack halfwords for five
  components, scale and hit flags. Frame size is 136 bytes. All three calls
  in `effect_update_dispatch` pass falloff 4096, scale 5000, and unsigned
  halfword/byte combat inputs. The non-unity path remains real retail code.
- The 128-actor scan excludes inactive, post-death and current actors;
  a signed 16-bit index advances with the 72-byte actor stride. Definitions
  are 152 bytes and supply the halfword collision height at +0x7c.
- Two proven calls: `actor_distance_to_point` and `actor_apply_damage`.
  Three validated HI16/LO16 pairs reference `actor_state+0x720`,
  `actor_state+0x2b3c` and `actor_state`. One validated internal jump targets
  +0x16c. There are no candidate references, strings or indirect transfers.
- Seven conditional branches, one internal jump, two calls, one return,
  and one divide-by-zero trap. Every transfer's delay slot belongs to it,
  including the final 136-byte stack restore.
- `divu` uses the full radius, then `andi 0xffff` narrows the ratio; logical
  shifts and another halfword narrowing produce the weight. A final
  `andi v0,v0,0xffff` at `0x8002d624` narrows both arms' damage scale before
  it is stored in the callee's eighth argument slot.
- No vendored inventory entry; actor-state traversal, game damage calls and
  effect dispatch callers identify game logic, not an SDK/runtime body.
- The exact `player_apply_radial_damage` at GAME `0x800166b4` independently
  uses a calculated-first arm and `u16` final attenuation. Its formula is
  analogous, but its player damage scale convention remains distinct.

First change: make non-unity falloff the first arm. Retail's branch at
`0x8002d5bc` jumps over the calculation when falloff equals 4096; the
calculated arm jumps to the common call-argument preparation at +0x16c.
Second independent change: model the final scale as `u16`, consistent with
the common narrowing and the exact damage callee's parameter.

Compare the first remaining divergence after each focused rebuild. Preserve
the other 28 functions, especially the newly banked damage callee. Do not
add artificial locals, stores or padding for residual prologue differences.

## Results

The first two changes restore the calculated-first branch and common halfword
narrowing. The fresh function still has 508 bytes: its entry setup materializes
4096 twice, and subsequent branch offsets move by one word. The loop body,
call arguments and final scale narrowing otherwise align with retail.

Next hypothesis: remove the explicit cached `remaining` initializer and use
`KF_FIXED12_ONE - falloff_value` at the multiplication, as the exact player
counterpart expresses this deficit. Retail's one constant and loop-invariant
deficit are compatible with that source organization; they do not establish
whether the original source manually cached the expression. This is a
focused source hypothesis, not a proved optimizer attribution.

That control was rejected: this probe keeps the subtraction in the conditional
loop body instead of matching retail's entry-time stack value. The frame
shrinks from 136 to 128 bytes and the full-width reload at `0x8002d5d8`
becomes a subtraction. Restore the explicit cached deficit; the retained
changes are only the calculated-first arm and the 16-bit final scale.

## Retained verdict

Strict native objdiff improves **86.103170% -> 90.087300%** with the same
`probe-gcc257-o2-g0` profile. This is partial progress, not an exact result;
no baseline row is banked.

Fresh compilation verified that the other 28 actor-core functions retain
their resolved instruction words and ordered call/data references. The unit
remains 25/29 exact, including `actor_apply_damage`. Comparing all 484 scored
rows found only this percentage change, with no artifact failures or score
regressions. Eligible exact counts remain GAME 304/362, OPEN 98/108 and
PSX 1/1 (403/471 overall; 13 vendored controls excluded).

The raw residual is localized, not explained away:

- Retail has 126 words, candidate 127. The first divergence is
  `0x8002d4d4`: retail masks the falloff argument while the candidate starts
  copying origin/radius/base-power arguments. Retail initializes one 4096
  constant in `s7`; the candidate first uses `v0`, then initializes `s7`
  separately before the loop. The deficit stack slot and 136-byte frame agree.
- Retail's loop through epilogue starts at +0x88; the candidate starts at
  +0x8c. Of those 92 words, 91 are identical after that alignment. The sole
  differing word is the internal `j`, whose destination moves by the same
  four bytes (+0x16c -> +0x170). This mapped comparison is diagnostic only;
  it is not an exactness or banking rule.
- Both ordered calls and all three ordered data addresses agree. The seven
  branch instructions, their predicates and relative targets, call arguments,
  transfer delay slots, trap and return are retained in the aligned body.

There is no new evidence for a signature, field-width or referent change in
the remaining entry setup. It remains an unattributed codegen residue, not
a proven compiler/register/scheduler limitation. No artificial carriers,
forced stores, constant locals, padding or assembly were introduced.

## Verification

The retained unit was freshly rebuilt with `kf try --unit game.actor` and
`kf match --unit game.actor`, then the full `kf build` ran. Full-build failures
remain the existing data-layout/placement and known-reference closure work:
GAME data 9/42, OPEN 2/19, PSX 0/1; target relink 75/77, 34/38, 1/1.
There were no artifact failures. All 665 repository tests passed in 82.887
seconds; `ruff check scripts tests` and `git diff --check` passed.

Concurrent menu-title/status-text work was left untouched and committed
separately as `c05548a`. This campaign changes only the radial source,
its evidence notes/index, and the generated score summary. No compiler,
tooling, shared type declaration, relocation inventory or banked row changes.
