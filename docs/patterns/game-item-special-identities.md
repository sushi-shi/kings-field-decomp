# GAME special items and illusion staff timer

## Function Match Plan

Start at `79e62a8` on master with hash-verified Japanese retail. Six-view
GAME dossiers, source snapshots and histories are captured under
`build/constant-names/item-special-identities/`. All selected units use
`probe-gcc 257-o2-g0`. These game inventory, input, rendering and map policies
are absent from the vendored roster; linked SDK services remain external.
The source's existing `flask`, `crystals` and `lamp` comments are hypotheses,
not resource evidence. Decode COM/STAT.DAT rows 54..79 using the audited
COM/MIX.TIM glyph atlas before promoting their names.

Name all nineteen nonempty special-item labels and propagate the actual IDs
through key/use dispatch, map selection and dialogue/passage stock accesses.
Seven empty labels remain empty evidence, not invented unused-item semantics.
Keep map-link numbers, effect kinds, geometry, pool indices and item IDs in
their own domains. Reuse existing effect capacity/free-slot/kind constants
and angle/Q12 units where they already state the evidenced contract.
Reuse the map-link inactive sentinel for the key/stone consumers. The harp
constructor stores protocol values in generic effect storage: rename its
six parameter identifiers across definition, declaration and identity to
first segment, segment count, progress per update, cell stagger, sweep
updates and hold countdown. The dispatcher establishes that the final two
values are counters, correcting the earlier item note’s coordinate claim.
Preserve all six parameter widths, stores and the existing function symbol.

Correct the player+0x52 field to `illusion_staff_timer`: Illusion Staff use
sets it to 1000; player update decrements its signed halfword and calls a
green lighting blend; map-cell rendering samples the same timer for a
four-phase mesh substitution. Rename every source/header/curated-field use
and update the existing inventory test's lookup, preserving its offset and
confidence checks. Give the -1 inactive sentinel a staff-specific name.
Preserve the 1001 qualifying lighting calls from initial counter 1000 through
zero, and the final decrement to -1. Original duration/flash tuning is unknown.

Require all 112 object sections except permissible debug lines, all 484
strict scores, complete objdiff report and ordered numeric referents to remain
unchanged. Force affected compilation, compare reviewed exact functions with
raw retail including delay slots, run modern checking, inventory checks,
ruff, repository tests, whitespace and full build before committing. No new
size assertions or per-item tests are introduced. Give every retained literal
in player_use_item.c a consumer-specific explanation.

## Evidence snapshots

| GAME address / bytes | Function | Starting strict % | Constraint |
| --- | --- | --- | --- |
| `0x80020cfc / 1500` | `item_load_database` | 99.746666 | COM/STAT.DAT path, fixed 80 ten-halfword rows and signed -1 glyph terminators anchor the item-name IDs. |
| `0x80022348 / 704` | `menu_root` | 96.86364 | Control: full-word selected item/sentinel return is forwarded through menu_enter_mode. |
| `0x80022608 / 1908` | `menu_use_item_panel` | 92.8218 | Name maps 55/73 in list construction and the map-viewer branch; preserve u8 codes/stock, integer cursor and all redundant range exclusions. |
| `0x80017edc / 200` | `actor_show_info_image` | 100.0 | Control: Mirror of Truth actor target formats floor/actor decimal path digits after two render calls. |
| `0x80017fa4 / 176` | `map_event_show_person_image` | 100.0 | Control: Mirror of Truth event fallback formats character decimal digits after two render calls. |
| `0x80018054 / 1116` | `player_use_item` | 100.0 | Preserve u8 switch and key fallthrough, matching object links, stock consumption, harp pool scan and six O32 spawn arguments. Name existing effect kind/free-slot/capacity and angle/Q12 units. |
| `0x800343e0 / 88` | `map_action_script_floor1` | 100.0 | Possession of byte stock[0][56] reveals copy region 1 once; flag and sound policy remain unchanged. |
| `0x80034a80 / 724` | `map_event_interact` | 100.0 | Character 8 consumes Mirror of Truth for magic 0; character 7 exchanges fruit for harp. Preserve dialogue gates and item/link domain distinction. |
| `0x80022d7c / 1024` | `menu_map_viewer` | 100.0 | Only proven caller passes maps 55/73; compare 55 chooses MAP/M1f., otherwise MAP/M2f.; preserve marker coordinates and two-buffer primitives. |
| `0x80037770 / 172` | `effect_pool_spawn_typed` | 100.0 | Control: harp constructor sets FLOOR_DEFORMATION52 and slot type 0xf0; rename constructor parameters from generic storage lanes to their dispatcher-established segment/progress/countdown meanings; keep widths and stores. |
| `0x80017cf8 / 324` | `player_warp_to_floor_entry` | 100.0 | Control: Green Dragon Staff calls this entry-cell warp; floor 5 conditional resource reload and both shimmer calls remain unchanged. |
| `0x8001e83c / 360` | `render_map_cells` | 100.0 | Control: grid traversal supplies col,row and cell-window class to render_map_cell. |
| `0x80018880 / 6684` | `player_update` | 96.94554 | Signed halfword timer compares -1, decrements, then calls lighting wrapper even when new value is -1. Preserve every existing partial-match instruction. |
| `0x800151cc / 740` | `game_state_initialize` | 100.0 | Initialize the same signed halfword at player+0x52 to -1; byte stock seeds remain unchanged. |
| `0x8001e5ec / 592` | `render_map_cell` | 96.74324 | Signed player+0x52 timer gates mesh remapping 68/69/70 to 23/24/25 for phases 0/1 of its four-state cycle. Rename local and field without changing comparisons. |

## Retail label evidence

The [shop resource audit](game-shop-price-domains.md) establishes item i at
`COM/STAT.DAT` offset `0xcd8 + 20*i`, with ten signed glyph halfwords and -1
termination. SHA256 of its 5708 bytes is
`3f51069ac6291bffdfeb981b14963a22564b40fa5d9034f226797f84247b97f4`.
The independently hash-checked COM/MIX.TIM font and existing decoder render
all 26 rows 54..79 under this campaign's ignored build directory. Seven labels
(65,66,71,72,77,78,79) begin with -1; this proves an empty name, not that their
slots can never be used. Nineteen labels are nonempty:

| ID | Retail Japanese | Source suffix after `KF_ITEM_` | Consumer evidence |
| --- | --- | --- | --- |
| 54 | ライト家の鍵 | `RAITO_FAMILY_KEY` | Keyed-door dispatch. Raito transcribes the katakana family name; no English spelling is claimed. |
| 55 | 番人の地図 | `WATCHMAN_MAP` | Listed first; map viewer selects MAP/M1f. for current floor f. |
| 56 | 竜の杯 | `DRAGON_CHALICE` | Matching object/link interaction; floor 1 possession also opens copy region 1 once. |
| 57 | 幻の杖 | `ILLUSION_STAFF` | Sets the signed player timer and consumes one possessed copy. |
| 58 | 緑竜の杖 | `GREEN_DRAGON_STAFF` | Warps to the current floor's entry; does not consume stock in this branch. |
| 59 | 真理の鏡 | `MIRROR_OF_TRUTH` | Actor info image, then character-image fallback; character 8 dialogue exchanges it for learning magic 0. |
| 60 | 地下牢の鍵 | `DUNGEON_KEY` | Keyed-door dispatch. |
| 61 | 竜の紋章 | `DRAGON_CREST` | Resource identity only; no dedicated use case is inferred. |
| 62 | 竪琴 | `HARP` | Floor 2/3 deformation trigger; character 7 dialogue trades a dragon-grass fruit for it. |
| 63 | 水の封印石 | `WATER_SEAL_STONE` | Matching object/link interaction. |
| 64 | 土の封印石 | `EARTH_SEAL_STONE` | Matching object/link interaction. |
| 67 | 封印の燭台 | `SEAL_CANDLESTICK` | Resource identity only; no dedicated use case is inferred. |
| 68 | 火の封印石 | `FIRE_SEAL_STONE` | Matching object/link interaction. |
| 69 | 風の封印石 | `WIND_SEAL_STONE` | Matching object/link interaction. |
| 70 | 聖の封印石 | `HOLY_SEAL_STONE` | Resource identity only; do not add it to the existing four-stone dispatch. |
| 73 | 魔導師の地図 | `SORCERER_MAP` | Listed second; map viewer selects MAP/M2f. |
| 74 | 魔導師の鍵 | `SORCERER_KEY` | Keyed-door dispatch. |
| 75 | 魔王の手 | `DEMON_KING_HAND` | Resource identity only; enlarged direct glyph rendering resolves 手 (hand). |
| 76 | サラマンダーの像 | `SALAMANDER_STATUE` | Resource identity only; no dedicated use case is inferred. |

These are readings/translations of the Japanese resources, not claims about
localized English proper names. The corrected item-use comment follows the
actual labels instead of the older crystal/flask/lamp interpretations.
Map link 55 remains a link number, not Watchman Map. Cell attribute 58 remains
an attribute, not Green Dragon Staff. The generic map viewer still chooses
its second map for any input other than 55, preserving the retail fallback.

## Illusion staff lifecycle

All current accesses to the signed halfword at `player_state+0x52` now use
`illusion_staff_timer`, including its curated field row and existing inventory
lookup assertions. The initializer and two consumers use
`KF_ILLUSION_STAFF_INACTIVE=-1`. The type, offset and supported confidence stay
unchanged. The staff writes 1000 before the stock guard, so a direct call with
zero stock still starts the effect; the menu normally filters unowned items.
Do not turn that ordering into a possession precondition.

Retail `8001a24c..8001a270` checks old timer != -1, decrements, then calls the
lighting wrapper. Initial 1000 therefore yields 1001 qualifying calls before
inactivity, including the call after zero becomes -1. Early returns from player
update skip this block, so this is not a guaranteed wall-clock duration.
The wrapper blends toward the green color matrix by 0xc00/0x1000, or 75%.

`render_map_cell` reads the same signed halfword at `8001e640`. For active
values with `(timer & 3)<2`, it substitutes model indices 68→23,69→24,70→25
before the usual index decrement/bank selection. This is a four-count cycle,
two states remapped and two unchanged. Keep the literals 3 and 2 as this exact
bit-phase formula; neither is an item ID. The original choice of 1000 counts,
75% blend and four-count flash timing is not established by the label.

## Harp constructor parameters

The harp scan rejects a second active `KF_EFFECT_KIND_FLOOR_DEFORMATION`
record and ignores `KF_EFFECT_SLOT_FREE` entries. It does not consume harp
stock. A successful spawn request plays its existing sound; allocation
failure is not tested by the caller, so preserve its acknowledgement behavior.
The six existing parameter widths remain u16,u16,u16,u16,s32,s32.

| Parameter | Shared storage lane | Floor 2 | Floor 3 | Dispatcher meaning |
| --- | --- | --- | --- | --- |
| `first_segment` | rotation.vx | 0 | 4 | Index of the first floor_deform_segments row. |
| `segment_count` | rotation.vy | 4 | 1 | Consecutive rows processed; together the two configurations cover all five rows. |
| `progress_per_update` | rotation.vz | 150 | 150 | Adds/subtracts 150 Q12 progress units per sweep update. |
| `cell_stagger` | direction.words.y | 800 | 800 | Per-cell Q12 progress step: negated while advancing, positive while reversing. |
| `sweep_updates` | position.vx, initially direction.words.x | 43 | 88 | Successful line-update calls in each sweep, followed by one transition/release call. |
| `hold_countdown` | position.vy | 70 | 270 | Predecrement until -1 gives 71/271 hold-state calls. |

The dispatcher is the semantic consumer, even though the reusable record
lanes are named rotation/position/direction. This corrects the previous
[consumable note](game-item-consumables-accessories.md), which called the last
two arguments coordinates. Parameter renaming makes that correction visible
at the source API while preserving all stored bytes and the function symbol.
The floor 2 rows contain 2,2,3,3 cells; the floor 3 row has 12. The supplied sweep
lengths fully cover their staggered transitions; the original timing margins
and hold durations remain authored values without an established rationale.

## Literal coverage

The complete [item-use ledger](game-item-use-literal-ledger.md) covers 84
remaining occurrences, down from 109, including both target-image helpers and
path buffers. Every retained literal has a specific representation, indexing,
control, authored-amount or unresolved-resource explanation. Unresolved model
and notification identities still require their own resource audits; a reason
for retaining a literal is not proof of its original proper name.
The complete [floor-script ledger](game-map-script-literal-ledger.md) falls
from 313 to 309 after the chalice, mirror and harp stock uses are named.

The batch removes a net 47 anonymous numeric occurrences across the eight edited
C files, adds nineteen item labels and one inactive-timer sentinel, and reuses
the existing angle, fixed-point, audio, effect and map-link constants. Naming
constructor parameters changes no numeric value. The broader item inventory,
remaining unknown fields and other literal families are still incomplete.

## Verification

Every section of all 112 compiled objects is identical to the `79e62a8`
baseline, including debug sections. All 484 strict scores and the complete
objdiff report are unchanged. Ten of the fifteen reviewed functions reproduce
1231 complete retail words, including their ordered calls, numeric referents
and return/branch delay slots. Partial matches remain partial; no bank rows
or compiler profiles change.

Inventory validation passes: 471 functions, 104 structures and 801 field rows.
The existing inventory test now queries the staff timer under its corrected
name, retaining the independently checked offset 0x52 and supported confidence.
Modern compilation retains the exact multiset of 320 existing diagnostics:
64 source/image variants pass and 48 fail. No conversions are suppressed and
no new test or source size assertion is introduced.

Ruff and whitespace checks pass. The 656-test run takes 83.753 seconds: 655 pass,
and the existing untracked save/load-hub raw-word test fails. Its source note
and test remain untouched and untracked. Full build retains 8/61 source-data
matches, 4/4 SDK/config-data matches, and target relink PSX 1/1, GAME 75/77,
OPEN 34/38. The existing two GAME/four OPEN section-base conflicts and other
data mismatches still make the full build exit unsuccessfully; there are no
artifact failures. No tooling implementation or flake change requires a
separate flake check.

## Per-function final verdicts

| GAME address | Function | Final strict % | Verdict |
| --- | --- | --- | --- |
| `0x80020cfc` | `item_load_database` | 99.746666 | Unchanged partial match |
| `0x80022348` | `menu_root` | 96.86364 | Unchanged partial match |
| `0x80022608` | `menu_use_item_panel` | 92.8218 | Unchanged partial match |
| `0x80017edc` | `actor_show_info_image` | 100.0 | Exact; 50 complete retail words |
| `0x80017fa4` | `map_event_show_person_image` | 100.0 | Exact; 44 complete retail words |
| `0x80018054` | `player_use_item` | 100.0 | Exact; 279 complete retail words |
| `0x800343e0` | `map_action_script_floor1` | 100.0 | Exact; 22 complete retail words |
| `0x80034a80` | `map_event_interact` | 100.0 | Exact; 181 complete retail words |
| `0x80022d7c` | `menu_map_viewer` | 100.0 | Exact; 256 complete retail words |
| `0x80037770` | `effect_pool_spawn_typed` | 100.0 | Exact; 43 complete retail words |
| `0x80017cf8` | `player_warp_to_floor_entry` | 100.0 | Exact; 81 complete retail words |
| `0x8001e83c` | `render_map_cells` | 100.0 | Exact; 90 complete retail words |
| `0x80018880` | `player_update` | 96.94554 | Unchanged partial match |
| `0x800151cc` | `game_state_initialize` | 100.0 | Exact; 185 complete retail words |
| `0x8001e5ec` | `render_map_cell` | 96.74324 | Unchanged partial match |
