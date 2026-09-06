# GAME consumable and accessory identities

## Function Match Plan

Start at `07629a3`, on master, with hash-verified Japanese retail. The six-view
GAME dossiers and source histories are captured under
`build/constant-names/item-consumables-accessories/`. All selected units use
`probe-gcc257-o2-g0`. These inventory, menu, player and dialogue policies have
no entries in the vendored roster; SDK rendering, input and math calls remain
external boundaries. The shared item-stock, name-row and player owners remain
unchanged. Neighboring functions and caller/callee signatures constrain the
same byte item IDs, halfword stats and full-word menu return protocol.

Decode the retail item-name rows 42..51 before naming their actual item-ID
uses through shop order/stock, menu healing, item dispatch, revival, accessory
bonuses and map scripts. Preserve numeric equality and the existing range
membership. Tie accessory bounds to the decoded first/next item identities.
Use existing status-bit names to spell the exact retained-bit masks 0xb and
0x3; these intentionally clear higher bits including fire-defense boost.
Document the observed healing/stat amounts without inventing why the original
balance values were chosen. The partial item-name inventory is not yet a
complete field type, so this pass does not claim exhaustive item enum typing.

Require all 112 objects and 484 scores to preserve instructions and ordered
referents, and compare each reviewed exact function directly with complete
retail words including delay slots. Force affected compilation, full build,
modern compiler comparison, lint, repository tests and whitespace checks
before committing. No new size assertions or per-item tests are needed.

## Evidence snapshots

| GAME address / bytes | Function | Starting strict % | Evidence and constraint |
| --- | --- | --- | --- |
| `0x80020cfc / 1500` | `item_load_database` | 99.746666 | Control: reads COM/STAT.DAT and copies 80 ten-halfword item-name rows after the 0xcd8-byte prefix. |
| `0x800212d8 / 608` | `item_menu_root` | 100.0 | Control: full-word shop index dispatches buy/sell; input edges and delay slots remain unchanged. |
| `0x80021538 / 1476` | `item_menu_buy` | 97.20054 | Name the first item in the authored 42..79 then 0..41 ordering; keep byte stock and signed row indices. |
| `0x80021afc / 1280` | `item_menu_sell` | 96.77187 | Control: accessory IDs remain below the exclusive Gold Cross boundary. |
| `0x80022348 / 704` | `menu_root` | 96.86364 | Control: passes selected full-word item code or negative sentinel up to menu_enter_mode. |
| `0x80022608 / 1908` | `menu_use_item_panel` | 92.8218 | Preserve unsigned (selection-42)<6, byte stock decrement, u16 HP/MP stores, exact AND masks 0xb/0x3 and call slots. |
| `0x80018054 / 1116` | `player_use_item` | 100.0 | u8 dispatch from player_update; 43..47 acknowledge effects already applied by menu; 42 adds 100 to u16 training before its increment call. |
| `0x800151cc / 740` | `game_state_initialize` | 100.0 | Preserve player/shop byte stock seeds; item indices are not equipment record counts. |
| `0x800154b0 / 412` | `player_death_restart` | 100.0 | Unsigned byte stock[0][47] gates revival with floor-1 flag; consume one before persistence and full HP/MP recovery. |
| `0x80015714 / 2068` | `player_recalculate_combat_stats` | 100.0 | Accessory byte selects 48..51, 42 and 52 stat bonuses; preserve all halfword arithmetic and existing excluded-ID cases. |
| `0x80015fc0 / 152` | `player_increment_magic_training` | 100.0 | Control: u16 counter increments then compares against 100, adds one base magic, resets training and caps magic at 999. |
| `0x80016324 / 912` | `player_apply_damage` | 100.0 | Darkness bit 2 tests accessory byte against 0x31 (49), the moon amulet; all eight damage arguments and calls remain unchanged. |
| `0x80034610 / 144` | `map_action_script_floor3` | 100.0 | Possession byte stock[0][50] unlocks magic record 7; preserve independent dialogue-gated record 5 and notification calls. |
| `0x80034a80 / 724` | `map_event_interact` | 100.0 | Character 7 dialogue path consumes fruit ID47 and grants item62; preserve stage/page gates and numeric referents. |
| `0x800238d8 / 1476` | `menu_equip_select` | 97.89973 | Control: accessory selection covers [48,52); header bounds may refer to Light Ring and Gold Cross identities. |
| `0x80018880 / 6684` | `player_update` | 96.94554 | Final usage search found another bracelet consumer before commit: accessory byte 50 plus selected magic byte 7 chooses a one-bit right shift of the unsigned halfword MP cost. Preserve cast and charging flow. |

The additional `player_update` dossier and source history were captured before
its edit. Its proven caller is `game_main_loop`; the `80019324..80019370`
retail path compares bracelet 50 and magic 7, loads the halfword cost, and
halves it in the jump delay slot. Add this one ID substitution to the plan.

## Retail names and observed behavior

The [shop resource audit](game-shop-price-domains.md) establishes the
`COM/STAT.DAT` item-name row at `0xcd8 + 20*item_id`. Its 5708 bytes have SHA256
`3f51069ac6291bffdfeb981b14963a22564b40fa5d9034f226797f84247b97f4`.
The same audit's COM/MIX.TIM glyph decoder renders rows 42..53. The following
names are literal readings/translations of those Japanese resource labels,
not claims about English localization. Bonuses below are the explicit switch
additions; armor-record contributions are separate.

| ID | Japanese name | Source identity after `KF_ITEM_` | Observed use |
| --- | --- | --- | --- |
| 42 | ヴァーダイト | `VERDITE` | Adds 100 magic-training counts before the ordinary increment/check; accessory switch also has a +1 magic case. |
| 43 | 薬草 | `MEDICINAL_HERB` | Restores 25 HP. Player starts with one; both shops stock it. |
| 44 | 毒消し草 | `ANTIDOTE_HERB` | Restores 10 HP and retains only curse, darkness and slowed flags. Both shops stock it. |
| 45 | 回復薬 | `RECOVERY_MEDICINE` | Restores 80 HP and retains only curse and darkness flags. Both shops stock it. |
| 46 | 竜王草の葉 | `DRAGON_KING_GRASS_LEAF` | Restores 150 HP and clears all status flags. The second shop stocks it. |
| 47 | 竜王草の実 | `DRAGON_KING_GRASS_FRUIT` | Restores 300 HP, fills MP and clears all flags; also supplies revival and a dialogue exchange. Both shops stock it. |
| 48 | ライトリング | `LIGHT_RING` | Adds 5 holy attack; second shop stocks it. First selectable accessory. |
| 49 | 月の魔除け | `MOON_AMULET` | Adds 7 magic defense and prevents darkness application. |
| 50 | 風刃の腕輪 | `WIND_BLADE_BRACELET` | Adds 7 fire defense; possession on floor 3 teaches magic record 7, and equipping it halves that spell’s MP cost (rounded down). |
| 51 | 双頭竜の指輪 | `TWO_HEADED_DRAGON_RING` | Adds 8 magic. Last selectable accessory. |

The use panel consumes IDs 42..47 exactly once, using the retail unsigned
subtraction/range test. It stores HP additions in the existing u16 fields,
then clamps HP/MP to their maxima. `player_update` passes the selected item to
`player_use_item`; its restorative cases only acknowledge the effect already
applied by the panel. Verdite's additional effect happens there: add 100,
then the training helper adds one, tests against 100, raises base magic once,
resets the counter to zero and caps the result at 999. For the ordinary
0..99 counter range this guarantees a single increase; preserve the actual
halfword stores and overflow behavior rather than rewriting this as a direct
base-stat increment. Why training uses a hundred-count scale is unproven.

The exact cure masks matter. Retail GAME `80022c44` uses `andi ...,0xb`, and
`80022c7c` uses `andi ...,0x3`. The former retains bits 1|2|8, the latter 1|2.
Both clear poison (4), fire-defense boost (16), and all other higher bits.
Leaf and fruit clear the complete status field. Timers are not reset by these
menu branches. Replacing the masks with just the complement of poison would
change behavior; named retained bits preserve it.

Revival requires both the floor-1 revival flag and a possessed fruit. It
consumes one, persists world state and fills HP/MP before returning to the
floor-1 revival position. Without those conditions the existing new-session
initialization path runs. Separately, character 7's stage-2 dialogue consumes
one fruit for item 62; the item name of that reward remains a later audit.

Accessory selection remains `[LIGHT_RING, GOLD_CROSS)`, IDs 48..51. Combat
recalculation still includes conditional Verdite and Gold Cross cases outside
that menu range; this pass names the cases without inventing an equip route.
The moon amulet guard at `800163a0` loads `0x31` (decimal 49), followed by the
accessory-byte comparison. Wind Blade Bracelet's learned-record write depends
on possession, not equipped status. Spell record 7 is not named from the
bracelet's apparent theme.

## Remaining amounts and literal scope

| Retained values | Reason |
| --- | --- |
| HP additions 25, 10, 80, 150, 300 | Authored restoration amounts for the five named restorative items, in HP. Each occurs in its own item branch and is capped afterward; the original balance rationale is unknown. |
| Accessory additions 5, 7, 7, 8, 1, 3 | Direct holy-attack, magic-defense, fire-defense, magic, magic and holy-attack points for Light Ring, Moon Amulet, Wind Blade Bracelet, Two-headed Dragon Ring, Verdite and Gold Cross respectively. Separate balances, not one shared seven-point rule. |
| Verdite training addition 100 | A full threshold's worth of the helper's training counter; its following increment and reset are preserved. |
| Magic record 7 and cost shift 1 | Bracelet-specific spell selector and division by two with truncation; the resource spell name is not established by the bracelet name. |
| Stock bank 0 and shop banks 1/2; stock seed 1 | Player quantity and two shop availability banks; new-game seeds grant one copy or enable availability. |
| Status assignment 0 | Clears every flag, including beneficial flags; not an item ID or timer sentinel. |
| Effect scan 47, effect kind 0x34, spawn arguments 0x2b/0x46 | Different domains inside `player_use_item`: last pool index, effect kind and spawn coordinates. They are not fruit, Gold Cross, herb or leaf IDs. |
| Armor record count 42; use-panel labels capacity 50; database endpoint 0x50 | Record/workspace counts and the exclusive 80-item endpoint. They are not Verdite or Wind Blade Bracelet IDs. |

The ten item identities and explicit retained-bit masks remove 43 anonymous
numeric occurrences from the six edited C files. This pass refreshes the complete [shop ledger](game-shop-literal-ledger.md)
from 155 to 153 numeric occurrences and the complete
[floor-script ledger](game-map-script-literal-ledger.md) from 316 to 313.
Other item IDs, menu control sentinels and amounts outside this family remain
for subsequent campaigns; this note does not claim complete global coverage.

## Verification

All sections of all 112 compiled objects are identical to the `07629a3`
baseline, including debug sections. All 484 strict function scores and the
complete objdiff report remain identical. The 16 reviewed functions include
nine exact controls reproducing 1719 complete raw retail words, with the same
ordered calls and numeric data referents. Existing partial matches remain
partial; no new bank rows or compiler profiles are introduced.

Modern compilation retains the identical multiset of 320 diagnostics:
64 source/image variants pass and 48 fail. These are existing type debts,
not successful whole-project modern compilation. Ruff and whitespace checks
pass. No tests, tooling implementation or flake files change in this batch.

The final 656-test run takes 96.731 seconds: 655 pass and the existing untracked
`test_complete_hub_words_referents_and_state_update_slots` fails. The stock-seed
tests accept the named IDs without changes to their independent expected data.
The unrelated save/load-hub source note and test remain untracked and untouched.

Full build retains 8/61 source-data matches and 4/4 SDK/config-data matches.
Target relink remains PSX 1/1, GAME 75/77 and OPEN 34/38, with the existing two
GAME/four OPEN section-base conflicts and zero artifact failures. The build
therefore still exits unsuccessfully; this naming batch does not resolve those
pre-existing ownership/data mismatches.

## Per-function final verdicts

| GAME address | Function | Final strict % | Verdict |
| --- | --- | --- | --- |
| `0x80020cfc` | `item_load_database` | 99.746666 | Unchanged partial match |
| `0x800212d8` | `item_menu_root` | 100.0 | Exact; 152 complete retail words |
| `0x80021538` | `item_menu_buy` | 97.20054 | Unchanged partial match |
| `0x80021afc` | `item_menu_sell` | 96.77187 | Unchanged partial match |
| `0x80022348` | `menu_root` | 96.86364 | Unchanged partial match |
| `0x80022608` | `menu_use_item_panel` | 92.8218 | Unchanged partial match |
| `0x80018054` | `player_use_item` | 100.0 | Exact; 279 complete retail words |
| `0x800151cc` | `game_state_initialize` | 100.0 | Exact; 185 complete retail words |
| `0x800154b0` | `player_death_restart` | 100.0 | Exact; 103 complete retail words |
| `0x80015714` | `player_recalculate_combat_stats` | 100.0 | Exact; 517 complete retail words |
| `0x80015fc0` | `player_increment_magic_training` | 100.0 | Exact; 38 complete retail words |
| `0x80016324` | `player_apply_damage` | 100.0 | Exact; 228 complete retail words |
| `0x80034610` | `map_action_script_floor3` | 100.0 | Exact; 36 complete retail words |
| `0x80034a80` | `map_event_interact` | 100.0 | Exact; 181 complete retail words |
| `0x800238d8` | `menu_equip_select` | 97.89973 | Unchanged partial match |
| `0x80018880` | `player_update` | 96.94554 | Unchanged partial match |
