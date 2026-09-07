# Retained map-event literals

Complete account of **129 numeric occurrences** in `map_event.c`,
`map_events.c` and `map_load.c` after the
[motion and sound follow-up](game-map-event-motion-constants.md).
Comments, strings, identifier digits, enums and retail claims are excluded.
Each repeated token has its own row; signs remain expression operators.

| Source / function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `map_event.c` / `map_event_refresh_dialogue_stage` | 26 | `0` | `event->dialogue_page_delay = 0;` | No pending page advance; ordinary zero countdown, reset on initialization or stage change. |
| `map_event.c` / `map_event_advance_animation_blocking` | 40 | `0` | `render_frame(0, 0);` | Null position/rotation overrides select the live player camera. |
| `map_event.c` / `map_event_advance_animation_blocking` | 40 | `0` | `render_frame(0, 0);` | Null position/rotation overrides select the live player camera. |
| `map_event.c` / `map_event_advance_animation_blocking` | 44 | `0` | `render_frame(0, 0);` | Null position/rotation overrides select the live player camera. |
| `map_event.c` / `map_event_advance_animation_blocking` | 44 | `0` | `render_frame(0, 0);` | Null position/rotation overrides select the live player camera. |
| `map_event.c` / `map_event_pool_load` | 50 | `0` | `u8 exhausted = 0;` | Definition-list exhaustion starts false. |
| `map_event.c` / `map_event_pool_load` | 52 | `1` | `u16 count = KF_MAP_EVENT_CAPACITY - 1;` | Inclusive countdown starts at capacity minus one; eight slots are visited. |
| `map_event.c` / `map_event_pool_load` | 55 | `1` | `if (exhausted == 1) {` | Boolean list-exhausted state after the free/end marker. |
| `map_event.c` / `map_event_pool_load` | 79 | `0` | `event->rotation_z = 0;` | Angular origin initializes the indicated rotation lane; no separate state encoding. |
| `map_event.c` / `map_event_pool_load` | 80 | `0` | `event->rotation_x = 0;` | Angular origin initializes the indicated rotation lane; no separate state encoding. |
| `map_event.c` / `map_event_pool_load` | 83 | `0` | `event->dialogue_page_delay = 0;` | No pending page advance; ordinary zero countdown, reset on initialization or stage change. |
| `map_event.c` / `map_event_pool_load` | 84 | `0` | `event->animation_clip = 0;` | First resource animation clip; clip indices are resource IDs, not actor-action enum values. |
| `map_event.c` / `map_event_pool_load` | 85 | `0` | `event->animation_phase = 0;` | Start at the beginning of that animation clip. |
| `map_event.c` / `map_event_pool_load` | 86 | `0` | `event->rotation_target = 0;` | Initial yaw target at the angular origin. |
| `map_event.c` / `map_event_pool_load` | 87 | `0` | `event->collision_turn_pending = 0;` | Clear the boolean indicating a collision-selected heading. |
| `map_event.c` / `map_event_pool_load` | 88 | `1` | `collision_adjust_cell_occupancy(event->cell_x, event->cell_z, 1);` | Add one event to the destination cell count. |
| `map_event.c` / `map_event_pool_load` | 90 | `1` | `exhausted = 1;` | Set the local list-exhausted boolean after the free/end marker. |
| `map_event.c` / `map_event_pool_load` | 95 | `0` | `} while (count-- != 0);` | Post-decrement zero endpoint visits the final slot before ending the fixed-size walk. |
| `map_event.c` / `map_event_distance_to_point` | 118 | `1` | `return -1;` | Conventional negative query-miss result, distinct from valid nonnegative distance or pool index. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 129 | `0` | `KfMapEvent *found = 0;` | Null result until an eligible event is found. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 131 | `0` | `s32 found_distance = 0;` | Distance output remains zero when no event is found; the pointer result distinguishes failure. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 133 | `1` | `u16 count = KF_MAP_EVENT_CAPACITY - 1;` | Inclusive countdown starts at capacity minus one; eight slots are visited. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 143 | `1` | `if (distance == -1) {` | Recognize the distance helper's query-miss result. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 150 | `1` | `if (angle >= KF_ANGLE_HALF_TURN + 1) {` | Exclusive half-turn boundary; the unit adjustment preserves the exact predicate. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 161 | `0` | `} while (event++, count-- != 0);` | Post-decrement zero endpoint visits the final slot before ending the fixed-size walk. |
| `map_event.c` / `map_event_pool_find_overlap` | 170 | `0` | `s16 index = 0;` | Start the pool walk at its first zero-based index. |
| `map_event.c` / `map_event_pool_find_overlap` | 175 | `1` | `event, point_x, point_z, event->radius + radius_padding) != -1) {` | A nonnegative distance means this active event overlaps the requested radius. |
| `map_event.c` / `map_event_pool_find_overlap` | 181 | `1` | `return -1;` | Conventional negative query-miss result, distinct from valid nonnegative distance or pool index. |
| `map_events.c` / `map_event_update_wander` | 56 | `1` | `collision_adjust_cell_occupancy(event->cell_x, event->cell_z, -1);` | Remove this event from its previous cell count before testing its movement. |
| `map_events.c` / `map_event_update_wander` | 67 | `0` | `point.vx, KF_COLLISION_IGNORE_HEIGHT, point.vz, event->radius, 0,` | Zero query height is passed with the height-ignore sentinel; preserve the observed collision argument. |
| `map_events.c` / `map_event_update_wander` | 68 | `0x80` | `KF_COLLISION_SKIP_MAP_EVENTS \| (0x80 << KF_COLLISION_CELL_FLAG_SHIFT))` | Select collision-cell flag bit 7 for rejection; the bit's authored map meaning is unresolved. |
| `map_events.c` / `map_event_update_wander` | 74 | `0` | `event->collision_turn_pending = 0;` | Clear the boolean indicating a collision-selected heading. |
| `map_events.c` / `map_event_update_wander` | 79 | `0` | `if (event->collision_turn_pending == 0 \|\| event->rotation == event->rotation_target) {` | Boolean clear test permits a new collision heading; the other branch detects completion of the prior turn. |
| `map_events.c` / `map_event_update_wander` | 81 | `1` | `event->collision_turn_pending = 1;` | Set that boolean to avoid repeatedly selecting a new heading during the same turn. |
| `map_events.c` / `map_event_update_wander` | 88 | `1` | `collision_adjust_cell_occupancy(event->cell_x, event->cell_z, 1);` | Add one event to the destination cell count. |
| `map_events.c` / `map_event_update_animation_loop` | 100 | `5` | `if (player_state.progress_state.current_floor == 5` | Authored floor selector for the animation-wrap sound. |
| `map_events.c` / `map_event_update_animation_loop` | 101 | `0` | `&& event == &map_event_pool[0]` | First event slot owns this floor-specific sound. |
| `map_events.c` / `map_event_update_animation_loop` | 102 | `0` | `&& map_event_pool[0].animation_phase < KF_MAP_EVENT_ANIMATION_LOOP_STEP) {` | Test that same first slot for animation wrap. |
| `map_events.c` / `map_event_update_animation_loop` | 104 | `0` | `(const VECTOR *)&map_event_pool[0].reference_x,` | Use that first event slot's world position for sound playback. |
| `map_events.c` / `map_event_pool_update` | 113 | `1` | `u16 index = KF_MAP_EVENT_CAPACITY - 1;` | Inclusive countdown starts at capacity minus one; eight slots are visited. |
| `map_events.c` / `map_event_pool_update` | 134 | `0` | `if (map_dialogue_advance_gate == 0 && event->dialogue_page_delay != 0) {` | Zero gate is the dialogue tick; nonzero delay means an advance is pending. |
| `map_events.c` / `map_event_pool_update` | 134 | `0` | `if (map_dialogue_advance_gate == 0 && event->dialogue_page_delay != 0) {` | Zero gate is the dialogue tick; nonzero delay means an advance is pending. |
| `map_events.c` / `map_event_pool_update` | 136 | `0` | `if (event->dialogue_page_delay == 0) {` | Countdown expired: advance and cap the page now. |
| `map_events.c` / `map_event_pool_update` | 137 | `1` | `s32 limit = event->dialogue_pages.last_page[event->dialogue_stage - 1];` | Convert a one-based dialogue stage to its zero-based last-page array index. |
| `map_events.c` / `map_event_pool_update` | 147 | `0` | `} while (index-- != 0);` | Post-decrement zero endpoint visits the final slot before ending the fixed-size walk. |
| `map_events.c` / `map_event_pool_update` | 153 | `1` | `*gate = current - 1;` | Decrement by one update; wrap/reload follows the old value's zero test. |
| `map_events.c` / `map_event_pool_update` | 154 | `0` | `if (current == 0) {` | Zero is the countdown endpoint that reloads the gate. |
| `map_events.c` / `map_event_pool_update` | 159 | `0` | `if (map_ambient_script_countdown-- == 0) {` | Post-decrement zero endpoint gives eleven updates per reload of ten. |
| `map_events.c` / `map_event_pool_update` | 162 | `1` | `case 1:` | Dungeon floor number directly selects that floor's script. |
| `map_events.c` / `map_event_pool_update` | 165 | `2` | `case 2:` | Dungeon floor number directly selects that floor's script. |
| `map_events.c` / `map_event_pool_update` | 168 | `3` | `case 3:` | Dungeon floor number directly selects that floor's script. |
| `map_events.c` / `map_event_pool_update` | 171 | `4` | `case 4:` | Dungeon floor number directly selects that floor's script. |
| `map_events.c` / `map_event_pool_update` | 174 | `5` | `case 5:` | Dungeon floor number directly selects that floor's script. |
| `map_events.c` / `map_world_state_persist` | 196 | `1` | `*out++ = 1;` | Serialized boolean records-present marker, read by the inverse parser. |
| `map_events.c` / `map_world_state_persist` | 199 | `0` | `for (i = 0; i < KF_MAP_EVENT_CAPACITY; i++, event++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_events.c` / `map_world_state_persist` | 204 | `1` | `*out++ = event->dialogue_pages.last_page[event->dialogue_stage - 1];` | Convert a one-based dialogue stage to its zero-based last-page array index. |
| `map_events.c` / `map_world_state_persist` | 210 | `0` | `active = 0;` | Start the sparse record count at zero before backfilling its count byte. |
| `map_events.c` / `map_world_state_persist` | 211 | `0` | `actor = &actor_state.actors[0];` | Begin at the first record of the owning array. |
| `map_events.c` / `map_world_state_persist` | 212 | `0` | `for (i = 0; i < KF_ACTOR_CAPACITY; i++, actor++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_events.c` / `map_world_state_persist` | 225 | `0` | `object = &map_object_state.objects[0];` | Begin at the first record of the owning array. |
| `map_events.c` / `map_world_state_persist` | 226 | `0` | `for (i = 0; i < KF_MAP_OBJECT_CAPACITY; i++, object++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_events.c` / `map_world_state_persist` | 231 | `0` | `active = 0;` | Start the sparse record count at zero before backfilling its count byte. |
| `map_events.c` / `map_world_state_persist` | 232 | `0` | `object = &map_object_state.objects[0];` | Begin at the first record of the owning array. |
| `map_events.c` / `map_world_state_persist` | 234 | `0` | `for (i = 0; i < KF_MAP_OBJECT_EFFECT_FIRST; i++, object++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_events.c` / `map_world_state_persist` | 256 | `1` | `s32 k = sizeof(object->link) - 1;` | Initialize the signed byte-copy countdown to the complete member extent minus one. |
| `map_events.c` / `map_world_state_persist` | 260 | `1` | `} while (--k != -1);` | Negative countdown endpoint preserves zero-count skipping and the complete byte/record traversal. |
| `map_events.c` / `map_world_state_persist` | 266 | `0` | `for (i = 0; i < KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_events.c` / `map_world_state_persist` | 270 | `8` | `*out++ = (u8)(*(const u16 *)&object->link >> 8);` | Extract the high byte of the little-endian two-byte link prefix. |
| `map_events.c` / `map_world_state_persist` | 274 | `0` | `for (i = 0; i < 2 * KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {` | Zero-based record traversal; multiplier 2 spans the adjacent definition-drop and placement-drop groups. |
| `map_events.c` / `map_world_state_persist` | 274 | `2` | `for (i = 0; i < 2 * KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {` | Zero-based record traversal; multiplier 2 spans the adjacent definition-drop and placement-drop groups. |
| `map_load.c` / `map_restore_floor_state` | 47 | `1` | `if (*in++ == 1) {` | Serialized records-present marker; only this exact byte value enables restoration. |
| `map_load.c` / `map_restore_floor_state` | 49 | `0` | `for (i = 0; i < KF_MAP_EVENT_CAPACITY; i++, event++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_load.c` / `map_restore_floor_state` | 54 | `1` | `event->dialogue_pages.last_page[event->dialogue_stage - 1] = *in++;` | Convert a one-based dialogue stage to its zero-based last-page array index. |
| `map_load.c` / `map_restore_floor_state` | 60 | `1` | `if (--i != -1) {` | Negative countdown endpoint preserves zero-count skipping and the complete byte/record traversal. |
| `map_load.c` / `map_restore_floor_state` | 66 | `1` | `} while (--i != -1);` | Negative countdown endpoint preserves zero-count skipping and the complete byte/record traversal. |
| `map_load.c` / `map_restore_floor_state` | 69 | `0` | `object = &map_object_state.objects[0];` | Begin at the first record of the owning array. |
| `map_load.c` / `map_restore_floor_state` | 70 | `0` | `for (i = 0; i < KF_MAP_OBJECT_CAPACITY; i++, object++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_load.c` / `map_restore_floor_state` | 75 | `1` | `while (--i != -1) {` | Negative countdown endpoint preserves zero-count skipping and the complete byte/record traversal. |
| `map_load.c` / `map_restore_floor_state` | 82 | `1` | `k = sizeof(object->link) - 1;` | Initialize the signed byte-copy countdown to the complete member extent minus one. |
| `map_load.c` / `map_restore_floor_state` | 85 | `1` | `} while (--k != -1);` | Negative countdown endpoint preserves zero-count skipping and the complete byte/record traversal. |
| `map_load.c` / `map_restore_floor_state` | 89 | `0` | `for (i = 0; i < KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_load.c` / `map_restore_floor_state` | 98 | `0` | `object->rotation.z = 0;` | Angular origin initializes the indicated rotation lane; no separate state encoding. |
| `map_load.c` / `map_restore_floor_state` | 99 | `0` | `object->rotation.y = 0;` | Angular origin initializes the indicated rotation lane; no separate state encoding. |
| `map_load.c` / `map_restore_floor_state` | 100 | `0` | `object->rotation.x = 0;` | Angular origin initializes the indicated rotation lane; no separate state encoding. |
| `map_load.c` / `map_restore_floor_state` | 102 | `8` | `*(u16 *)&object->link \|= *in++ << 8;` | Restore that high byte while preserving the previously read low byte. |
| `map_load.c` / `map_restore_floor_state` | 103 | `0` | `object->link.spawn_sequence = 0;` | Initialize the transient drop sequence to zero. |
| `map_load.c` / `map_restore_floor_state` | 104 | `0` | `object->link.vertical_velocity = 0;` | Start the transient drop with zero vertical velocity. |
| `map_load.c` / `map_restore_floor_state` | 108 | `0` | `for (i = 0; i < 2 * KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {` | Zero-based record traversal; multiplier 2 spans the adjacent definition-drop and placement-drop groups. |
| `map_load.c` / `map_restore_floor_state` | 108 | `2` | `for (i = 0; i < 2 * KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {` | Zero-based record traversal; multiplier 2 spans the adjacent definition-drop and placement-drop groups. |
| `map_load.c` / `map_restore_floor_state` | 120 | `0` | `object->rotation.x = 0;` | Angular origin initializes the indicated rotation lane; no separate state encoding. |
| `map_load.c` / `map_restore_floor_state` | 122 | `0` | `object->rotation.z = 0;` | Angular origin initializes the indicated rotation lane; no separate state encoding. |
| `map_load.c` / `map_restore_floor_state` | 124 | `0` | `*(u16 *)&object->link = 0;` | Initialize both bytes of the transient drop's link prefix to zero, not the absent-link encoding 255. |
| `map_load.c` / `map_restore_floor_state` | 125 | `0` | `object->link.spawn_sequence = 0;` | Initialize the transient drop sequence to zero. |
| `map_load.c` / `map_restore_floor_state` | 126 | `0` | `object->link.vertical_velocity = 0;` | Start the transient drop with zero vertical velocity. |
| `map_load.c` / `map_restore_floor_state` | 131 | `1` | `case 1:` | Dungeon floor number directly selects that floor's script. |
| `map_load.c` / `map_restore_floor_state` | 133 | `1` | `map_apply_copy_region(1);` | Authored copy-region index for that floor-1 passage. |
| `map_load.c` / `map_restore_floor_state` | 136 | `7` | `index = actor_pool_find_at_tile(7, 0x28);` | Authored tile X=7, Z=40 locates the actor controlled by that floor-1 script. |
| `map_load.c` / `map_restore_floor_state` | 136 | `0x28` | `index = actor_pool_find_at_tile(7, 0x28);` | Authored tile X=7, Z=40 locates the actor controlled by that floor-1 script. |
| `map_load.c` / `map_restore_floor_state` | 137 | `1` | `if (index != -1) {` | Do not index the actor pool when the tile query returns its negative miss sentinel. |
| `map_load.c` / `map_restore_floor_state` | 142 | `0x33` | `map_object_pool_clear_link(0x33);` | Authored link ID removed by that persistent transition; no proper object identity is inferred. |
| `map_load.c` / `map_restore_floor_state` | 145 | `2` | `case 2:` | Dungeon floor number directly selects that floor's script. |
| `map_load.c` / `map_restore_floor_state` | 147 | `0x33` | `map_object_pool_clear_link(0x33);` | Authored link ID removed by that persistent transition; no proper object identity is inferred. |
| `map_load.c` / `map_restore_floor_state` | 149 | `3` | `if (player_state.progress_state.highest_floor >= 3) {` | Visiting floor 3 retires the first floor-2 event. |
| `map_load.c` / `map_restore_floor_state` | 150 | `0` | `map_event_pool[0].state = KF_MAP_EVENT_DISABLED;` | First floor-2 event slot selected by that progression rule. |
| `map_load.c` / `map_restore_floor_state` | 153 | `3` | `case 3:` | Dungeon floor number directly selects that floor's script. |
| `map_load.c` / `map_restore_floor_state` | 155 | `0x33` | `map_object_pool_clear_link(0x33);` | Authored link ID removed by that persistent transition; no proper object identity is inferred. |
| `map_load.c` / `map_restore_floor_state` | 158 | `2` | `map_apply_copy_region(2);` | First authored region index enabled by completion of four floor-3 map-piece reveals. |
| `map_load.c` / `map_restore_floor_state` | 159 | `3` | `map_apply_copy_region(3);` | Second authored region index enabled by completion of four floor-3 map-piece reveals. |
| `map_load.c` / `map_restore_floor_state` | 162 | `4` | `case 4:` | Dungeon floor number directly selects that floor's script. |
| `map_load.c` / `map_restore_floor_state` | 164 | `5` | `case 5:` | Dungeon floor number directly selects that floor's script. |
| `map_load.c` / `map_restore_floor_state` | 166 | `1` | `map_event_pool[1].state = KF_MAP_EVENT_ACTIVE;` | Second floor-5 slot is the transfer destination (character ID 11). |
| `map_load.c` / `map_restore_floor_state` | 169 | `7` | `actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_MELEE] = KF_ACTOR_ANIMATION_NONE;` | Authored actor definition 7; slot names carry the decoded action role, and the inverse reveal script writes the same definition. |
| `map_load.c` / `map_restore_floor_state` | 170 | `7` | `actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT0] = KF_ACTOR_ANIMATION_NONE;` | Authored actor definition 7; slot names carry the decoded action role, and the inverse reveal script writes the same definition. |
| `map_load.c` / `map_restore_floor_state` | 171 | `7` | `actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT1] = KF_ACTOR_ANIMATION_NONE;` | Authored actor definition 7; slot names carry the decoded action role, and the inverse reveal script writes the same definition. |
| `map_load.c` / `map_restore_floor_state` | 172 | `7` | `actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT2] = KF_ACTOR_ANIMATION_NONE;` | Authored actor definition 7; slot names carry the decoded action role, and the inverse reveal script writes the same definition. |
| `map_load.c` / `map_restore_floor_state` | 173 | `7` | `actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_MULTI_HIT_ATTACK] = KF_ACTOR_ANIMATION_NONE;` | Authored actor definition 7; slot names carry the decoded action role, and the inverse reveal script writes the same definition. |
| `map_load.c` / `map_restore_floor_state` | 175 | `4` | `map_apply_copy_region(4);` | Authored floor-5 map-copy region applied after the reveal flag is set. |
| `map_load.c` / `map_restore_floor_state` | 177 | `0` | `if (item_stock[0][KF_ITEM_DRAGON_SWORD] != 0 \|\| item_stock[0][KF_ITEM_MOONLIGHT_SWORD] != 0` | Stock row zero and nonzero possession tests for the named Dragon Sword and Moonlight Sword; the script flag supplies the separate alternative. |
| `map_load.c` / `map_restore_floor_state` | 177 | `0` | `if (item_stock[0][KF_ITEM_DRAGON_SWORD] != 0 \|\| item_stock[0][KF_ITEM_MOONLIGHT_SWORD] != 0` | Stock row zero and nonzero possession tests for the named Dragon Sword and Moonlight Sword; the script flag supplies the separate alternative. |
| `map_load.c` / `map_restore_floor_state` | 177 | `0` | `if (item_stock[0][KF_ITEM_DRAGON_SWORD] != 0 \|\| item_stock[0][KF_ITEM_MOONLIGHT_SWORD] != 0` | Stock row zero and nonzero possession tests for the named Dragon Sword and Moonlight Sword; the script flag supplies the separate alternative. |
| `map_load.c` / `map_restore_floor_state` | 177 | `0` | `if (item_stock[0][KF_ITEM_DRAGON_SWORD] != 0 \|\| item_stock[0][KF_ITEM_MOONLIGHT_SWORD] != 0` | Stock row zero and nonzero possession tests for the named Dragon Sword and Moonlight Sword; the script flag supplies the separate alternative. |
| `map_load.c` / `map_restore_floor_state` | 179 | `0x34` | `map_object_pool_clear_link(0x34);` | Authored link ID cleared by either item possession or the transition flag. |
| `map_load.c` / `map_restore_floor_state` | 182 | `0xd` | `map_object_pool_trigger_link(0xd);` | Authored link ID triggered after completion. |
| `map_load.c` / `map_restore_floor_state` | 183 | `0` | `actor_pool_begin_death_by_definition(0);` | Authored actor-definition selector in the completion cleanup set {0,2,3,4}; no proper creature identity is inferred. |
| `map_load.c` / `map_restore_floor_state` | 184 | `2` | `actor_pool_begin_death_by_definition(2);` | Authored actor-definition selector in the completion cleanup set {0,2,3,4}; no proper creature identity is inferred. |
| `map_load.c` / `map_restore_floor_state` | 185 | `3` | `actor_pool_begin_death_by_definition(3);` | Authored actor-definition selector in the completion cleanup set {0,2,3,4}; no proper creature identity is inferred. |
| `map_load.c` / `map_restore_floor_state` | 186 | `4` | `actor_pool_begin_death_by_definition(4);` | Authored actor-definition selector in the completion cleanup set {0,2,3,4}; no proper creature identity is inferred. |
| `map_load.c` / `map_refresh_dialogue_stages` | 198 | `1` | `u16 index = KF_MAP_EVENT_CAPACITY - 1;` | Inclusive countdown starts at capacity minus one; eight slots are visited. |
| `map_load.c` / `map_refresh_dialogue_stages` | 205 | `0` | `} while (index-- != 0);` | Post-decrement zero endpoint visits the final slot before ending the fixed-size walk. |
