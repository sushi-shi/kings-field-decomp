# Inventory item-ID domain

## Function Match Plan

Make the existing seventy item names, including the absent value 0xff, a
`KfItemId` byte enum. Propagate it through all seven equipped-item fields, five
byte arrays used by the buy/sell/equip/drop/use lists, the three byte-argument
player operations, and the six word-argument item presentation/pickup helpers.
Keep each legacy C parameter width through the existing enum macros.

Item loops still count integer table positions. Mixed menu results still carry
negative control values as well as item selections. Encode/decode at those
boundaries instead of narrowing their control state. The shared list widget
also previews magic icons; decode its integer payload only in item branches.
Decode the optional pickup byte after its menu mode is selected. Encode items
at resource/table indices, weapon filename digits, and object/link-ID boundaries.
The equipment-range constants describe integer iteration endpoints and the
armor-table origin; keep them as encoded numeric values.

The [equipment evidence](game-equipment-domains.md),
[retail item labels](game-item-equipment-identities.md),
[consumable/accessory identities](game-item-consumables-accessories.md),
[item-use flow](game-item-use-flow.md), [shop controls](game-shop-menu-controls.md)
and [pickup results](game-item-pickup-outcomes.md) establish these game-owned
interfaces and their consumers. Current source, curated field/signature widths,
source history and cached image-specific disassembly/CFG/caller/callee/string
and historical match dossiers were reviewed. Stored matches are not validation
of these edits. Authentic SDK/resource APIs keep their integer contracts.

All addresses identify GAME.EXE. The following related functions either need
an explicit source boundary or inherit typed fields/constants through the shared
headers. The byte stores/loads in equipment and menu code constrain storage;
fixed word parameters remain word parameters in the retail C view.

| Function | Address / extent | Retail frame bytes | Intended scope |
| --- | --- | ---: | --- |
| `item_menu_buy` | `0x80021538 / 0x5c4` | 1856 | Byte item-list storage; integer iteration and mixed result boundaries; typed item calls. |
| `item_menu_sell` | `0x80021afc / 0x500` | 1856 | Byte item-list storage; integer iteration and mixed result boundaries; typed item calls. |
| `item_pickup_confirm` | `0x80021ffc / 0x2b8` | 96 | Typed item argument with unchanged legacy signed-word ABI; numeric table/resource boundary. |
| `map_restore_floor_state` | `0x80035e44 / 0x69c` | 40 | Shared item constants/fields; encode table indices where consumed. |
| `map_action_script_floor1` | `0x800343e0 / 0x58` | 24 | Shared item constants/fields; encode table indices where consumed. |
| `map_action_script_floor3` | `0x80034610 / 0x90` | 24 | Shared item constants/fields; encode table indices where consumed. |
| `map_floor5_transition_cutscene` | `0x800346a8 / 0x38c` | 208 | Shared item constants/fields; encode table indices where consumed. |
| `map_event_interact` | `0x80034a80 / 0x2d4` | 32 | Shared item constants/fields; encode table indices where consumed. |
| `menu_use_item_panel` | `0x80022608 / 0x774` | 1208 | Byte item-list storage; integer iteration and mixed result boundaries; typed item calls. |
| `menu_draw_name_list` | `0x8002718c / 0x838` | 48 | Shared item constants/fields; encode table indices where consumed. |
| `menu_enter_mode` | `0x80036e38 / 0xc8` | 24 | Decode optional pickup byte after mode dispatch; retain shop and root payloads. |
| `menu_draw_item_detail` | `0x80027b7c / 0x2dc` | 160 | Typed item argument with unchanged legacy signed-word ABI; numeric table/resource boundary. |
| `menu_drop_item` | `0x800249a8 / 0x4bc` | 1856 | Byte item-list storage; integer iteration and mixed result boundaries; typed item calls. |
| `menu_item_model_preview` | `0x800279c4 / 0x1b8` | 152 | Typed item argument with unchanged legacy signed-word ABI; numeric table/resource boundary. |
| `menu_list_interact` | `0x80028380 / 0x354` | 104 | Decode the shared preview payload in item branches; retain magic-icon and integer widget protocol. |
| `menu_map_viewer` | `0x80022d7c / 0x400` | 224 | Typed item argument with unchanged legacy signed-word ABI; numeric table/resource boundary. |
| `menu_option_root` | `0x800236ac / 0x22c` | 48 | Shared item constants/fields; encode table indices where consumed. |
| `menu_draw_item_name_frame` | `0x800292f8 / 0x7b8` | 224 | Typed item argument with unchanged legacy signed-word ABI; numeric table/resource boundary. |
| `menu_load_item_model` | `0x8002aea4 / 0x68` | 32 | Typed item argument with unchanged legacy signed-word ABI; numeric table/resource boundary. |
| `menu_equip_select` | `0x800238d8 / 0x5c4` | 520 | Byte item-list storage; integer iteration and mixed result boundaries; typed item calls. |
| `player_set_equipment_slot` | `0x80016848 / 0x1e8` | 24 | Byte item argument; typed equipment or use dispatch; numeric resource/filename/link boundaries. |
| `player_equip_weapon` | `0x80016a30 / 0xf4` | 24 | Byte item argument; typed equipment or use dispatch; numeric resource/filename/link boundaries. |
| `player_begin_weapon_attack` | `0x80016b24 / 0x9c` | 24 | Shared item constants/fields; encode table indices where consumed. |
| `player_update_weapon_attack` | `0x80016bc0 / 0x264` | 120 | Shared item constants/fields; encode table indices where consumed. |
| `player_update_vertical_motion` | `0x80017a80 / 0x278` | 40 | Shared item constants/fields; encode table indices where consumed. |
| `game_state_initialize` | `0x800151cc / 0x2e4` | 24 | Shared item constants/fields; encode table indices where consumed. |
| `player_death_restart` | `0x800154b0 / 0x19c` | 24 | Shared item constants/fields; encode table indices where consumed. |
| `player_recalculate_combat_stats` | `0x80015714 / 0x814` | 24 | Shared item constants/fields; encode table indices where consumed. |
| `player_apply_damage` | `0x80016324 / 0x390` | 56 | Shared item constants/fields; encode table indices where consumed. |
| `player_update` | `0x80018880 / 0x1a1c` | 224 | Shared item constants/fields; encode table indices where consumed. |
| `player_use_item` | `0x80018054 / 0x45c` | 72 | Byte item argument; typed equipment or use dispatch; numeric resource/filename/link boundaries. |

## Representation and remaining identities

`KfItemId` describes inventory database IDs, not item-stock bank indices, map
object IDs, equipment category/slot IDs, or the
[floor decoration base sprite index](floor-sprite-base-index.md). The latter
retains its independent halfword index and 0xffff stream terminator. The packed
object container payload and common menu result are decoded only after their
consuming branch establishes an item meaning.

The byte enum preserves unnamed/unused database slots; it does not validate a
range or assign names to unproven entries. Existing numeric values, control
branches, authored table contents, RNG and call order must remain unchanged.
Initialization's item-zero argument with refresh-only equipment slot remains
an explicit decode because that argument is ignored, not a request to equip a
Short Sword. The save code copies the existing player bytes unchanged.

No builds, compiler checks, tests, post-edit matches or banking will run until
naming is finished. Reconcile current literal expressions and review every
changed boundary in source; do not claim byte matching from that accounting.

## Source result

The seventy existing names now form `KfItemId`. All seven equipped-item fields,
five item-list byte arrays and nine item-specific interfaces carry that domain.
Seventeen C files need explicit edits; `menu_option_root` inherits the typed
equipment comparison through its shared header. Field offsets, enum values,
stock contents and identity evidence tiers are unchanged.

The shared list widget decodes its payload with
`KF_ENUM_DECODE(KF_ENUM_PARAM(KfItemId, s32), item_id)` in each item branch.
The inner macro preserves the original signed-word C argument; decoding directly
to the byte typedef there would introduce a new narrowing operation. Byte
decodes remain at the old byte stores, byte parameters and optional pickup-byte
read. Signed selection/control values remain integers until those boundaries.

Every function in the plan has the same final verdict: source propagation
reviewed, post-edit compiler and match verification deferred. The review covered
all current calls of the nine interfaces, equipped-field uses, list storage,
named item comparisons and integer/resource boundaries. Eleven retained-literal
ledgers were reconciled to their changed expressions. All 111 C files still
account for 6,007 occurrences with individual reasons; this establishes source
documentation coverage only. Unresolved item slots and the broader naming goal
remain open.
