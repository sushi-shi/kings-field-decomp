# GAME status-page rating arithmetic

## Function Match Plan

GAME `800264d8`, `menu_draw_status_details`, owns `0xcb4` bytes (813
instructions). Baseline `cb1e59d` is 95.110700%; the unit uses
`probe-gcc257-o2-g0`, not a proved historical compiler attribution.

The hash-identical retail body and all six semantic views were read before
editing. Retail has 72 direct calls, 16 conditional branches, two internal
jumps, one return, and an 80-byte frame saving `s0..s8,ra`. The original
source produces 804 instructions, 13 conditional branches and the same
calls, frame, and 89 ordered HI16/LO16 referents. The caller is
`menu_status_panel` at `80024350`, with no arguments and no consumed return
value. The preceding header renderer ends at this entry; the exact name-list
renderer begins at `8002718c`.

This is game-specific player-field/atlas rendering, not an SDK body. Its
callees are the shared game string renderer, number renderer and signed
number formatter. There are no literal-string references; the labels are
halfword atlas codes. Shared `KfPlayerState` fields are unsigned halfwords
at the attack/defense boundary, and `MenuGlyphString` has unsigned halfword
coordinates and signed halfword codes. Neither shared type changes here.
The source history begins with `52e9c9b`; subsequent ownership and naming
work preserved the original aggregate arithmetic.

First correct the two omitted internal-jump relocations from raw words,
then separately recover signed attack division and signed defense division.
Follow the actual subtotal's signed word lifetime and its ordering before
the row advance. Preserve calls, input field identities, constants, the
status/class branches, and the return delay slot. No frame fillers,
constant-carrying locals or register constraints are permitted.

## Relocation evidence

Both sites are reachable instructions inside this function, not data:

| Site | Raw word | Decoded target | Owned delay slot |
| --- | --- | --- | --- |
| `800267e8` | `08009a00` | `80026800` (`+328`) | `00002021`: zero the magic-tier result |
| `80026820` | `08009a0b` | `8002682c` (`+354`) | `24840003`: add the middle physical-tier stride |

The first is reached by base magic below 40; the second by physical power
40..59. Both join the directly decoded class-selection flow. Their old
`instruction-word`/candidate rows were excluded by the safe delinker.
Reviewing these two rows enables the existing paired CFG comparison. The
source is unchanged by this inventory step; its strict score becomes
95.123000%. The regenerated target must relink to every original retail word.

## Arithmetic evidence

The attack subtotal reads `player_state+30,+32,+34`, sums them, multiplies
by three, then logically shifts right by one at `80026b70`. It adds twice
the sum of `+36,+38`. Retail then multiplies the signed word subtotal by
ten, branches on its sign (`80026b8c`), adds seven only for a negative
numerator, and performs arithmetic shift-right by three (`80026b98`). This
is signed division by eight with truncation toward zero, not the old
outer right shift. The halfword Y coordinate is loaded/advanced only after
that division and stored in the formatter call's delay slot.

The defense calculation first divides the unsigned-halfword poison
resistance by signed five using normal integer promotion (`80026bd0`).
It adds the three physical defense halfwords and the magic/fire defense
halfwords. The ten-times total is then divided by signed seven at
`80026c48`, including the assembler's divide-by-zero and signed-overflow
guards. The source's leading `u32` cast incorrectly selected `divu` and
omitted the overflow-guard branches. Remove that cast, keeping all original
halfword fields and addition order.

These nonnegative halfword ranges fit the signed intermediate: the maximum
attack numerator is 5,570,470, and the maximum defense numerator is
3,407,820. Numerical agreement over those ranges would not justify retaining
the wrong signedness or instruction sequence.

## Focused results

The retail body SHA-256 is
`f210314ce339a8c9c3fd1f3e81596d2e4e3b8fb0755a971ce57b9582d3e9c2aa`.

| Source control, with reviewed relocations | Bytes | Strict objdiff |
| --- | ---: | ---: |
| Original source | 3216 | 95.123000% |
| Change only the final attack shift to signed `/ 8` | 3228 | 96.291510% |
| Also remove the unsigned defense cast | 3252 | 97.091020% |
| Signed attack subtotal, calculated before advancing Y | 3252 | 98.161130% |

The intermediate direct-argument division trials swap the first two attack
loads and still emit a logical final shift. A real signed `attack_rating`
subtotal, scaled/divided before the row advance, recovers the arithmetic
shift and all 89 ordered numeric address pairs. It also restores retail's
31 blocks and 16 branches, with the same 72 calls and one return. Block
agreement is not semantic equivalence or an exactness claim.

A separate signed defense subtotal, also calculated before advancing Y,
produces identical text and ordered text relocations to the retained attack
snapshot. It is not kept. The independently recompiled canonical source,
the retained trial and the canonical build object also agree completely
in text and ordered text relocations. The shared relocation validator
accepts both reviewed jump rows; relinking the target reproduces all 813
retail words, including the final `addiu sp,sp,80` return delay slot.

## Final verdict and verification

This section records the rating pass. The later
[shared-index correction](game-stats-header-index.md#detailed-status-follow-up)
recovers the class-index register roles and improves the score to 98.296430%;
the signed rating arithmetic and its remaining operand differences are unchanged.

Keep the signed attack subtotal and both division corrections. The function
improves from 95.110700% to **98.161130%**, remains partial, and is not banked.
Retail/candidate size is 3252 bytes with the same 80-byte frame. All 72
ordered calls and 89 ordered numeric data references agree.

The first remaining raw divergence is `+3c`: retail loads Y=35 into `s2`
in the primitive-buffer pointer's load-delay slot; source emits a nop and
loads the coordinate later. Other differences include class-index register
roles, the placement/reuse of 16-pixel row increments and blank/glyph
constants, and registers holding the signed rating arithmetic. The first
paired control-operand difference is the attack sign guard using `a0`
in retail versus `v1` in the candidate. No missing source early return is
inferred from these differences, and no compiler mechanism is attributed.

All 484 reported function rows were compared against the pre-campaign
snapshot: only this function's percentage changes, with zero artifact
failures. Exact counts remain GAME 313/362, OPEN 98/108, PSX 1/1; all 13
vendored source controls remain exact. Fresh focused compilation/raw audit,
Ruff, `git diff --check`, and all 680 repository tests pass (90.290 seconds).
Full `kf build` retains the existing data/ownership failures: data PSX 0/1,
GAME 9/42, OPEN 2/19; target relink 1/1, 75/77, 34/38, respectively. There
are six conflicting section bases and zero artifact failures. No tooling,
shared-header, OPEN source, or banking change belongs to this campaign.
