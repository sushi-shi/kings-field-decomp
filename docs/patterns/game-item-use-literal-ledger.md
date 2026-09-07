# Retained item-use literals

Complete current ledger for `src/game/player_use_item.c`, including both image
path buffers and all three functions: **54 numeric/character occurrences**.
The [range and effect settings review](game-item-use-constants.md) names 23
previously inline uses. The [special-item audit](game-item-special-identities.md)
supplies item/resource identities and consumer evidence.

Every token and complete source-line expression is accounted for, including
duplicates. Claims, comments, strings, identifier digits and named definitions
are excluded. Object identifiers remain pending domain typing; their ledger
rows record current evidence, not an exemption from further naming.

The [floor enum review](game-floor-enum-domain.md) propagates floor identifiers
through the current source; this ledger reflects its named comparisons and
explicit numeric boundaries.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 25 | `14` | `char enemy_info_image_path_template[14] = "ENE0\\EI00.TIM";` | Thirteen path characters plus NUL; fixed buffer extent, not an item or effect ID. |
| `initializers` | 28 | `15` | `char person_image_path_template[15] = "PRSN\\PER00.TIM";` | Fourteen path characters plus NUL; fixed buffer extent, not a gameplay amount. |
| `actor_show_info_image` | 33 | `0` | `render_frame(0, 0);` | Null camera overrides use the current player position/rotation for the two frames preceding the target image. |
| `actor_show_info_image` | 33 | `0` | `render_frame(0, 0);` | Null camera overrides use the current player position/rotation for the two frames preceding the target image. |
| `actor_show_info_image` | 34 | `0` | `render_frame(0, 0);` | Null camera overrides use the current player position/rotation for the two frames preceding the target image. |
| `actor_show_info_image` | 34 | `0` | `render_frame(0, 0);` | Null camera overrides use the current player position/rotation for the two frames preceding the target image. |
| `actor_show_info_image` | 35 | `3` | `enemy_info_image_path_template[3] = '0' + KF_ENUM_ENCODE(u8, player_state.progress_state.current_floor);` | Path byte 3 is the one-based floor digit; ASCII zero converts its numeric value. |
| `actor_show_info_image` | 35 | `'0'` | `enemy_info_image_path_template[3] = '0' + KF_ENUM_ENCODE(u8, player_state.progress_state.current_floor);` | Path byte 3 is the one-based floor digit; ASCII zero converts its numeric value. |
| `actor_show_info_image` | 36 | `7` | `enemy_info_image_path_template[7] = '0' + actor->definition_id / 10;` | Path byte 7 is the actor-definition decimal tens digit; base ten and ASCII zero are representation constants. |
| `actor_show_info_image` | 36 | `'0'` | `enemy_info_image_path_template[7] = '0' + actor->definition_id / 10;` | Path byte 7 is the actor-definition decimal tens digit; base ten and ASCII zero are representation constants. |
| `actor_show_info_image` | 36 | `10` | `enemy_info_image_path_template[7] = '0' + actor->definition_id / 10;` | Path byte 7 is the actor-definition decimal tens digit; base ten and ASCII zero are representation constants. |
| `actor_show_info_image` | 37 | `8` | `enemy_info_image_path_template[8] = '0' + actor->definition_id % 10;` | Path byte 8 is the actor-definition decimal ones digit; modulo ten and ASCII zero are representation constants. |
| `actor_show_info_image` | 37 | `'0'` | `enemy_info_image_path_template[8] = '0' + actor->definition_id % 10;` | Path byte 8 is the actor-definition decimal ones digit; modulo ten and ASCII zero are representation constants. |
| `actor_show_info_image` | 37 | `10` | `enemy_info_image_path_template[8] = '0' + actor->definition_id % 10;` | Path byte 8 is the actor-definition decimal ones digit; modulo ten and ASCII zero are representation constants. |
| `map_event_show_person_image` | 44 | `0` | `render_frame(0, 0);` | Null camera overrides use the current player position/rotation for the two frames preceding the target image. |
| `map_event_show_person_image` | 44 | `0` | `render_frame(0, 0);` | Null camera overrides use the current player position/rotation for the two frames preceding the target image. |
| `map_event_show_person_image` | 45 | `0` | `render_frame(0, 0);` | Null camera overrides use the current player position/rotation for the two frames preceding the target image. |
| `map_event_show_person_image` | 45 | `0` | `render_frame(0, 0);` | Null camera overrides use the current player position/rotation for the two frames preceding the target image. |
| `map_event_show_person_image` | 46 | `8` | `person_image_path_template[8] = '0' + event->character_id / 10;` | Path byte 8 is the character decimal tens digit; base ten and ASCII zero are representation constants. |
| `map_event_show_person_image` | 46 | `'0'` | `person_image_path_template[8] = '0' + event->character_id / 10;` | Path byte 8 is the character decimal tens digit; base ten and ASCII zero are representation constants. |
| `map_event_show_person_image` | 46 | `10` | `person_image_path_template[8] = '0' + event->character_id / 10;` | Path byte 8 is the character decimal tens digit; base ten and ASCII zero are representation constants. |
| `map_event_show_person_image` | 47 | `9` | `person_image_path_template[9] = '0' + event->character_id % 10;` | Path byte 9 is the character decimal ones digit; modulo ten and ASCII zero are representation constants. |
| `map_event_show_person_image` | 47 | `'0'` | `person_image_path_template[9] = '0' + event->character_id % 10;` | Path byte 9 is the character decimal ones digit; modulo ten and ASCII zero are representation constants. |
| `map_event_show_person_image` | 47 | `10` | `person_image_path_template[9] = '0' + event->character_id % 10;` | Path byte 9 is the character decimal ones digit; modulo ten and ASCII zero are representation constants. |
| `player_use_item` | 69 | `0` | `u8 used = 0;` | No applicable use action has been acknowledged yet; Boolean false. |
| `player_use_item` | 73 | `0` | `index = 0;` | Begin the object-pool search at zero-based slot zero. |
| `player_use_item` | 81 | `1` | `if (index == -1) {` | Negative query-miss sentinel ends the object scan, outside nonnegative pool indices. |
| `player_use_item` | 86 | `81` | `case 81:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 87 | `83` | `case 83:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 88 | `85` | `case 85:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 89 | `89` | `case 89:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 90 | `117` | `case 117:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 91 | `118` | `case 118:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 92 | `119` | `case 119:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 93 | `120` | `case 120:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 94 | `121` | `case 121:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 95 | `122` | `case 122:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 98 | `89` | `} else if (object->object_id != 89` | Only object model 89 applies the facing test before key use; proper model name remains unresolved. |
| `player_use_item` | 101 | `1` | `used = 1;` | Boolean true acknowledges an applicable action, including an examined door with a wrong key. |
| `player_use_item` | 105 | `89` | `if (object->object_id == 89) {` | Only model 89 adds the second unlock sound; retain this authored model-specific rule without inventing a model name. |
| `player_use_item` | 124 | `1` | `if (index == -1) {` | Negative query-miss sentinel ends the object scan, outside nonnegative pool indices. |
| `player_use_item` | 132 | `0` | `item_stock[0][object->object_id] = 0;` | Clear all player-owned copies in stock bank zero when the matching chalice/seal-stone object triggers its link. |
| `player_use_item` | 132 | `0` | `item_stock[0][object->object_id] = 0;` | Clear all player-owned copies in stock bank zero when the matching chalice/seal-stone object triggers its link. |
| `player_use_item` | 133 | `1` | `used = 1;` | Boolean true acknowledges an applicable action, including an examined door with a wrong key. |
| `player_use_item` | 143 | `1` | `for (slot = KF_EFFECT_CAPACITY - 1; slot != -1; slot--, record++) {` | Inclusive countdown from the last of 48 records through slot 0, stopping at -1. Each step advances the record pointer once. |
| `player_use_item` | 143 | `1` | `for (slot = KF_EFFECT_CAPACITY - 1; slot != -1; slot--, record++) {` | Inclusive countdown from the last of 48 records through slot 0, stopping at -1. Each step advances the record pointer once. |
| `player_use_item` | 165 | `1` | `used = 1;` | Boolean true acknowledges an applicable action, including an examined door with a wrong key. |
| `player_use_item` | 172 | `1` | `used = 1;` | Boolean true acknowledges an applicable action, including an examined door with a wrong key. |
| `player_use_item` | 179 | `0` | `if (item_stock[0][KF_ITEM_ILLUSION_STAFF] != 0) {` | Only decrement a possessed player-bank copy; the timer write already happened before this guard. |
| `player_use_item` | 179 | `0` | `if (item_stock[0][KF_ITEM_ILLUSION_STAFF] != 0) {` | Only decrement a possessed player-bank copy; the timer write already happened before this guard. |
| `player_use_item` | 180 | `0` | `item_stock[0][KF_ITEM_ILLUSION_STAFF]--;` | Consume one copy from the player-owned bank zero. |
| `player_use_item` | 190 | `0` | `if (actor != 0) {` | A non-null actor wins the mirror query and its info image is shown immediately. |
| `player_use_item` | 200 | `0` | `if (event == 0) {` | No event target after the actor miss leaves the mirror use unacknowledged. |
| `player_use_item` | 207 | `1` | `used = 1;` | Boolean true acknowledges an applicable action, including an examined door with a wrong key. |
