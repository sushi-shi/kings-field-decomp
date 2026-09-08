# GAME item preview compiler traces

## Function Match Plan

Continue from `bbddfd5` with hash-verified GAME.EXE and the unchanged
`probe-gcc257-o2-g0` profile. The inventory and shop preview helpers share
their model transform, name-row copy, quantity read and confirmed menu
callers. Their existing units retain WIP ownership boundaries.

| GAME function | Address / bytes | Starting strict score |
| --- | --- | --- |
| `menu_item_model_preview` | `800279c4` / 440 | 98.181816% |
| `menu_draw_item_detail` | `80027b7c` / 732 | 92.207650% |

The six-view dossiers, full disassembly/CFG, native/traced baseline objects
and raw comparisons are in `build/gcc257/menu-preview/`. Whole ELF parity
holds for both units. Source history and the previous preview source/layout
campaigns were reviewed, including the rejected complete-row pointer,
per-bank selected-price-row and named-price-snapshot trials.

The preview has seven proven direct callers passing byte IDs or the widget's
word input, no consumed return, ten calls, eight address pairs, two conditional
branches and no strings or indirect transfers. Its 152-byte frame contains
the 24-byte glyph string and three SDK matrices; s0/s1/s2 and ra are saved.
The only three unequal words are at `80027a88..80027a90`: the candidate
finishes the item*20 shift before materializing `item_name_rows`, whereas
retail loads that base before the last shift. Ordered targets already agree.
The shared eight-byte rotation at `80057b70` is explicitly zero-initialized.

The detail helper has six proven callers passing item, one-based shop column
and buy/sell mode as words; its result is unused. It has seventeen calls,
twenty address pairs, three conditional branches and no strings or indirect
transfers. The 160-byte frame saves s0 through s4 and ra. Thirty-two words
differ, beginning with shop/mode saved registers at `80027b9c`; name-copy
registers/order and price address/argument setup also differ. Keep sell as
the default bank and overwrite it only for zero buy mode, preserving the
unsigned-halfword price read and shop-minus-one column. The exact marker,
frame-quad and dialog-frame siblings remain controls.

Retail caller windows, the preceding name-list return and following quad,
model-renderer and formatter bodies constrain these interfaces and boundaries.
The shared MenuGlyphRow is ten s16 codes; both price banks have eighty rows
of two u16 columns. Preserve these owners, signed word indices, the 255
empty-item guard, translation 560/140/1500, sixteen-unit wrapped yaw step,
light matrix, glyphs, line origins and all call/delay-slot behavior.

These are game UI policies. RotMatrix and matrix setters retain their
LIBGTE evidence and authentic MATRIX/SVECTOR interfaces; MulMatrix0's SDK
lineage has a known unresolved earlier revision. No vendored body is being
reconstructed and no new compiler attribution is proposed.

First test direct `item_name_rows[item_id].codes[i]` inside the existing
ten-iteration copy, removing the separately named interior glyph pointer.
This uses the independently owned row representation and the source form
already matching in the calling list builders. Trace where loop optimization
places the row base and stride, then compare from the first raw divergence.
Run the two functions independently, preserve exact siblings and reject an
unchanged or wrong source form. Any price-lookup follow-up needs its own
evidence-directed axis after this result; do not permute declarations or
introduce register carriers.

Strict 100% is required for banking. Before retaining an exact result, audit
all raw words and ordered referents, native/traced parity, exact controls and
rotation data, rebuild the affected unit, run Ruff, repository tests,
whitespace checks and the full build. Generated trials and reports stay in
`build/`; this campaign must preserve the earlier exact menu matches.

## Follow-up hypotheses

Direct row indexing leaves both strict scores and their first raw divergences
unchanged; the preview still differs at `80027a88`, and the detail helper at
`80027b9c`. The three exact detail siblings retain all their retail words.
Do not retain these no-effect source changes.

The preview's name-address arithmetic precedes primitive-cursor acquisition
and glyph-origin stores in retail. Next test selecting the existing name
pointer immediately after model rendering, before those stores. This moves a
real address calculation to its observed operation group without changing
memory effects or introducing a new value. Keep the established indexed copy.

For detail prices, the baseline has two source references each for shop and
mode, crossing seven calls; their respective live lengths 84 and 80 allocate
them to s4 and s2. Retail instead retains shop in s2 and mode in s4, forms the
item row before the shop column and loads the price before formatter argument
setup. Test selecting the actual u16 price value in mutually exclusive buy
and sell arms, then calling the existing formatter once. Each arm uses the
same proven bank/row/column expression. This differs from the earlier rejected
pointer-row and named-snapshot trials: the two actual price expressions expose
the conditional value ownership. Compare branch polarity, ordered bank
targets, all index widths and loads, and the first raw divergence before
considering any composition with a name-address result.

The name-preparation move leaves the preview's three unequal words unchanged.
Conditional price values recover all twenty detail differences outside its
name-copy region: strict score is 98.579230%, with twelve unequal words and
the same 732-byte extent. Both bank targets, branch polarity, row/column
address sequence, formatter setup and saved shop/mode registers now agree.
Shop gains a third reference and is allocated s2; mode remains at two and is
allocated s4. Three exact siblings retain all words and ordered targets.

Both retail name loops explicitly advance the source pointer after each
halfword load. Next test `gs.codes[i] = *name++` with the existing loop bound,
destination index and initial row pointer. This represents the directly
observed traversal using the real pointer, rather than the baseline's
separate compiler-created induction value. Test it independently in each
original unit before composing with the verified price-value change. Retain
neither a changed read count nor a different pointer referent.

The source-cursor trial still leaves the name-stride scheduling difference.
It assigns the cursor to a2 in both functions: this is wrong for inventory
preview but correct for shop detail, whose counter and destination remain
exchanged. The preview scores 97.818184%, detail 92.316940%; neither is kept.
Retail increments its copy counter before the halfword store. As a final
copy-loop axis, advance both the destination index and source pointer in the
copy expression, `gs.codes[i++] = *name++`, with the same ten-copy bound.
This has no unsequenced index read on the right-hand side. It tests actual
traversal ownership, without new locals or changed field widths; compare
counter/destination registers and the complete copy before retaining it.

Advancing the index in the copy expression produces the same strict scores
as the source-cursor trial. No copy-loop variant is retained. The price is
stored as u16 in the conditional-value trial but consumed by the formatter
as s32; the full unsigned-halfword range fits that word type. Test the same
conditional value as s32 to distinguish the storage width of the bank from
the width of its numeric formatter input. Both variants must retain the u16
bank load, and this follow-up changes no parameter or shared object type.

The word-sized price gives the same twelve name-copy differences as the u16
trial. It is retained as the actual formatter input; this is still partial,
not an exact match. Both retail copies are bottom-tested loops: the first
halfword read is unconditional after the item guard, and the sole copy branch
follows the counter increment and signed comparison with ten. Test that
control flow directly as `i = 0; do { gs.codes[i] = name[i]; i++; } while
(i < MENU_GLYPHS_PER_ROW);`, keeping the established indexed traversal. This
adds no state and preserves all ten reads and writes. Test each helper
separately, composing only with the already audited detail price correction.

## Retained source and verdicts

Both bottom-tested variants are identical to their respective production
baselines in every linked word and ordered target; neither is retained.
The final source selects the actual price in its buy/sell arms as s32, then
formats it once. The banks remain u16 and the formatter receives the same
nonnegative value. The compiler merges the selection into retail's existing
sell-default branch, including the row calculation in its delay slot.

The trace identifies a concrete change in the probe: shop's allocation inputs
move from two references, seven crossed calls and live length 84 to three
references, seven calls and length 87; its saved register becomes s2. Mode
retains two references and seven calls, with length 80 becoming 79, and is
allocated s4. The price has three references, no crossed calls and live length
four, allocated a0. The u16 and s32 input variants emit the same instructions.
These observations explain this probe's twenty recovered words; they are not
historical compiler attribution or a completed match.

| GAME function | Final strict score | Verdict |
| --- | ---: | --- |
| `800279c4 menu_item_model_preview` | 98.181816% | Unchanged; three name-stride ordering words differ. |
| `80027b7c menu_draw_item_detail` | 98.579230% | Price selection improved from 92.207650%; twelve name-copy words differ. |
| `80027e58 menu_add_marker_quad` | 100% | All eighteen words and ordered referents preserved. |
| `80027ea0 menu_add_frame_quad` | 100% | All seventeen words and ordered referents preserved. |
| `80027ee4 menu_draw_dialog_frame` | 100% | All 295 words and ordered referents preserved. |

The first preview difference remains `80027a88`: candidate shifts before the
same two-word item-name base materialization. The detail helper now first
differs at `80027c50` with that same three-word order, followed by nine counter,
source-pointer and destination-pointer register differences. Its original
saved-argument, price-branch, index and formatter differences are gone. Calls,
CFG, ordered physical targets and all other raw instructions agree. Both
functions remain open; no copy-loop or compiler limitation is inferred.

Verification freshly compiles both complete units using native, host-debug
and two traced compilers. All four whole ELFs agree per unit, and repeated
JSONL traces are byte-identical. Raw comparison covers 623 instruction words,
56 calls and sixty address materializations; all three exact siblings retain
their 330 retail words. Relinking every target function reproduces its retail
extent. An intentionally wrong model-renderer referent creates exactly one
additional differing word in the partial detail body, so that negative control
does not merely rediscover the existing residue. The complete eight-byte
rotation initializer matches retail. Generated evidence, including rejected
trials, remains under `build/gcc257/menu-preview/`.

The focused canonical detail match performs an actual compile and reproduces
98.579230%; the full build retains GAME 324/362, OPEN 106/108 and PSX 1/1 exact,
with all thirteen vendored controls exact. No new function is closed or banked.
Ruff and whitespace checks pass; all 703 repository tests pass in 90.549 seconds
with nine skips. The full build exits nonzero for the existing data/ownership
and placement gaps: source data PSX 0/1, GAME 10/42, OPEN 3/19; target relink
1/1, 75/77 and 34/38 respectively, with no artifact failures. The three already
exact detail-unit siblings have their verified input hashes refreshed after
staging the retained source. Toolchain/profile, shared types and inventories
are unchanged. Forty GAME/OPEN functions still require strict 100% closure.
