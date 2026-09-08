# GAME status-rendering traces

## Function Match Plan

Continue from `ce37455` with hash-verified GAME.EXE and the unchanged
`probe-gcc257-o2-g0`. The confirmed menu-root/status-panel calls connect the
three selected functions. Their six semantic views, complete retail bodies,
caller and adjacent windows, shared types, source history and previous status
campaigns were inspected before edits. Generated evidence lives under
`build/gcc257/status-render/`; native and traced baseline ELFs agree.

| GAME function | Retail bytes / candidate bytes | Strict starting score |
| --- | ---: | ---: |
| `8002430c menu_status_panel` | 1692 / 1420 | 81.900710% |
| `80025f38 menu_draw_stats_header` | 1440 / 1428 | 97.991670% |
| `800264d8 menu_draw_status_details` | 3252 / 3252 | 98.296430% |

All have void interfaces with no parameters or consumed return. Root calls
the header three times and the panel once; the panel calls the detailed
renderer once per frame. Preserve the shared complete menu assets, positioned
glyph strings, player fields and authentic SDK packet interfaces. These are
game composition/formatting policies, not vendored bodies. SetSemiTrans has
exact Release 2.5 LIBGPU/PRIM evidence; PadRead has the documented 1994 SDK
lineage. The renderers call only the exact game string/number/formatter
helpers, whose bodies and interfaces were reviewed.

The panel has twenty calls, 105 address pairs, five conditional branches,
one internal jump and a frame-restoring return. Its 112-byte retail frame
versus 48-byte candidate is unchanged. The current compiler already narrows
UV arithmetic to byte loads, including the halfword-backed extents; there is
no missing byte-read correction. The supplied LIBGPU.H setXY4/setUV4 macros
are comma-separated assignments without narrowing temporaries or stack
objects. Retain the proven 2x2 reflected tile layout, frame-two release gate,
shared increment tail and sole dismissal return. No panel source edit is
supported by this inspection; do not add a duplicate descriptor view or an
unobserved 64-byte object.

The header has 28 calls, 36 address pairs, nine branches, two internal jumps
and one return. Its frame is 72 retail bytes versus 64 in the candidate.
The existing glyph-index correction is preserved. Retail retains row pitch
23 from the first class-guard delay slot onward; baseline expand through
final scheduling instead retain immediate additions of 23, with no dedicated
source pseudo for that constant. This does not justify a new constant carrier.
No header edit is proposed from this observation.

The detailed renderer has 72 calls, 89 address pairs, sixteen branches,
two internal jumps, no strings or unresolved transfers, and an 80-byte frame
saving ra and s0..s8. The earliest difference is the missing early Y=35
materialization at `80026514`. Later differences include the retained row
pitch 16, blank/terminator materialization and aggregate rating registers.
Preserve all signed and unsigned field widths, ordered references, thresholds
40/60, four-code class title, right-to-left status icons, formatting widths,
14/16-pixel row steps and return delay slot. The 24-byte glyph workspace is
the only source stack object; no artificial frame or register control is allowed.

First test one actual signed word rating value for the two successive attack
and defense computations. Retail forms both final aggregate numerators in a0
before scaling/division, then passes each rating to the same formatter as a0.
The existing attack value dies after its call; the defense computation starts
after number drawing and produces the next numeric row. Replace the dedicated
`attack_rating` name with `rating` and assign the complete defense expression
to that same variable before advancing Y. Keep the earlier separately named
defense-subtotal negative control distinct: this trial shares the actual
working value across both consumed rating phases. Do not change addition
order, unsigned first attack shift, signed divisions by eight/five/seven,
truncation guards or source reads. Compare the first changed raw instruction
and trace this value's allocation before deciding whether to retain it.

Only strict 100% closes a function. Every retained change requires fresh
whole-unit compilation, raw-word and ordered-target audit, focused matching,
Ruff, repository tests, whitespace check and full build. Preserve all exact
callers/callees. Bank only exact source from the staged campaign; compiler
traces, partial percentages and bounded negative controls are not closure.

The shared-rating trial changes twelve raw words and reaches 98.345634%,
with unchanged extent and ordered calls/addresses. Seven changes give the
attack subtotal, sign guard and final shift the retail a0 role. Five change
defense numerator registers but do not yet reproduce retail. The shared
pseudo has twelve references, zero crossed calls and live length fifteen,
allocated a0; the original attack-only value had ten references, zero calls
and length seven, allocated v1. No function is exact.

Retail also holds the completed defense subtotal in a0 before scaling it
into the separate division numerator. Next expose that same consumed step:
assign the defense sum to the shared `rating`, then `rating = rating * 10 / 7`
before the existing Y advance and formatter. This composes the audited
shared value with the independently decoded subtotal/scaling boundary, as
already expressed for attack. Keep poison's signed division by five in the
same sum position; add no operation, read or dummy state.

## Retained result and verification

The shared-subtotals trial reaches strict **98.431730%**. Exactly sixteen
candidate words change: seven attack operands and nine defense operands.
Every changed word equals its corresponding retail instruction, using the
already established four-byte sequence displacement caused by the earlier
extra materialization. All other candidate words are unchanged. Both final
aggregate subtotals now use a0, and the defense division numerator/divisor
and signed guard operands agree. The shared rating has fifteen references,
zero crossed calls and live length eighteen, allocated a0. Keep this humane
working value and the separately expressed scaling of each real subtotal.

The function is still non-exact. At +0x3c the candidate retains a nop where
retail loads Y=35 into s2; the candidate materializes that value later in t0.
Its Y=35 pseudo has three references, crosses eleven calls and has live
length 308, with no assigned global hard register. Row pitch 16 remains an
immediate rather than retail's retained s2 value, and blank/terminator/label
constant placements still differ. Those observations do not justify adding
constant carriers or claiming a compiler limitation.

| GAME function | Final strict result | Verdict |
| --- | ---: | --- |
| `menu_status_panel` | 81.900710% | Unchanged; byte loads already agree. No owner or frame hypothesis retained. |
| `menu_draw_stats_header` | 97.991670% | Unchanged; row-spacing register and frame remain unresolved. |
| `menu_draw_status_details` | 98.431730% | Sixteen rating words recovered from 98.296430%; partial, not banked. |

Fresh native, host-debug and two traced builds of the final source produce
identical whole ELFs; the repeated trace files are byte-identical. Full raw
comparison covers all 813 candidate words, seventy-two ordered direct calls
and eighty-nine address materializations. The delinked target reproduces all
813 retail words. A wrong formatter referent changes exactly the twenty-two
formatter call instructions. The focused canonical match performs a real
compile and reproduces the trial. The literal ledger retains the same tokens
and reasons, updated to the shared rating expressions.

Ruff and whitespace checks pass. All 703 repository tests pass in 108.648
seconds, with nine skips. The final full build preserves all banked functions:
GAME 324/362, OPEN 106/108, PSX 1/1, plus thirteen exact vendored controls.
It retains the existing data/ownership/placement failures: source data PSX
0/1, GAME 10/42 and OPEN 3/19; target relink 1/1, 75/77 and 34/38; no artifact
failures. No shared types, profiles, curated inventories or bank rows change.
Forty GAME/OPEN functions still require strict 100% closure.
