# GAME equipment names and corrected slot identities

## Function Match Plan

Start at `7d6d8f9` on master with hash-verified Japanese retail. Six-view
GAME dossiers, source snapshots, histories and decoded labels are captured
under `build/constant-names/item-equipment-identities/`. All selected units
use `probe-gcc257-o2-g0`. Their inventory, player, map and menu policy is
absent from the vendored roster; SDK and other library calls stay external.

Decode item rows0..41 and window1's nine menu labels. The labels independently
contradict the old shield/head/body naming: IDs13..18 are head equipment,
19..25 are body armor, and26..31 are shields. The raw category switches and
slot writes establish the corresponding byte fields and record pointers.
Correct the three naming cycles simultaneously across headers, enums, source,
curated fields and durable documentation. Preserve each field's physical
position, enum value, record origin, call argument and ordered referent.
The old type names cannot be treated as semantic ground truth.

Name all38 nonempty rows and their actual source uses: stock seeds, equipped
item comparisons, weapon magic switches, Full Plate restrictions and the
Dragon Sword/Moonlight Sword transformation/reload path. Four empty labels
remain unnamed; preserve their current range membership. Gold Coin, Blood
Stone and Moonstone are named resource rows, not evidence to invent extra
pickup behavior. Preserve the refresh-only setter's ignored item-zero input.
Reuse decoded item identities for equipment range origins/endpoints where
appropriate; record capacities retain their own meanings.

Require unchanged sections of all112 objects (apart from allowable debug
lines), all484 strict scores, the complete objdiff report and physical
referents. Force affected compilation and compare exact controls against raw
retail including delay slots. Run modern compilation, inventory validation,
ruff, repository tests, whitespace and full build before commit. No new size
assertions or per-item tests. Refresh complete equipment and map ledgers;
explain retained special-item timing/bonus values without inventing tuning.

## Evidence snapshots

| GAME address / bytes | Function | Starting strict % | Constraint |
| --- | --- | --- | --- |
| `0x80020cfc / 1500` | `item_load_database` | 99.746666 | STAT.DAT loader fixes the 80-name row origin and 20-byte stride; item labels and menu labels are independent resource views. |
| `0x80021538 / 1476` | `item_menu_buy` | 97.20054 | Control: shop ordering and availability reads must remain unchanged. |
| `0x80021afc / 1280` | `item_menu_sell` | 96.77187 | Propagate corrected equipped-ID field identities through exclusion tests; preserve byte quantities and all sold-item rules. |
| `0x80021ffc / 696` | `item_pickup_confirm` | 100.0 | Control: generic item-ID handling and stack-capacity protocol remain unchanged. |
| `0x800236ac / 556` | `menu_option_root` | 100.0 | Retail menu row2=shield,row3=head,row4=body; Full Plate21 blocks arm/leg rows, not a special helmet. |
| `0x800238d8 / 1476` | `menu_equip_select` | 97.89973 | Correct category names/range constants/slot calls together. Raw rows2/3/4 store to player+92/+90/+91 and pass slot4/0/1. Full Plate clears arms/legs. |
| `0x8002718c / 2104` | `menu_draw_name_list` | 100.0 | Label-row order and reads match shield+92,head+90,body+91; preserve twenty-pixel row stride and empty-name handling. |
| `0x80016848 / 488` | `player_set_equipment_slot` | 100.0 | Slot0/1/4 store byte IDs at+90/+91/+92 and resolve pointers+7c/+80/+84: head/body/shield. Correct names only, never rearrange fields. |
| `0x80016a30 / 244` | `player_equip_weapon` | 100.0 | Control: byte ID and 44-byte record stride; path formatting and stale-pointer-on-empty behavior unchanged. |
| `0x80016b24 / 156` | `player_begin_weapon_attack` | 100.0 | Control: weapon-none gate, animation and charging remain unchanged. |
| `0x80016bc0 / 612` | `player_update_weapon_attack` | 100.0 | Weapon3 is Colichemarde; preserve its conditional fixed attack-offset path and SDK/O32 call widths. |
| `0x80017a80 / 632` | `player_update_vertical_motion` | 100.0 | Leg item38 is Feather Boots; preserves the special attribute93 exception to the existing fatal-drop test. |
| `0x800151cc / 740` | `game_state_initialize` | 100.0 | Name all seeded item indices and starting Short Sword; keep refresh-only slot call item0 as ignored input and all initialization ordering. |
| `0x800154b0 / 412` | `player_death_restart` | 100.0 | Control: fruit revival and complete initialization fallback remain unchanged. |
| `0x80015714 / 2068` | `player_recalculate_combat_stats` | 100.0 | Rename head/body/shield field accesses and identify Black Mask16 physical-power penalty; preserve repeated attack-lane additions and unsigned storage. |
| `0x80016324 / 912` | `player_apply_damage` | 100.0 | Control: accessory-based darkness immunity and all damage lanes remain unchanged. |
| `0x80018880 / 6684` | `player_update` | 96.94554 | Correct equipped fields; Skull Armor23 gates cast/charge block. Name weapon switch IDs3/7/8/11 and Shadow Blade9 environment condition without changing timing or queued effects. |
| `0x800343e0 / 88` | `map_action_script_floor1` | 100.0 | Control: named Dragon Chalice possession and passage flag remain unchanged. |
| `0x80034610 / 144` | `map_action_script_floor3` | 100.0 | Control: named bracelet possession and learned flags remain unchanged. |
| `0x800346a8 / 908` | `map_floor5_transition_cutscene` | 100.0 | Dragon Sword10 becomes displayed Moonlight Sword11; inventory and displayed object IDs align in this explicit transformation path. |
| `0x80034a80 / 724` | `map_event_interact` | 100.0 | Control: named item exchanges and dialogue sequence unchanged. |
| `0x80035e44 / 1692` | `map_restore_floor_state` | 99.96454 | Named Dragon Sword/Moonlight Sword stock checks gate floor5 link clearing; preserve unrelated link52 domain. |
| `0x800150a8 / 84` | `weapon_records_load_and_mirror_angles` | 100.0 | Control: sixteen 44-byte records; selectable weapon band still includes blank row12 and stops before Iron Mask13. |
| `0x800150fc / 44` | `armor_records_load` | 100.0 | Control:42 records with item origin13; capacity is not the count of named equipment. |
| `0x800187a4 / 76` | `lighting_apply_weapon9_environment` | 100.0 | Control: Shadow Blade caller uses existing function symbol, blend2500/4096 and half fog-near; no symbol or instruction changes. |

## Resource evidence and names

`COM/STAT.DAT` is 5708 bytes, SHA-256
`3f51069ac6291bffdfeb981b14963a22564b40fa5d9034f226797f84247b97f4`.
Item row `i` starts at `0xcd8 + 20*i`, with ten signed halfword glyphs.
Window `w`, row `n` starts its glyphs at `0x390 + w*264 + 24 + n*24 + 4`.
`COM/MIX.TIM` is 331488 bytes, SHA-256
`9e1031c32ea9efc2d124ae97dd6f2a0291bc88153b74b8c1d57f99abefd55959`.
Its fifth TIM starts at 231552; the sixteen RGB555 palette entries start at
231572 and the 32768-byte image starts at 232096. Decode the 256×256 image
as low-nibble-first 4bpp pixels. Glyph cells are 14×12 pixels, sixteen per row.
The low twelve code bits select a cell; `0x1000` overlays dakuten cell `0x2e`,
`0x2000` overlays handakuten cell `0x2f`, -1 terminates, and 255 is blank.
Ignored scripts and renders in the campaign directory preserve the decoding.

These identifiers translate or transcribe the Japanese resource labels; they
do not claim an official English localization. `KF_ITEM_` prefixes are omitted
from the table.

| ID | Retail label | Identifier |
| --- | --- | --- |
| 0 | ショートソード | `SHORT_SWORD` |
| 1 | バトルアックス | `BATTLE_AXE` |
| 2 | ナイトソード | `KNIGHT_SWORD` |
| 3 | コリシュマルド | `COLICHEMARDE` |
| 4 | モーニングスター | `MORNING_STAR` |
| 5 | バスタードソード | `BASTARD_SWORD` |
| 6 | クレセントアックス | `CRESCENT_AXE` |
| 7 | トリプルファング | `TRIPLE_FANG` |
| 8 | フレイムソード | `FLAME_SWORD` |
| 9 | シャドウブレード | `SHADOW_BLADE` |
| 10 | ドラゴンソード | `DRAGON_SWORD` |
| 11 | ムーンライトソード | `MOONLIGHT_SWORD` |
| 13 | アイアンマスク | `IRON_MASK` |
| 14 | ナイトヘルム | `KNIGHT_HELM` |
| 15 | グレートヘルム | `GREAT_HELM` |
| 16 | ブラックマスク | `BLACK_MASK` |
| 17 | ミストクラウン | `MIST_CROWN` |
| 19 | ブレストプレート | `BREASTPLATE` |
| 20 | ナイトプレート | `KNIGHT_PLATE` |
| 21 | フルプレート | `FULL_PLATE` |
| 22 | ファイアメイル | `FIRE_MAIL` |
| 23 | スカルアーマー | `SKULL_ARMOR` |
| 24 | フォレストアーマー | `FOREST_ARMOR` |
| 26 | スモールシールド | `SMALL_SHIELD` |
| 27 | ナイトシールド | `KNIGHT_SHIELD` |
| 28 | タワーシールド | `TOWER_SHIELD` |
| 29 | ブラックドラゴン | `BLACK_DRAGON` |
| 30 | ウィンドガード | `WIND_GUARD` |
| 32 | ガントレット | `GAUNTLET` |
| 33 | ナイトグローブ | `KNIGHT_GLOVE` |
| 34 | ストーンハンド | `STONE_HAND` |
| 35 | アイアンブーツ | `IRON_BOOTS` |
| 36 | ナイトブーツ | `KNIGHT_BOOTS` |
| 37 | レッグガーダー | `LEG_GUARDS` |
| 38 | フェザーブーツ | `FEATHER_BOOTS` |
| 39 | 金貨 | `GOLD_COIN` |
| 40 | ブラッドストーン | `BLOODSTONE` |
| 41 | ムーンストーン | `MOONSTONE` |

Rows 12, 18, 25 and 31 are empty. Together with the previous 31 named rows,
this names all 69 nonempty labels in the eighty-row table. The other empty
rows are 65, 66, 71, 72 and 77–79. Empty labels do not prove unused records;
range membership and the sixteen-weapon/42-armor record capacities remain.

## Corrected equipment identity

Window 1 reads 武器, 魔法, 防具・盾, 防具・頭, 防具・胴, 防具・腕,
防具・足, その他, もどる: weapon, magic, shield, head, body, arms, legs,
other/accessory, return. This independently confirms the item-name grouping.

| Meaning | Menu row | Setter slot | Player ID offset | Record pointer offset | Item interval |
| --- | --- | --- | --- | --- | --- |
| Head | 3 | 0 | `0x90` | `0x7c` | [13,19) |
| Body | 4 | 1 | `0x91` | `0x80` | [19,26) |
| Shield | 2 | 4 | `0x92` | `0x84` | [26,32) |
| Arms | 5 | 2 | `0x93` | `0x88` | [32,35) |
| Legs | 6 | 3 | `0x94` | `0x8c` | [35,39) |
| Accessory | 7 | 5 | `0x95` | — | [48,52) |

All addresses below belong to GAME. Range switch table `0x80012310` sends
menu rows 2/3/4 to `0x8002395c/0x80023968/0x80023974`. Those paths select
26..31, 13..18, 19..25 respectively. Write table `0x80012330` sends them to
`0x80023dac/0x80023dc8/0x80023de4`, storing player bytes `+92/+90/+91`
and passing setter slots 4/0/1. Setter table `0x80012030` independently
establishes the six slot writes and the ensuing pointer refresh.

The previous head/body/shield names were wrong. This correction renames
their fields, slot/category constants, ranges and all uses together; it does
not reorder storage or change numeric call arguments. In particular, the
previous “special helmet” explanations referred to body armor. Curated field
descriptions and the earlier equipment/stat notes now carry the correction.

## Special behaviors and remaining literals

Full Plate (21) clears the separate arm/leg equipment when selected and blocks
those two menu rows while worn. Skull Armor (23) gates the input casting and
charging block in `player_update`; the queued weapon-magic emission block is
outside that guard, so this does not establish that every magic effect is
disabled. Black Mask (16) subtracts eight physical-power points during stat
recalculation; the existing unsigned storage and later cap remain significant.
The original balance rationale for eight is unknown.

Feather Boots (38) bypass the fatal-drop branch only when the cell attribute
is 93 and `floor_height - target < -3000`. The attribute remains numeric
because its general meaning is unproven. This is a specific exception, not
general fall or hazard immunity; the separate attribute-82 death check remains.

Colichemarde (3) selects attack-phase window [1000,1300), while other weapons
use [3072,3372); phase advances by 300 per active attack update. The unsigned
16-bit subtraction tests are preserved. The selected overlap probe has a
1000-world-unit vertical offset and uses the weapon record's forward offset.
These are observed timing and geometry, with no historical tuning explanation.

Weapon-magic input windows remain [2400,3900] for Flame Sword (8), Triple
Fang (7) and Moonlight Sword (11), and [900,2400] for Colichemarde. Flame
Sword queues `(3900-phase)/300+1` shots; Colichemarde doubles that expression;
the other two queue one. Triple Fang and Moonlight Sword require physical
power and magic at least 80; Colichemarde requires both at least 60. Flame
Sword instead tests learned magic record 5. Their effect-kind/record/delay
triples remain 20/20/3, 36/18/3, 8/8/1, and 5/5/2 respectively. Effect and
magic-record numbers are separate domains from item IDs. No wall-clock timing
or reason for the authored thresholds is inferred.

Shadow Blade (9) selects the existing `lighting_apply_weapon9_environment`
symbol: a 2500/4096 (61.03515625%) Q12 color blend and half fog-near distance.
The precise blend's artistic rationale remains unknown. The floor-5 cutscene
removes the Dragon Sword (10), displays its model and replaces that display
with Moonlight Sword (11). Only this explicit model/item correspondence uses
the item constants. The reload condition checks either sword or the existing
transformation flag before clearing link 52, which remains a link ID.

Stock seed values stay one: a player quantity in bank zero and shop
availability in banks one/two. The refresh-only equipment setter's item-zero
argument is ignored and stays numeric. Gold Coin, Bloodstone and Moonstone
names do not imply additional pickup behaviors.

The refreshed [equipment ledger](game-equipment-literal-ledger.md) explains
all 146 retained occurrences in 100 expression groups, down from 147.
The [floor-script ledger](game-map-script-literal-ledger.md) explains all
305 in 175 groups, down from 309. These are scoped inventories, not a claim
that every remaining literal elsewhere has been audited.

## Verification and verdicts

Forced compilation and comparison passed. All sections of all 112 objects,
all 484 strict scores and the complete objdiff report are unchanged from
`7d6d8f9`; even debug-line sections are identical. Each of the 25 functions
in the evidence table retains its starting percentage: the 19 exact entries
remain exact, and the six partial entries remain partial. The exact controls
also match 2922 complete retail words, including delay slots and ordered
call/address referents. No new function is banked.

Inventory validation, `ruff check scripts tests` and `git diff --check` pass.
Modern compilation remains 64/112 passing, with the identical multiset of
320 diagnostics across 48 failing variants. The repository suite ran 656
tests in 105.791 seconds: 655 pass; the existing untracked save/load-hub test
`test_complete_hub_words_referents_and_state_update_slots` still fails its
retail-word comparison. That unrelated test is left untouched.

Full `kf build` was run and remains unsuccessful with the existing data
divergences/ownership gaps. Source data matches remain 8/61, independent
config contributions 4/4, and target relinks PSX 1/1, GAME 75/77, OPEN 34/38.
The same two GAME/four OPEN section-base conflicts remain, with zero artifact
failures. This batch introduces no new diagnostics or matching regressions.
