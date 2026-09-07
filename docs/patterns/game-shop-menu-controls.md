# GAME shop action and panel controls

## Function Match Plan

Name and type the shop root's pending buy/sell action separately from its
open/closed phase. Reuse `KfMenuPanelPhase`; add `KfShopMenuAction` with none,
buy and sell. The numeric cursor still traverses three displayed rows and
becomes an action only after the return-row guard. Derive the existing buy
and sell row constants from those action values. Keep the return and gold
rows outside the pending-action domain.

Preserve the root's encoded phase/action equality for its final redraw,
matching the established equipment-panel source model. Do not simplify it
into a different predicate. Preserve action reset before phase testing, all
button edges and release waits, and the three initial draws. Buy/sell results
remain a mixed item-ID/control channel: name their pending/no-selection
values with the existing list constants, without retyping item IDs as phases
or confirmation outcomes. Name the shared nine-row shop viewport.

Initialize hash-identical retail and inspect six GAME semantic views for
the three shop functions, their mode-dispatch caller and list/window/detail
consumers. Review raw immediates, branches, stores, ordered calls and
relocations, neighboring claims, source history and the
[shop flow evidence](game-shop-panel-flow.md). The
[window resource audit](game-menu-window-kinds.md) independently decodes the
buy/sell/return rows and separate gold label. These are game UI/inventory
policies, not vendor bodies; SDK calls stay external.

The unchanged shop ABI is `void(s32 shop_id)`. Root calls at GAME
`800213f4/80021404` forward the retained word. The root compares pending action
and phase as words and sets closed minus one on cancel/return. Buy's
`800216f8/800216fc` writes nine to the byte-sized visible-row field; sell
stores the same viewport size. Both selected-item channels carry minus 99
until confirmation, minus one on cancellation, and byte-loaded item IDs on
acceptance. Preserve those widths and the existing partial source shapes.

Before edits, capture current objects and all 484 scores. Pin the committed
unit profiles for isolated before/after compilation of all 112 variants;
concurrent profile experiments are separate inputs and must not be attributed
to these names. Force affected live builds, compare all unchanged-profile
objects/scores and every campaign function's raw instructions/references,
and inspect the first divergence of each partial control. Require modern enum
rejection controls, unchanged diagnostic baseline, inventory, Ruff, repository
tests, whitespace and a full `kf build`. No size assertions or new permanent
tests. Refresh complete item literal accounting after the substitutions.

## Baseline snapshots

| GAME VA / bytes | Function | Strict baseline |
| --- | --- | ---: |
| `0x800212d8 / 608` | `item_menu_root` | 100.000000% |
| `0x80021538 / 1476` | `item_menu_buy` | 99.769646% |
| `0x80021afc / 1280` | `item_menu_sell` | 99.609375% |
| `0x80027b7c / 732` | `menu_draw_item_detail` | 92.207650% |
| `0x80028380 / 852` | `menu_list_interact` | 100.000000% |
| `0x80028914 / 348` | `menu_draw_window` | 87.540230% |
| `0x80028a70 / 1916` | `menu_list_render` | 100.000000% |
| `0x8002ad6c / 140` | `menu_list_init` | 100.000000% |
| `0x80036e38 / 200` | `menu_enter_mode` | 100.000000% |

## Final evidence and verification

The shop root uses `KfShopMenuAction` for its pending dispatch and
`KfMenuPanelPhase` for loop lifetime. Cursor traversal remains numeric; only
its guarded buy/sell rows are decoded as actions. The return row closes the
panel without becoming an action. The shared buy/sell row definitions derive
from the action values, keeping table positions consistent with dispatch.

The final redraw intentionally compares encoded phase/action words. Both
closed phase and no pending action encode minus one, as in the equipment
root. Their direct enum comparison is rejected by modern compilation; the
explicit encoding records this particular retail relationship. Resetting the
action still occupies the branch delay slot at GAME `80021414`.

Both outer item-selection channels retain numeric item IDs plus the existing
`KF_MENU_LIST_PENDING` and `KF_MENU_LIST_NO_SELECTION` controls. These are not
shop actions or confirmation results. The nine-row viewport has one private
name shared by buy and sell. All 21 former inline selector/viewport occurrences
are removed from the complete item ledger; arithmetic endpoints, button masks,
quantities, glyph data and file-format values retain their per-occurrence reasons.

Ten Clang negative controls reject raw action/phase initialization, assignment,
comparison, implicit integer encoding/decoding and cross-domain action/phase
or confirmation-choice use. The positive control accepts each domain and the
explicit cursor/redraw boundaries. These temporary compiler checks remain in
`build/`; no permanent tests or size assertions were added. Whole-tree modern
checking retains the same 300 errors, with 65/112 variants passing.

Every one of the 9 functions in the baseline table retains its result:
5 exact and 4 partial. All 1,889 candidate words, 132 calls and
115 address references are unchanged. The exact controls also match
all 929 retail instruction words and their delinked targets.

The partial controls retain the following first divergences. These are
existing raw differences, without an attributed optimizer cause.

| Function | GAME site | Candidate / retail |
| --- | --- | --- |
| `item_menu_buy` | `0x80021568` | `addiu s2, zero, -0x63` / `addiu s1, zero, -0x63` |
| `item_menu_sell` | `0x80021b28` | `move s2, zero` / `move s3, zero` |
| `menu_draw_item_detail` | `0x80027b9c` | `move s4, a1` / `move s2, a1` |
| `menu_draw_window` | `0x80028918` | `sw ra, 0x2c(sp)` / `sw ra, 0x28(sp)` |

Independent before/after compilation of all 112 variants used the committed
`79df865` unit profiles. Allocated bytes, alignment, runtime symbols and
ordered relocations are identical; only `game.item` debug line information
changed. The concurrent profile experiments were reverted before final live
verification: all current objects agree with the isolated controls, and all
484 strict scores remain unchanged. No new exact function is claimed or banked.

Forced affected builds, inventory, Ruff, whitespace and all 683 repository
tests pass (90.175 seconds). Full `kf build` retains the existing source-data
mismatches (PSX 0/1, GAME 9/42, OPEN 2/19 units match) and target-relink conflicts
(PSX 1/1, GAME 75/77, OPEN 34/38 units verified), with no artifact failures.

The census now contains 6,640 retained occurrences. Complete ledgers cover
38 files and 3,778 occurrences, including the zero-literal warp wrapper.
The ten source lines containing fourteen `unknown_` tokens remain unresolved.
The broader naming/type-propagation goal remains incomplete.
