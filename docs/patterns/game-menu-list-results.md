# GAME list-selection controls and recovery amounts

## Function Match Plan

Start at `81ded14` on master, with hash-verified GAME retail and the existing
`probe-gcc257-o2-g0` profiles. Capture six semantic views, histories, adjacent
functions, shared confirmation/asset helpers, 112 object files and 484 strict
scores under `build/constant-names/menu-list-results/` before source edits.
The concurrent OPEN display-initialization work is outside this campaign.

| GAME address | Function | Retail bytes | Initial strict match | Evidence and first hypothesis |
| --- | --- | --- | --- | --- |
| `800222b4` | `menu_save_confirm` | 148 | 100% | Same-unit control; typed confirmation remains unchanged. |
| `80022348` | `menu_root` | 704 | 96.86364% | Sole direct item/magic caller; names the full-word magic no-selection comparison at `80022464`. Root result controls remain separate. |
| `80022608` | `menu_use_item_panel` | 1908 | 99.48218% | Signed selection initialized at `80022630`; byte item-code load after confirmation; four source model-failure exits and cancellation return -1. Name controls and five immediate HP additions. |
| `8002317c` | `menu_magic_panel` | 1328 | 100% | Signed selection initialized at `800231a4`; byte spell-code load at `800232fc`; failure, cancellation and insufficient-MP paths stay distinct. Name controls and Bless's three-times-MAGIC recovery. |
| `800236ac` | `menu_option_root` | 556 | 100% | Same-unit control; full-word equipment category selects the two panels, with MAGIC handled separately. |
| `800238d8` | `menu_equip_select` | 1476 | 100% | Signed selection initialized at `80023908`; byte item ID loaded at `80023b10`; -1 skips equipment publication, while item 255 is a valid unequip choice. |
| `80023e9c` | `menu_spell_select` | 1136 | 99.54225% | Signed selection initialized at `80023ec4`; `80024074` loads the selected row directly, then `800242c8..2e0` indexes the typed spell array for publication/call. |
| `800249a8` | `menu_drop_item` | 1212 | 99.54785% | Signed selection initialized at `800249d0`; byte item ID loaded at `80024bdc`; -1 skips stock decrement at `80024e28`. |

The five list panels share pending -99 and no-selection -1 controls. A
cancelled nested confirmation restores pending; cancelling the outer panel
or acknowledging its empty-list state yields no selection. Item/magic loader
failures also return no selection from the two value-returning panels.
Use plain named integer constants: these locals carry both negative controls
and nonnegative item IDs, spell IDs or row indices. Do not narrow them to a
byte or pretend a control-only enum represents every valid payload. Keep the
existing typed confirmation and selected-spell boundaries intact.

Preserve pending assignments in post-confirmation delay slots, each panel's
different frame/reset ordering, long-list branches, and shared normal-return
moves. In particular, magic's insufficient-MP path returns the selected ID
without casting; no-selection must not be named solely cancellation or error.
Do not merge root, confirmation and list protocols just because values agree.

The five consumable branches add 25, 10, 80, 150 and 300 to current HP at
`80022c14`, `80022c3c`, `80022c74`, `80022cac` and `80022cdc`. Retail writes
the unsigned halfword before testing against maximum HP; preserve this order.
Bless loads MAGIC from player +0x28, doubles then adds it at `8002363c`/`80023640`,
and adds that amount to current HP. Name these local recovery quantities,
document their units and truncation/clamp order, and make no balance-rationale
claim. Existing item/spell identities supply the branch meanings.

These functions implement game inventory, spell and equipment policy and
have no vendor-inventory match or SDK-wrapper shape. PadRead and rendering
primitives remain SDK boundaries. Root/equipment indirect switches retain
their current curated tables and evidence confidence; no candidate edge is
promoted by this naming pass.

Force the four affected units, compare every section and all strict scores,
then independently compare linked words, ordered calls and address pairs for
each snapshot. Preserve every existing exact function. Refresh affected
literal ledgers, including the earlier window/choice substitutions, and add
complete item-use/drop-panel coverage. Run modern checking, inventory, Ruff,
the existing tests, whitespace and full `kf build` before commit. No permanent
tests, size assertions, compiler-profile changes or banking are proposed.

## Result domains

`KF_MENU_LIST_PENDING=-99` means the list is still choosing; it initializes the
local and is restored when the nested confirmation is cancelled.
`KF_MENU_LIST_NO_SELECTION=-1` means no payload was selected. It covers outer
cancellation and empty-list acknowledgement, plus the direct model/texture
failure returns from item use and instant magic. The numeric choices of -99
and -1 have no recovered design rationale; their predicates and producers
establish their roles.

The successful payload is not uniform:

| Panel | Nonnegative selection | Final consumer |
| --- | --- | --- |
| Item use | Item ID from `codes[selected_index]` | Local consumable effects and the root's item/control return channel |
| Instant magic | Spell ID from `codes[selected_index]` | Spell record, MP cost and instant effect dispatch |
| Equipment | Item ID from `codes[selected_index]` | Equipment byte and typed player-slot API |
| Equipment magic | List row from `selected_index` | Index `codes[selection]` to obtain the typed selected-spell ID |
| Discard | Item ID from `codes[selected_index]` | Decrement the selected player stock byte |

Consequently `selection` remains `s32`, and the two constants are plain integer
enum members. A byte type would conflate no-selection -1 with valid unequip
255. A closed control enum would omit the successful ID/row payloads.
`KfMenuConfirmResult` still types the nested widget result and
`KfSelectedMagicId` still types equipment-magic storage and its publication.
The root compares the instant panel's return with the named list control,
then produces its separately named root result.

The confirmation does not supply a row: callers test its result, then load
their own selected code or row. Pending assignments remain in the decoded
post-call branch slots (`80022928`, `800232ec`, `80023b00`, `80024070`,
`80024bcc`). The normal magic cancellation shares the final result move,
while texture failure supplies a constant directly; insufficient MP returns
the selected spell without debiting MP or applying an effect. These return
paths are preserved exactly, including their delay slots.

## Recovery quantities

Item identities were established by the earlier
[consumable audit](game-item-consumables-accessories.md). The immediate values
below are additions to the unsigned current-HP halfword, not percentages of
maximum HP or durations. Their local names identify the responsible item.

| Item | Local constant | Added HP |
| --- | --- | --- |
| Medicinal Herb | `MEDICINAL_HERB_HP_RECOVERY` | 25 |
| Antidote Herb | `ANTIDOTE_HERB_HP_RECOVERY` | 10 |
| Recovery Medicine | `RECOVERY_MEDICINE_HP_RECOVERY` | 80 |
| Dragon King Grass Leaf | `DRAGON_KING_GRASS_LEAF_HP_RECOVERY` | 150 |
| Dragon King Grass Fruit | `DRAGON_KING_GRASS_FRUIT_HP_RECOVERY` | 300 |

The existing masks and MP effects remain unchanged: fruit also assigns maximum
MP, while leaf and fruit clear the full status bitset. Each addition is stored
as a halfword before the subsequent unsigned maximum-HP comparison; these
constants do not turn that sequence into a wide saturating addition.

`BLESS_HP_RECOVERY_MAGIC_MULTIPLIER=3` multiplies the current MAGIC stat to
produce HP recovery. Retail implements `2*magic + magic` and adds it to HP
before the same halfword-store/clamp pattern. Ordinary Healing adds MAGIC
directly. The quantities are gameplay tuning values, and neither the source
nor the decoded retail instructions prove why those amounts were chosen.

## Literal coverage and final verdicts

The batch names 43 numeric uses: 37 list controls, including the root caller,
and six recovery quantities. The five panels go from 314 to 272 retained
occurrences; the root caller removes one more. Named definitions are excluded
from these use-site counts.

The new [item-list ledger](game-menu-item-list-literal-ledger.md) covers all
115 retained occurrences in item use and discard, down from 135. Equipment
selection's existing [ledger](game-equipment-literal-ledger.md) falls from
138 to 126. The [spell ledger](game-spell-literal-ledger.md) now accounts for
97 uses: this batch removes ten, while eighteen already-named window/choice
uses are removed from stale ledger rows. The [root-choice ledger](game-menu-choice-literal-ledger.md)
falls from 97 to 96 and updates the unchanged equipment-root line locations.
Every retained expression keeps a specific reason. Shared eighty-entry
stock/table bounds and ten-glyph name widths still need propagation through
their wider save, shop and asset owners; these ledgers do not claim the broad
constant-naming goal is finished.

All four affected units were forcibly compiled. All 112 objects preserve
every executable/data/relocation section; only `.debug_line` changes in
`game.menu` and `game.menu_panels`. All 484 strict scores are unchanged.
The eight captured functions preserve all 2116 resolved source instruction
words, ordered calls and address materializations. The four exact controls
also independently equal the retail and delinked target, 877 words total.

| GAME address | Function | Final strict match | Verdict |
| --- | --- | --- | --- |
| `800222b4` | `menu_save_confirm` | 100% | Exact retained |
| `80022348` | `menu_root` | 96.86364% | Unchanged partial |
| `80022608` | `menu_use_item_panel` | 99.48218% | Unchanged partial |
| `8002317c` | `menu_magic_panel` | 100% | Exact retained |
| `800236ac` | `menu_option_root` | 100% | Exact retained |
| `800238d8` | `menu_equip_select` | 100% | Exact retained |
| `80023e9c` | `menu_spell_select` | 99.54225% | Unchanged partial |
| `800249a8` | `menu_drop_item` | 99.54785% | Unchanged partial |

Modern checking retains the same 320 errors and 64/112 passing units. Inventory,
`ruff check scripts tests`, all 678 repository tests (80.732 seconds) and
whitespace validation pass. Full `kf build` runs and fails its existing data,
ownership and relink gates: PSX data 0/1, GAME 9/42, OPEN 2/19; target relink
1/1, 75/77 and 34/38 respectively. Six conflicting section bases and zero
artifact failures remain. No new match is banked, and no size assertion or
permanent test is added.
