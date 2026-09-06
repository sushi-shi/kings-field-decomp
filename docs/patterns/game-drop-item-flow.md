# GAME drop-item panel: initialization and traversal order

## Function Match Plan at `eaae676`

GAME `800249a8 menu_drop_item`, 1212 bytes, is the last partial member of
`game.menu_item_drop`; the hub and both slot panels are exact. Its starting
strict score is 98.854780%. A fresh pinned compile reproduces the same
1212-byte body and 1856-byte frame. The current first difference is the
ordering/register roles of the initial confirmation/input/selection values,
not a missing early return. All branch destinations currently agree.

Hash-identical retail, the six semantic queries, complete body, sole caller
at 22494, neighboring status/hub boundaries, original source/history, list
layout and widget/model interfaces were inspected. The caller passes no
arguments and ignores v0. This custom inventory/menu policy is not vendored:
it filters eighty owned item IDs against the player's seven equipment IDs,
builds ten-halfword labels, and decrements one accepted inventory byte.
PadRead remains the separately identified LIBETC boundary.

Retail has twenty direct calls, eleven validated internal jumps, three
validated address pairs and no strings, indirect transfers or candidates.
The three numeric addresses are player_state+64 (800a07e4), item_name_rows
(80058dc0) and item_stock (800652a8), in that order. Frame objects are the
40-byte KfMenuList at sp+24, 1600 label bytes at sp+64, eighty quantity bytes
at sp+1664 and eighty item-code bytes at sp+1744. Registers ra/s0..s5 are
saved. List indices/counts are bytes and the selected item is a full signed
integer so -99/-1 remain distinct from the item IDs.

List initialization takes (&ctx,0,4). The confirmation widget receives
(&ctx,1,0,item_id,0,0), with its fifth/sixth arguments on the O32 stack;
its result is accepted/cancelled, never an item index. Both model-load
failure branches return directly without the ordinary model-release tail.
The normal exit waits for pad release, releases the model, and decrements
inventory only for a non-cancelled selection. Keep this distinction.

The related equipment-panel source and retail both initialize confirmation before
current input, then initialize the selection sentinel. This source currently
initializes input first. Test that one observed initialization sequence,
without reordering arrays or uninitialized locals. The historical note
mentions declaration-order experiments but does not establish this current
named/shared-type input as exact or prove a compiler limitation. Do not
enumerate alternative declaration orders if this correction is flat.

Two traversal sequence facts are also directly visible: retail clears the
item counter immediately after the accepted-row count, before forming the
three object addresses; at the back edge it advances the label pointer
before incrementing/testing the item counter. The source instead initializes
the item counter in the for header after object setup and advances labels in
the comma expression after the counter. Test those two existing assignments
at their observed positions separately; preserve the stable inventory base,
all byte/halfword widths, nested-loop limits and exact neighbors. Do not add
a second inventory pointer or an unused value to steer the delay slot.

Compare the first divergence and all raw calls, targets, state initialization
and back-edge words after each focused compile. A matching CFG does not
authorize a 100% claim; any remaining register/order residue stays unbanked.

The initialization correction puts confirmation before current input, as
observed, but leaves the three register roles unchanged. The first remaining
difference is therefore still the input/selection register choice. Preserve
that recovered initialization order and test only the planned item-counter
setup before the data addresses next.

Moving the existing counter initialization recovers its retail placement
before the three address pairs without changing their referents or the CFG.
The main-loop register roles and the name/inventory back-edge order remain.
Test the final planned label advance at the end of the loop body, before
the for-loop's counter update, without adding a traversal pointer.

The label advance recovers the complete retail back-edge sequence, including
the inventory increment in its delay slot. The only focused differences
left are the three main-loop register roles: retail uses selection=s1,
codes-base=s2 and input=s3; the probe uses selection=s3, codes-base=s1 and
input=s2. Frame, object slots, constants, calls, branch destinations and
other instruction order now agree. Do not pursue further source permutations
without a new independently evidenced fact.

## Raw and strict result

Strict native objdiff improves from **98.854780% to 99.603960%**. The
independent fresh compilation still differs in exactly 24 of the 303 words:

```text
+024 +028 +1e0 +204 +224 +230 +234 +23c
+264 +274 +278 +28c +290 +2a0 +33c +34c
+3d0 +3f0 +400 +414 +430 +454 +480 +484
```

Each difference is a register operand belonging to the three roles above;
the first is input initialization at +024. No normalization or mask is used
to declare the body exact. All twenty numeric calls, eleven internal jumps,
three ordered data targets, and the complete sequence of decoded transfer
kinds/sites/destinations agree. The delinked target relinks to every retail
word. Confirmation initialization at +020, the counter clear at +04c and
the five back-edge words at +160..170 now agree literally.

The same fresh compilation reproduces every word of the 152-word hub,
282-word save panel and 220-word load panel. They remain strict 100%.
No new exact function is claimed or banked; the unit remains 3/4 exact.

All 484 scored bodies were compared with the preceding campaign snapshot;
only the drop-panel score changes. Eligible exact counts remain GAME 299/362,
OPEN 98/108 and PSX 1/1, or 398/471 overall. The focused real rebuild,
fresh raw-word audit, Ruff, all 657 repository tests and whitespace checks
pass. Full kf build still fails the existing data/relink closure gates:
source data 8/61, SDK config data 4/4 and target relink 110/116, with six
conflicting section bases and no data artifact failures. No baseline rows,
shared headers, compiler profiles or unrelated dirty inputs are banked.
