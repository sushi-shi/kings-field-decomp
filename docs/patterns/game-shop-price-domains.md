# GAME shop price domains and special item identities

## Function Match Plan

Start at `6622531` with newly hash-verified retail. The six-view dossiers,
call sites, neighboring functions and source histories are captured under
`build/constant-names/shop-price-domains/`. All units use the pinned
`probe-gcc257-o2-g0` probe. The selected bodies implement shop, inventory,
player or dialogue policy and are absent from the vendored roster; SDK calls
remain external provider boundaries.

Type buy/sell price selection as `KfItemPriceMode` through the detail renderer,
confirmation widget and every caller. Preserve full-word arguments and the
retail zero/nonzero predicate. The widget previously annotated this argument
as u32 and the detail renderer as s32; one s32 stored enum unifies their
full-word ABI without changing any signed comparison, since the consumer only
tests zero. The sell caller passes its confirmation flag under an exact-one
guard; replace this with the semantically appropriate SELL value one. Name
shop identifiers as such, while retaining them as integer bank/column indices;
merchant identity has not been independently decoded. Do not invent a shop
name from the character ID.

Rename the misleading local `category` byte arrays to `available`: they hold
shop availability bytes or unequipped player quantities, not categories.

Name the shared shop-window resource and its buy/sell/return/gold rows. Cursor
positions remain integer indices passed to the generic renderer. The root's
pending/exit values are a separate control protocol from price selection and
remain explicitly documented. Preserve its unusual final-redraw guard and
all exact root instructions.

Decode item-name rows with the retail atlas before promoting special IDs.
Name Gold Cross (52) and Key of the Dead (53) across shop availability,
selection bounds, the dialogue exchange, item dispatch and conditional combat
bonus. The effect kind 52 and map link 52 are different domains and keep their
own identities. Preserve the fact that only Gold Cross shop stock decrements.
Account for every retained literal in the root/buy/sell functions, and refresh
existing ledgers affected by the new enum arguments and item names.

Require all 112 objects, 484 scores and existing exact functions to retain
code and ordered references, except any independently inspected change from
replacing the guarded sell argument. Check modern compilation, lint, repository
tests, whitespace and full build before committing. No size assertions or
per-field tests are introduced.

## Evidence snapshots

| GAME address / bytes | Function | Starting strict % | Constraint |
| --- | --- | --- | --- |
| `0x80020cfc / 1500` | `item_load_database` | 99.746666 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x800212d8 / 608` | `item_menu_root` | 100.0 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x80021538 / 1476` | `item_menu_buy` | 97.20054 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x80021afc / 1280` | `item_menu_sell` | 96.77187 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x80027b7c / 732` | `menu_draw_item_detail` | 91.28416 | Full-word price flag; zero buys and nonzero sells; item*4 plus (shop-1)*2 selects an unsigned halfword. |
| `0x80028380 / 852` | `menu_list_interact` | 87.24413 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x80036e38 / 200` | `menu_enter_mode` | 100.0 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x80022608 / 1908` | `menu_use_item_panel` | 92.8218 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x8002317c / 1328` | `menu_magic_panel` | 95.89759 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x800238d8 / 1476` | `menu_equip_select` | 97.89973 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x80023e9c / 1136` | `menu_spell_select` | 98.06338 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x800249a8 / 1212` | `menu_drop_item` | 98.85478 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x80029de0 / 1328` | `menu_draw_string` | 99.98795 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x8002ad6c / 140` | `menu_list_init` | 100.0 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x800151cc / 740` | `game_state_initialize` | 100.0 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x80015714 / 2068` | `player_recalculate_combat_stats` | 100.0 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x80018054 / 1116` | `player_use_item` | 100.0 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x80034a80 / 724` | `map_event_interact` | 100.0 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |
| `0x80028914 / 348` | `menu_draw_window` | 87.54023 | Preserve captured input edges, call/return delay slots, ordered referents and signed/unsigned storage; named constants do not alter values. |

## Resource and behavior evidence

`KF/COM/STAT.DAT` is 5708 bytes, SHA256
`3f51069ac6291bffdfeb981b14963a22564b40fa5d9034f226797f84247b97f4`.
The loader copies 0x390 bytes of menu assets and 0x948 bytes of windows before
80 ten-halfword item-name rows, so item i begins at file offset `0xcd8 + 20*i`.
Window 7 row n's glyphs start at `0x390 + 7*264 + 24 + n*24 + 4`.
The [confirmation font audit](game-menu-confirmation-protocol.md) supplies the
hash, palette and 14-by-12-cell decoding for COM/MIX.TIM. Static visual decoding
of these exact resources yields:

| Resource | Text | Meaning |
| --- | --- | --- |
| Item 52 | 金の十字架 | Gold Cross |
| Item 53 | 死者の鍵 | Key of the Dead |
| Shop row 0 | 買う | Buy |
| Shop row 1 | 売る | Sell |
| Shop row 2 | もどる | Return |
| Shop row 3 | 所持 … ゴールド | Held gold label/value layout |

The second shop starts with one Gold Cross. The buy completion path decrements
shop stock only for this item; all other nonzero shop entries function as
availability flags. Character-3 dialogue consumes one player Gold Cross and
sets Key of the Dead stock to one. The key is part of the door-key dispatch.
Combat recalculation has a Gold Cross accessory case adding three holy-attack
points, although the current equipment menu's accessory range excludes ID 52;
this names the existing conditional case without inventing an equip route.
The sell list also ends before ID 52, and the use-list's later item band starts
at ID 52. Names describe these boundaries without changing their membership.

## Literal coverage and verification

The [shop ledger](game-shop-literal-ledger.md) explains every remaining literal
in the root/buy/sell functions: 155 occurrences, down from 188. Shared preview
coverage is refreshed from 213 to 204, equipment from 149 to 147, and floor
scripts from 319 to 316. Each retained Boolean, index, field width, authored
viewport bound or sentinel has its specific reason; the price-mode flag and
shop-window/item identities no longer remain anonymous numeric uses.

Every section of all 112 compiled objects is identical to the `6622531`
baseline, including debug sections. All 484 strict scores and the complete
objdiff report are unchanged. In particular, replacing the guarded confirmation
flag with the named SELL value preserves the old code. Seven reviewed exact
functions reproduce 1399 complete retail words with the same ordered calls and
numeric data referents. No bank rows or compiler profiles change.

Modern checks retain the exact multiset of 320 diagnostics: 64 passing and
48 failing source/image variants. Ruff and whitespace checks pass. The full
656-test run takes 84.951 seconds and initially finds two failures: the known
untracked save/load-hub failure and a stock-seed regex that only accepted hex
indices. The existing stock test now resolves literal-valued item enums while
retaining its independent SEEDS tuple. All six stock tests then pass in 2.788
seconds; no new test is added and the unrelated hub files are preserved.

Full build retains 8/61 source-data matches, 4/4 SDK/config-data matches, and
target relink PSX 1/1, GAME 75/77, OPEN 34/38. There are still two GAME/four
OPEN section-base conflicts and no artifact failures. The previous preview
note's source-data denominator is corrected from 60 to 61: the new rotation
owner added a data-owning unit. No tooling implementation or flake changed.

## Per-function final verdicts

| GAME address | Function | Final strict % | Verdict |
| --- | --- | --- | --- |
| `0x80020cfc` | `item_load_database` | 99.746666 | Unchanged partial match |
| `0x800212d8` | `item_menu_root` | 100.0 | Exact; 152 raw retail words |
| `0x80021538` | `item_menu_buy` | 97.20054 | Unchanged partial match |
| `0x80021afc` | `item_menu_sell` | 96.77187 | Unchanged partial match |
| `0x80027b7c` | `menu_draw_item_detail` | 91.28416 | Unchanged partial match |
| `0x80028380` | `menu_list_interact` | 87.24413 | Unchanged partial match |
| `0x80036e38` | `menu_enter_mode` | 100.0 | Exact; 50 raw retail words |
| `0x80022608` | `menu_use_item_panel` | 92.8218 | Unchanged partial match |
| `0x8002317c` | `menu_magic_panel` | 95.89759 | Unchanged partial match |
| `0x800238d8` | `menu_equip_select` | 97.89973 | Unchanged partial match |
| `0x80023e9c` | `menu_spell_select` | 98.06338 | Unchanged partial match |
| `0x800249a8` | `menu_drop_item` | 98.85478 | Unchanged partial match |
| `0x80029de0` | `menu_draw_string` | 99.98795 | Unchanged partial match |
| `0x8002ad6c` | `menu_list_init` | 100.0 | Exact; 35 raw retail words |
| `0x800151cc` | `game_state_initialize` | 100.0 | Exact; 185 raw retail words |
| `0x80015714` | `player_recalculate_combat_stats` | 100.0 | Exact; 517 raw retail words |
| `0x80018054` | `player_use_item` | 100.0 | Exact; 279 raw retail words |
| `0x80034a80` | `map_event_interact` | 100.0 | Exact; 181 raw retail words |
| `0x80028914` | `menu_draw_window` | 87.54023 | Unchanged partial match |
