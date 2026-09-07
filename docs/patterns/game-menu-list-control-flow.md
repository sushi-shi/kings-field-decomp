# GAME menu-list quantities and row traversal

## Function Match Plan

Target: GAME `80028a70 menu_list_render`, 1916 bytes (`0x77c`), sole function
in `game.menu_list_render`. Starting strict objdiff is 88.252610%; the fresh
candidate has 1980 bytes and a 104-byte frame versus retail's 96. The pinned
`probe-gcc257-o2-g0` profile remains unchanged and historically unproved.

Read all six image-qualified semantic views, the complete retail block
listing, all fifteen caller argument/result windows, adjacent window/two-option
helpers, primitive and text/number callees, shared `KfMenuList`/sprite types,
source history, and the menu-layer/literal dossiers before source edits.
Thirteen panel calls pass a stack list at +24; two interaction calls pass the
retained list pointer. No caller consumes a result. Preserve the existing
`void (const KfMenuList *)` interface and shared 40-byte object.

Evidence snapshot:

- Retail: 479 words, 14 calls, 13 branches, one checked-division trap, no
  internal absolute jumps, one return with its frame-restore delay slot.
  The fresh candidate has 16 branches and separate signed/unsigned divides.
- Seventy validated HI16/LO16 pairs refer to `display_state+0x20`,
  `current_poly_ft4`, and fields of the existing `menu_assets` owner. There
  are no candidate references, strings, or unresolved indirect transfers.
- List geometry/count/width fields are bytes at +24..31; glyph/quantity
  pointers are words at +32/+36. Glyph copies use halfwords. The positioned
  scratch glyph run begins at sp+16. Quantity division is unsigned by 10,
  with both MFLO and MFHI consuming that one division.
- Glyph source advances by two inside each copy iteration (`80028b64`),
  before the width is reread; there is no post-copy width-based advance.
- Both row loops use signed word comparisons. The first loop tests visible
  rows and entry count at entry and after each row. Highlight selection uses
  the row index; row pitch is 12, text inset 3, quantity X inset 110 and Y
  inset 1. The bottom edge adds `(tile height + 3)` to the row Y base.
- Quantity tens is stored before its zero/blank test; ones is stored after
  that test, followed by the -1 terminator. No glyph, tile, draw order or
  ordering-table constant is to change.
- The target is absent from the vendored inventory and combines game list
  state, menu atlases and game primitive helpers. Called SDK routines remain
  SDK boundaries; no library body is reconstructed here.

First hypothesis: spell both decimal operations with an unsigned divisor,
as directly required by retail DIVU, then recompile and inspect the first
remaining divergence. Next restore the per-glyph pointer advance. After
each focused build, investigate the still-evidenced row-loop guards, digit
store order and coordinate grouping separately. Do not manufacture locals,
change shared field widths, alter profiles or use assembly to recover bytes.

Require strict 100% and complete raw word/ordered-reference agreement for
banking. Compare all 484 scored rows before/after, retain every previous
exact function, run the full build and required repository checks, and keep
unrelated concurrent edits out of the campaign commit.

## Workspace ownership hypothesis

The private `MenuGlyphBuffer.codes[22]` originated in the first reconstruction
(`ee84e34`), without a capacity proof. The 48-byte space between retail sp+16
and its first saved register is an upper bound, not a declared-object extent.
The earlier literal ledger's wording "observed extent" overstates that evidence.

All seven panel producers explicitly store 10 at list+31: `80021704`,
`80021c88`, `8002287c`, `80023254`, `80023a94`, `80023fbc`, `80024b24`.
Their row-copy stride is 20 bytes. `menu_list_init` defaults the width to 8
at `8002adf4`; the interaction wrapper retains the caller's list without
changing its width. Both draw APIs already accept the shared 24-byte
`MenuGlyphString` (origin plus ten codes), also used by adjacent menu widgets.

Test that existing object family, removing the private oversized type and
casts. Do not select another arbitrary capacity to tune the frame. This is a
source-ownership hypothesis supported by the complete known caller family;
the frame alone cannot prove the original declaration.

## Exact source and rejected intermediate forms

The shared glyph object restores retail's 96-byte frame without inventing a
new capacity. Both decimal operations use an unsigned divisor, capturing
quotient and remainder before changing the glyph workspace. This emits one
DIVU, one checked-division trap, MFLO/MFHI, and the observed tens/blank/ones
store sequence. Moving the remainder expression after the blank guard instead
of capturing its value first rereads the quantity and emits a second DIVU;
that experiment was rejected.

The glyph source advances within the copy loop, as at `80028b64`. The text
row loop uses signed comparisons at both entry and back edge. Its increment
belongs to the back-edge comparison (`++row`): the visible-row load then
precedes the increment at `80028c0c/80028c10`. Highlight rows similarly use
a signed entry guard, with the row increment before primitive initialization.
Initializing both row and Y offset in a `for` header hoisted the offset too
early and removed the highlight preheader; the guarded loop retains it.

Set the positioned glyph origin first, then add `row * 12` to its Y field.
The single expression `list_y + 3 + row * 12` instead hoists the inset into
the induction variable. Keeping a separate authored text-offset accumulator
with the two-stage origin restored arithmetic order but retained the wrong
saved-register assignment. Combining the evidenced origin update with the
row-derived displacement naturally emits the retail zero-based induction
variable and all its register operands. No declaration permutation, forced
register, fake local or compiler-option change was used.

For quantities, source X adjustment followed by Y adjustment emits retail's
Y-before-X load/store schedule. Both highlight and end-tile bottom edges add
`(tile->height + 3)` to the row base, not the flattened three-term sum. The
four corrected expressions reproduce every corresponding raw instruction.

These source facts compose to strict objdiff **100.000000000%**, from
88.252610%. This contradicts the initial reconstruction commit's attribution
of the list residue to a compiler/register-allocation wall. It does not prove
the historical compiler or the unique original C spelling.

## Verification

A second, fresh pinned compilation was independently linked through its real
relocations. First the delinked target was reconstructed to retail; then the
candidate matched all 479 raw words (1916 bytes), all 14 ordered direct calls,
and all 70 ordered HI16/LO16 target addresses. This includes every immediate,
branch displacement, the 96-byte frame, and all owned delay slots. The linked
function SHA-256 is
`d4b8a9f0c3875684e55a8d69850d09f97364dc9496c43da16fa42576a7a4a02f`.

No data, relocation or function-identity inventory changed. The full build
retains the existing data-placement/ownership failures: source data PSX 0/1,
GAME 9/42, OPEN 2/19; target relinks PSX 1/1, GAME 75/77, OPEN 34/38; zero
artifact failures. This function is exact, not a claim of whole-image closure.

All other 483 scored functions retain their scores, including every previous
exact result. Eligible counts advance GAME 304/362 to 305/362 and overall
403/471 to 404/471; OPEN remains 98/108 and PSX 1/1. Ruff, `git diff --check`
and all 678 repository tests pass (80.615 seconds). No tooling/flake changed.

An additional fresh compilation using the committed `0d0b156` menu header
produces identical complete text and relocation records, proving this exact
source does not depend on the concurrent menu-choice naming edits. After
staging only this campaign, `kf bank --unit game.menu_list_render` correctly
refused those other unstaged build inputs. The source is verified exact;
ledger banking is deferred until that independent campaign is staged or
committed. Do not bypass the dirty-input guard or stage another campaign to
make this bank succeed.
