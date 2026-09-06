# Retained item-use literals

Complete per-occurrence ledger for `src/game/player_use_item.c`, including its
two image-path buffers and target-image helpers, after the
[special-item audit](game-item-special-identities.md). All **84 remaining
occurrences**, down from 109, have explicit consumer explanations. Claims,
comments, strings and named enum definitions are excluded; signs are operators.

| Function | Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- | --- |
| `initializers` | 8 | `14` | `char enemy_info_image_path_template[14] = "ENE 0\\EI 00.TIM";` | Thirteen path characters plus NUL; fixed buffer extent, not an item or effect ID. |
| `initializers` | 11 | `15` | `char person_image_path_template[15] = "PRSN\\PER 00.TIM";` | Fourteen path characters plus NUL; fixed buffer extent, not a gameplay amount. |
| `actor_show_info_image` | 16, 17 | `0 × 4` | `render_frame(0, 0);` | Null camera overrides use the current player position/rotation for the two frames preceding the target image. |
| `actor_show_info_image` | 18 | `3, '0'` | `enemy_info_image_path_template[3] = '0' + player_state.progress_state.current_floor;` | Path byte 3 is the one-based floor digit; ASCII zero converts its numeric value. |
| `actor_show_info_image` | 19 | `7, '0', 10` | `enemy_info_image_path_template[7] = '0' + actor->definition_id / 10;` | Path byte 7 is the actor-definition decimal tens digit; base ten and ASCII zero are representation constants. |
| `actor_show_info_image` | 20 | `8, '0', 10` | `enemy_info_image_path_template[8] = '0' + actor->definition_id % 10;` | Path byte 8 is the actor-definition decimal ones digit; modulo ten and ASCII zero are representation constants. |
| `map_event_show_person_image` | 27, 28 | `0 × 4` | `render_frame(0, 0);` | Null camera overrides use the current player position/rotation for the two frames preceding the target image. |
| `map_event_show_person_image` | 29 | `8, '0', 10` | `person_image_path_template[8] = '0' + event->character_id / 10;` | Path byte 8 is the character decimal tens digit; base ten and ASCII zero are representation constants. |
| `map_event_show_person_image` | 30 | `9, '0', 10` | `person_image_path_template[9] = '0' + event->character_id % 10;` | Path byte 9 is the character decimal ones digit; modulo ten and ASCII zero are representation constants. |
| `player_use_item` | 52 | `0` | `u8 used = 0;` | No applicable use action has been acknowledged yet; Boolean false. |
| `player_use_item` | 54 | `1000` | `reach_x = player_state.camera_position.vx - ((rsin(player_state.camera_rotation.vy) * 1000) >> KF_FIXED12_BITS);` | Forward probe is 1000 world units (half a map tile) from the camera; signed sine/cosine direction is converted from Q12. Original reach tuning unproven. |
| `player_use_item` | 55 | `1000` | `reach_z = player_state.camera_position.vz + ((rcos(player_state.camera_rotation.vy) * 1000) >> KF_FIXED12_BITS);` | Forward probe is 1000 world units (half a map tile) from the camera; signed sine/cosine direction is converted from Q12. Original reach tuning unproven. |
| `player_use_item` | 56 | `0` | `index = 0;` | Begin the object-pool search at zero-based slot zero. |
| `player_use_item` | 63, 106 | `800 × 2` | `index = map_object_pool_find_interaction_from(index, reach_x, reach_z, 800);` | 800 world units of radius padding around the forward query point (0.4 map tile); original interaction tolerance choice is unproven. |
| `player_use_item` | 64, 107 | `1 × 2` | `if (index == -1) {` | Negative query-miss sentinel ends the object scan, outside nonnegative pool indices. |
| `player_use_item` | 69 | `81` | `case 81:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 70 | `83` | `case 83:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 71 | `85` | `case 85:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 72 | `89` | `case 89:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 73 | `117` | `case 117:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 74 | `118` | `case 118:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 75 | `119` | `case 119:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 76 | `120` | `case 120:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 77 | `121` | `case 121:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 78 | `122` | `case 122:` | Authored object-model ID eligible for keyed-link handling. Model identities need mesh/resource evidence; these are not map behavior/action enum values with coincident numbers. |
| `player_use_item` | 80, 113, 190 | `0x12 × 3` | `notify_enqueue(0x12);` | Notification 18 for no applicable use, an already cleared link or no target; localized message text remains undecoded. |
| `player_use_item` | 81 | `89` | `} else if (object->object_id != 89` | Only object model 89 applies the facing test before key use; proper model name remains unresolved. |
| `player_use_item` | 83 | `12` | `player_state.camera_rotation.vy, KF_ANGLE_HALF_TURN - object->rotation.y, KF_ANGLE_FULL_TURN / 12)) {` | Door 89 uses one-twelfth-turn tolerance, truncated to 341/4096 revolution (29.970703125 degrees), around the mirrored half-turn facing. Original tolerance tuning unproven. |
| `player_use_item` | 84, 116, 142, 149, 184 | `1 × 5` | `used = 1;` | Boolean true acknowledges an applicable action, including an examined door with a wrong key. |
| `player_use_item` | 87 | `0x6e` | `sound_ref_play(&gameplay_sound_ref_12, 0x6e);` | Authored key-unlock cue volume 110 out of 127; original mix choice unproven. |
| `player_use_item` | 88 | `89` | `if (object->object_id == 89) {` | Only model 89 adds the second unlock sound; retain this authored model-specific rule without inventing a model name. |
| `player_use_item` | 92 | `4` | `notify_enqueue(4);` | Notification 4 for an applicable door whose required key ID differs from the used key; localized message text remains undecoded. |
| `player_use_item` | 115 | `0 × 2` | `item_stock[0][object->object_id] = 0;` | Clear all player-owned copies in stock bank zero when the matching chalice/seal-stone object triggers its link. |
| `player_use_item` | 126 | `1 × 2` | `for (slot = KF_EFFECT_CAPACITY - 1; slot != -1; slot--, record++) {` | Inclusive countdown from the last of 48 records through slot 0, stopping at -1. Each step advances the record pointer once. |
| `player_use_item` | 134 | `2` | `if (player_state.progress_state.current_floor == 2) {` | One-based floor 2 chooses the first four deformation segments. |
| `player_use_item` | 135 | `0, 4, 0x96, 0x320, 0x2b, 0x46` | `effect_pool_spawn_typed(0, 4, 0x96, 0x320, 0x2b, 0x46);` | Segment start 0/count 4; temporal Q12 progress 150 per update; cell stagger 800;43 updates per sweep; hold counter 70 yields 71 hold-state calls. These are protocol values, not object coordinates; original timing choices unproven. |
| `player_use_item` | 136 | `3` | `} else if (player_state.progress_state.current_floor == 3) {` | One-based floor 3 chooses the last deformation segment; other floors do not spawn this effect. |
| `player_use_item` | 137 | `4, 1, 0x96, 0x320, 0x58, 0x10e` | `effect_pool_spawn_typed(4, 1, 0x96, 0x320, 0x58, 0x10e);` | Segment start 4/count 1; temporal Q12 progress 150 per update; cell stagger 800;88 updates per sweep; hold counter 270 yields 271 hold-state calls. Original timing choices unproven. |
| `player_use_item` | 155 | `1000` | `player_state.illusion_staff_timer = 1000;` | Initial signed counter 1000 causes 1001 qualifying lighting calls through zero before becoming inactive at -1. New use resets it; original duration unproven. |
| `player_use_item` | 156 | `0 × 2` | `if (item_stock[0][KF_ITEM_ILLUSION_STAFF] != 0) {` | Only decrement a possessed player-bank copy; the timer write already happened before this guard. |
| `player_use_item` | 157 | `0` | `item_stock[0][KF_ITEM_ILLUSION_STAFF]--;` | Consume one copy from the player-owned bank zero. |
| `player_use_item` | 164, 174 | `6000 × 2` | `6000,` | Mirror of Truth target-query distance in world units: three map tiles. Actor search precedes event fallback; original reach choice unproven. |
| `player_use_item` | 165, 175 | `12 × 2` | `KF_ANGLE_FULL_TURN / 12,` | Target-cone angular tolerance 341/4096 revolution, the truncated one-twelfth turn; shared representation with the door-facing tolerance, without assuming their tuning rationale. |
| `player_use_item` | 167 | `0` | `if (actor != 0) {` | A non-null actor wins the mirror query and its info image is shown immediately. |
| `player_use_item` | 177 | `0` | `if (event == 0) {` | No event target after the actor miss leaves the mirror use unacknowledged. |
| `player_use_item` | 183 | `100` | `player_state.magic_training += 100;` | Add an entire 100-count training threshold before the normal helper increments once and resets the counter; see the consumable audit for caps and halfword behavior. |
