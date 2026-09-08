# GAME menu-preview source controls

## Function Match Plan at `57f6ce9`

GAME `800279c4 menu_item_model_preview`, 440 bytes, one-function unit
`game.menu_item_model_preview`, starts at strict 98.181816% with the existing
`probe-gcc257-o2-g0`. Hash validation, all six GAME semantic views, all seven
direct call windows, source history, adjacent boundaries, shared types and
the formatter/model-renderer/SDK contracts were inspected before editing.

The interface remains `void (s32 item_id)`. Callers supply byte item IDs or
the confirmation widget's word item input; none consumes a return. Preserve
the 255 empty-slot guard. The 152-byte frame saves ra/s2/s1/s0, with the
24-byte glyph workspace at sp+16 and SDK matrices at +40/+72/+104.
There are five blocks, two conditional branches, ten calls, eight validated
address pairs, one return with frame restoration in its delay slot, and no
strings, jump tables or unresolved indirect transfers. The complete shared
rotation initializer at `80057b70` is eight zero bytes.

This is game menu policy, absent from the vendored inventory. RotMatrix and
the three GTE matrix setters remain attributed SDK providers; MulMatrix0 has
cross-overlay/library lineage with an unresolved earlier revision. The
supplied headers define the actual MATRIX/SVECTOR interfaces. No SDK body,
profile or compiler-attribution claim changes.

A fresh full-unit trial identifies only a three-word ordering difference:
retail +0xc4/+0xc8 loads `item_name_rows`, then +0xcc performs the last left shift
of the item*20 offset. The candidate shifts first, then loads the same base.
The neighboring detail renderer repeats the retail ordering but has other
register/price-selection differences; it is not a substitute exact control.
The preceding exact equipment-name renderer copies whole rows with unaligned
word operations, unlike this function's explicit halfword loop.

First test a pointer to the existing complete `MenuGlyphRow` selected by
`&item_name_rows[item_id]`, consuming `name->codes[i]` inside the unchanged
ten-iteration copy. The loader independently proves eighty 20-byte rows;
this needs no new structure, cast, storage or owner. It distinguishes row
selection from the current interior-glyph pointer without changing input
width, copy count, side-effect order, labels or geometry. Inspect the first
raw difference after a fresh compile and retain only an evidenced correction.
No declaration permutation, padding, volatile carrier, forced register,
assembly or compiler flag is permitted. Exact closure still requires strict
100%, every raw word and ordered physical referent, and the full build.

The complete-row trial emits the original 110 linked words. Exactly +0xc4,
+0xc8 and +0xcc differ from retail; all calls, ordered numeric addresses and
eight initializer bytes agree. The retail SHA-256 is
`1ed2fed2e0f166cb5dcda02f43515b13e177ad5c9baac479b63e6d5077c6f56a`.
Reject this no-effect source control; do not reorder the target relocations
or change the proved row declaration to conceal instruction ordering.

## Detail-renderer Function Match Plan

GAME `80027b7c menu_draw_item_detail`, 732 bytes, starts at 91.284160% in
`game.menu_item_detail`. The six refreshed views, complete raw body, six
retail callers, adjacent preview/quad helpers, shared headers, history and
shop-bank evidence precede this trial. The three other unit functions are
strict-exact controls. Preserve the 160-byte frame, glyph workspace and three
SDK matrices, seven blocks, three conditional branches, seventeen calls,
twenty validated address pairs, and one frame-restoring return. There are no
strings or unresolved indirect transfers. This is the same custom menu/SDK
boundary as the preview, not a vendored function.

The full-word item/shop/price-mode interface is already shared. Buy callers
pass zero and sell callers one; the confirmation widget forwards the word
mode. All nonzero modes select the sell bank. Each 80-by-2 unsigned-halfword
bank is independently owned; item*4 + shop*2 - 2 selects the one-based shop
column. Keep the 255 empty-item guard, name-copy loop, all formatting widths,
gold/quantity reads, glyphs and complete ordered call set.

The first raw difference is the saved shop/mode register assignment at +0x20;
later the name-copy registers differ. The independently visible price CFG
also differs: retail materializes sell prices before a nonzero branch over
the buy-price assignment, with the item-row shift in the branch delay slot.
Current ternary C materializes buy prices first, then branches over sell
selection on zero. Test the retail's default-and-overwrite structure:
`prices = item_sell_prices; if (price_mode == KF_ITEM_PRICE_BUY)
prices = item_buy_prices;`. This retains every result and read, but makes
the source's selection/fallthrough ownership explicit. Inspect its earliest
raw consequence without forcing saved registers or delaying the row shift
through artificial dependencies. Preserve all three exact unit siblings.

The default-sell trial changes exactly three raw words: the two selected-bank
low immediates at +0x14c/+0x160 exchange places, and the intervening branch at
+0x154 changes from BEQZ to BNEZ. All other instructions are unchanged. This
recovers retail's ordered twenty data targets and the sell/buy fallthrough
policy, with all seventeen call targets unchanged. The three exact siblings
retain all 18/17/295 retail words and ordered numeric referents. Keep this
local CFG correction; register assignment and later argument setup remain
unexplained, not grounds for forced allocation.

Retail computes item*4 in the selection branch's slot, adds that to the
selected bank, and only then forms the shop-column offset. The current code
retains a whole-bank pointer and computes shop*2 before item*4 after selection.
Next test selecting the actual item's two-halfword row from each bank, then
indexing that row by shop-1. Preserve the default-sell/conditional-buy CFG,
both complete bank owners, the unsigned-halfword load and the signed word
index arithmetic. This is a proved row-subobject view, not a new storage
owner, raw byte-offset expression or artificial register carrier. Reject
the row view if it does not explain the observed address construction.

The row-selection trial grows to 736 bytes and duplicates the row-base add
across the two arms. It leaves the first saved-register difference and does
not recover the branch-slot shift, so reject it. The verified default-sell
form remains the production base.

The remaining price load is an actual value consumed by the formatter:
retail loads the unsigned halfword before setting width six and padding zero,
whereas the candidate sets those arguments first and loads the price last.
Test a word-sized `price` snapshot of the selected bank element immediately
before the formatter call. This preserves the existing load, full unsigned
halfword range, shop index, bank selection and all calls; it adds no new
memory access or constant carrier. Check whether this explicit input-value
boundary explains the observed load/argument sequence, without changing the
already correct three siblings or retaining a no-effect spelling.

The named-price trial is identical to the default-sell form in all allocated
section bytes/layouts and ordered symbolic relocations. It supplies no
additional correction and is not kept. The production object independently
equals the audited default-sell trial by those same checks, excluding only
source/debug metadata. Retail's complete detail-body SHA-256 is
`d7f8839ba6f7c83a5f56146877159ed95999d57786dd5b310b3ee47139d9b669`.

## Kept verdicts

- `800279c4 menu_item_model_preview`: unchanged at strict 98.181816%,
  440 bytes. The complete-row control is negative; its three-word row-stride
  ordering residue remains unattributed.
- `80027b7c menu_draw_item_detail`: **91.284160% -> 92.207650%**, 732 bytes.
  Keep only the sell-default/conditional-buy source structure. It recovers
  the ordered bank references and branch polarity; the saved shop/mode roles,
  name-copy registers/stride schedule, and price-load/setup sequence remain
  concrete non-exact differences.
- The unit's marker/frame quad and dialog-frame functions remain strict 100%
  and reproduce every retail word and ordered numeric referent. Their known
  callers, data ownership and code are unchanged.

Neither partial function is banked. The comparison of all 484 reported rows
finds only the detail renderer's score change; no banked match regresses.
GAME remains 313/362 exact, OPEN 98/108 and PSX 1/1, with all thirteen
vendored verification functions exact. No configuration, shared header,
baseline or unrelated source is changed by this campaign.

Ruff and `git diff --check` pass. All 680 repository tests pass in 85.673
seconds. The full `kf build` checks all images and retains only the existing
data/ownership/placement failures: data PSX 0/1, GAME 9/42 and OPEN 2/19;
target relink 1/1, 75/77 and 34/38, respectively; zero artifact failures.
The focused canonical match was an actual source compile, not a Ninja no-op.
No tooling or flake implementation changes were made.

## Instrumented follow-up

The [compiler-trace campaign](game-preview-traces.md) replaces bank-pointer
selection with conditional actual price values, recovering twenty more detail
words and reaching 98.579230%. The detail name-copy residue and the preview
three-word ordering residue remain open; this is not an exact closure.
