# Retained map-event literals

Reviewed ledger for `map_event.c`, `map_events.c` and `map_load.c` after the
[map-event literal campaign](game-map-event-literals.md). Source line numbers
locate this reviewed version; the function and expression identify the use.
Repeated occurrences on different lines are grouped only when their complete
expression and reason agree. Minus signs are C operators, so `-1` has one
numeric token, recorded as `1` below. Ownership claims and named constant
definitions are accounted for separately in the campaign note.

All **157 retained occurrences** are mapped below; no fallback category is used.

| Source / function | Lines | Tokens | Expression | Reason |
| --- | --- | --- | --- | --- |
| `map_event.c` / `map_event_refresh_dialogue_stage` | 22 | `0` | `event->dialogue_page_delay = 0;` | No pending page advance; ordinary zero countdown, reset on initialization or stage change. |
| `map_event.c` / `map_event_advance_animation_blocking` | 36, 40 | `0 × 4` | `render_frame(0, 0);` | Null position/rotation overrides select the live player camera. |
| `map_event.c` / `map_event_pool_load` | 46 | `0` | `u8 exhausted = 0;` | Definition-list exhaustion starts false. |
| `map_event.c` / `map_event_pool_load` | 48 | `1` | `u16 count = KF_MAP_EVENT_CAPACITY - 1;` | Inclusive countdown starts at capacity minus one; eight slots are visited. |
| `map_event.c` / `map_event_pool_load` | 51 | `1` | `if (exhausted == 1) {` | Boolean list-exhausted state after the free/end marker. |
| `map_event.c` / `map_event_pool_load` | 75 | `0` | `event->rotation_z = 0;` | Angular origin initializes the indicated rotation lane; no separate state encoding. |
| `map_event.c` / `map_event_pool_load` | 76 | `0` | `event->rotation_x = 0;` | Angular origin initializes the indicated rotation lane; no separate state encoding. |
| `map_event.c` / `map_event_pool_load` | 79 | `0` | `event->dialogue_page_delay = 0;` | No pending page advance; ordinary zero countdown, reset on initialization or stage change. |
| `map_event.c` / `map_event_pool_load` | 80 | `0` | `event->animation_clip = 0;` | First resource animation clip; clip indices are resource IDs, not actor-action enum values. |
| `map_event.c` / `map_event_pool_load` | 81 | `0` | `event->animation_phase = 0;` | Start at the beginning of that animation clip. |
| `map_event.c` / `map_event_pool_load` | 82 | `0` | `event->rotation_target = 0;` | Initial yaw target at the angular origin. |
| `map_event.c` / `map_event_pool_load` | 83 | `0` | `event->collision_turn_pending = 0;` | Clear the boolean indicating a collision-selected heading. |
| `map_event.c` / `map_event_pool_load` | 84 | `1` | `collision_adjust_cell_occupancy(event->cell_x, event->cell_z, 1);` | Add one event to the destination cell count. |
| `map_event.c` / `map_event_pool_load` | 86 | `1` | `exhausted = 1;` | Set the local list-exhausted boolean after the free/end marker. |
| `map_event.c` / `map_event_pool_load` | 91 | `0` | `} while (count-- != 0);` | Post-decrement zero endpoint visits the final slot before ending the fixed-size walk. |
| `map_event.c` / `map_event_distance_to_point` | 114 | `1` | `return -1;` | Conventional negative query-miss result, distinct from valid nonnegative distance or pool index. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 125 | `0` | `KfMapEvent *found = 0;` | Null result until an eligible event is found. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 126 | `30000` | `s16 best_angle = 30000;` | Initial sentinel above every folded angle (at most 2048), within s16; choice of 30000 is otherwise unproven. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 127 | `0` | `s32 found_distance = 0;` | Distance output remains zero when no event is found; the pointer result distinguishes failure. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 129 | `1` | `u16 count = KF_MAP_EVENT_CAPACITY - 1;` | Inclusive countdown starts at capacity minus one; eight slots are visited. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 139 | `1` | `if (distance == -1) {` | Recognize the distance helper's query-miss result. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 146 | `1` | `if (angle >= KF_ANGLE_HALF_TURN + 1) {` | Exclusive half-turn boundary; the unit adjustment preserves the exact predicate. |
| `map_event.c` / `map_event_pool_find_target_in_cone` | 157 | `0` | `} while (event++, count-- != 0);` | Post-decrement zero endpoint visits the final slot before ending the fixed-size walk. |
| `map_event.c` / `map_event_pool_find_overlap` | 166 | `0` | `s16 index = 0;` | Start the pool walk at its first zero-based index. |
| `map_event.c` / `map_event_pool_find_overlap` | 171 | `1` | `event, point_x, point_z, event->radius + radius_padding) != -1) {` | A nonnegative distance means this active event overlaps the requested radius. |
| `map_event.c` / `map_event_pool_find_overlap` | 177 | `1` | `return -1;` | Conventional negative query-miss result, distinct from valid nonnegative distance or pool index. |
| `map_events.c` / `map_event_update_wander` | 50 | `1` | `collision_adjust_cell_occupancy(event->cell_x, event->cell_z, -1);` | Remove this event from its previous cell count before testing its movement. |
| `map_events.c` / `map_event_update_wander` | 52 | `70` | `heading = angle_approach(event->rotation, event->rotation_target, 70);` | Observed turn cap: 70/4096 turn (about 6.15 degrees) per update; original tuning rationale unknown. |
| `map_events.c` / `map_event_update_wander` | 55 | `20` | `vector2s_scale_shift11(20, &forward);` | Q12 direction scaled by 20 then shifted by 11 yields nominal 40 world units; original tuning rationale unknown. |
| `map_events.c` / `map_event_update_wander` | 61 | `0` | `point.vx, KF_COLLISION_IGNORE_HEIGHT, point.vz, event->radius, 0,` | Zero query height is passed with the height-ignore sentinel; preserve the observed collision argument. |
| `map_events.c` / `map_event_update_wander` | 62 | `0x80` | `KF_COLLISION_SKIP_MAP_EVENTS \| (0x80 << KF_COLLISION_CELL_FLAG_SHIFT))` | Select collision-cell flag bit 7 for rejection; the bit's authored map meaning is unresolved. |
| `map_events.c` / `map_event_update_wander` | 68 | `0` | `event->collision_turn_pending = 0;` | Clear the boolean indicating a collision-selected heading. |
| `map_events.c` / `map_event_update_wander` | 69 | `1584` | `if (event->rotation == event->rotation_target && rand() < 1584) {` | Accept 1584 of the 32768 SDK rand outputs after a clear move reaches its heading; no original probability rationale is proven. |
| `map_events.c` / `map_event_update_wander` | 70, 74 | `3 × 2` | `event->rotation_target = rand() >> 3;` | Reduce the SDK's 15-bit random range to the 12-bit angle domain. |
| `map_events.c` / `map_event_update_wander` | 73 | `0` | `if (event->collision_turn_pending == 0 \|\| event->rotation == event->rotation_target) {` | Boolean clear test permits a new collision heading; the other branch detects completion of the prior turn. |
| `map_events.c` / `map_event_update_wander` | 75 | `1` | `event->collision_turn_pending = 1;` | Set that boolean to avoid repeatedly selecting a new heading during the same turn. |
| `map_events.c` / `map_event_update_wander` | 82 | `1` | `collision_adjust_cell_occupancy(event->cell_x, event->cell_z, 1);` | Add one event to the destination cell count. |
| `map_events.c` / `map_event_update_animation_loop` | 94 | `5` | `if (player_state.progress_state.current_floor == 5` | Authored floor selector for the animation-wrap sound. |
| `map_events.c` / `map_event_update_animation_loop` | 95 | `0` | `&& event == &map_event_pool[0]` | First event slot owns this floor-specific sound. |
| `map_events.c` / `map_event_update_animation_loop` | 96 | `0` | `&& map_event_pool[0].animation_phase < KF_MAP_EVENT_ANIMATION_LOOP_STEP) {` | Test that same first slot for animation wrap. |
| `map_events.c` / `map_event_update_animation_loop` | 98 | `0` | `(const VECTOR *)&map_event_pool[0].reference_x,` | Use that first event slot's world position for sound playback. |
| `map_events.c` / `map_event_update_animation_loop` | 99 | `18000, 50000` | `KF_AUDIO_MAX_VOLUME, 18000, 50000);` | Exclusive audible cutoff 18000 and attenuation scale 50000 world units; distinct roles proven by audio callee, original tuning rationale unknown. |
| `map_events.c` / `map_event_pool_update` | 107 | `1` | `u16 index = KF_MAP_EVENT_CAPACITY - 1;` | Inclusive countdown starts at capacity minus one; eight slots are visited. |
| `map_events.c` / `map_event_pool_update` | 128 | `0 × 2` | `if (map_dialogue_advance_gate == 0 && event->dialogue_page_delay != 0) {` | Zero gate is the dialogue tick; nonzero delay means an advance is pending. |
| `map_events.c` / `map_event_pool_update` | 130 | `0` | `if (event->dialogue_page_delay == 0) {` | Countdown expired: advance and cap the page now. |
| `map_events.c` / `map_event_pool_update` | 131 | `1` | `s32 limit = event->dialogue_pages.last_page[event->dialogue_stage - 1];` | Convert a one-based dialogue stage to its zero-based last-page array index. |
| `map_events.c` / `map_event_pool_update` | 141 | `0` | `} while (index-- != 0);` | Post-decrement zero endpoint visits the final slot before ending the fixed-size walk. |
| `map_events.c` / `map_event_pool_update` | 147 | `1` | `*gate = current - 1;` | Decrement by one update; wrap/reload follows the old value's zero test. |
| `map_events.c` / `map_event_pool_update` | 148 | `0` | `if (current == 0) {` | Zero is the countdown endpoint that reloads the gate. |
| `map_events.c` / `map_event_pool_update` | 153 | `0` | `if (map_ambient_script_countdown-- == 0) {` | Post-decrement zero endpoint gives eleven updates per reload of ten. |
| `map_events.c` / `map_event_pool_update` | 156 | `1` | `case 1:` | Dungeon floor number directly selects that floor's script. |
| `map_events.c` / `map_event_pool_update` | 159 | `2` | `case 2:` | Dungeon floor number directly selects that floor's script. |
| `map_events.c` / `map_event_pool_update` | 162 | `3` | `case 3:` | Dungeon floor number directly selects that floor's script. |
| `map_events.c` / `map_event_pool_update` | 165 | `4` | `case 4:` | Dungeon floor number directly selects that floor's script. |
| `map_events.c` / `map_event_pool_update` | 168 | `5` | `case 5:` | Dungeon floor number directly selects that floor's script. |
| `map_events.c` / `map_world_state_persist` | 190 | `1` | `*out++ = 1;` | Serialized boolean records-present marker, read by the inverse parser. |
| `map_events.c` / `map_world_state_persist` | 193 | `0` | `for (i = 0; i < KF_MAP_EVENT_CAPACITY; i++, event++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_events.c` / `map_world_state_persist` | 198 | `1` | `*out++ = event->dialogue_pages.last_page[event->dialogue_stage - 1];` | Convert a one-based dialogue stage to its zero-based last-page array index. |
| `map_events.c` / `map_world_state_persist` | 204, 225 | `0 × 2` | `active = 0;` | Start the sparse record count at zero before backfilling its count byte. |
| `map_events.c` / `map_world_state_persist` | 205 | `0` | `actor = &actor_state.actors[0];` | Begin at the first record of the owning array. |
| `map_events.c` / `map_world_state_persist` | 206 | `0` | `for (i = 0; i < KF_ACTOR_CAPACITY; i++, actor++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_events.c` / `map_world_state_persist` | 219, 226 | `0 × 2` | `object = &map_object_state.objects[0];` | Begin at the first record of the owning array. |
| `map_events.c` / `map_world_state_persist` | 220 | `0` | `for (i = 0; i < KF_MAP_OBJECT_CAPACITY; i++, object++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_events.c` / `map_world_state_persist` | 228 | `0` | `for (i = 0; i < KF_MAP_OBJECT_EFFECT_FIRST; i++, object++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_events.c` / `map_world_state_persist` | 250 | `1` | `s32 k = sizeof(object->link) - 1;` | Initialize the signed byte-copy countdown to the complete member extent minus one. |
| `map_events.c` / `map_world_state_persist` | 254 | `1` | `} while (--k != -1);` | Negative countdown endpoint preserves zero-count skipping and the complete byte/record traversal. |
| `map_events.c` / `map_world_state_persist` | 260 | `0` | `for (i = 0; i < KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_events.c` / `map_world_state_persist` | 264 | `8` | `*out++ = (u8)(*(const u16 *)&object->link >> 8);` | Extract the high byte of the little-endian two-byte link prefix. |
| `map_events.c` / `map_world_state_persist` | 268 | `0, 2` | `for (i = 0; i < 2 * KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {` | Zero-based record traversal; multiplier 2 spans the adjacent definition-drop and placement-drop groups. |
| `map_load.c` / `map_restore_floor_state` | 43 | `1` | `if (*in++ == 1) {` | Serialized records-present marker; only this exact byte value enables restoration. |
| `map_load.c` / `map_restore_floor_state` | 45 | `0` | `for (i = 0; i < KF_MAP_EVENT_CAPACITY; i++, event++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_load.c` / `map_restore_floor_state` | 50 | `1` | `event->dialogue_pages.last_page[event->dialogue_stage - 1] = *in++;` | Convert a one-based dialogue stage to its zero-based last-page array index. |
| `map_load.c` / `map_restore_floor_state` | 56 | `1` | `if (--i != -1) {` | Negative countdown endpoint preserves zero-count skipping and the complete byte/record traversal. |
| `map_load.c` / `map_restore_floor_state` | 62 | `1` | `} while (--i != -1);` | Negative countdown endpoint preserves zero-count skipping and the complete byte/record traversal. |
| `map_load.c` / `map_restore_floor_state` | 65 | `0` | `object = &map_object_state.objects[0];` | Begin at the first record of the owning array. |
| `map_load.c` / `map_restore_floor_state` | 66 | `0` | `for (i = 0; i < KF_MAP_OBJECT_CAPACITY; i++, object++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_load.c` / `map_restore_floor_state` | 71 | `1` | `while (--i != -1) {` | Negative countdown endpoint preserves zero-count skipping and the complete byte/record traversal. |
| `map_load.c` / `map_restore_floor_state` | 78 | `1` | `k = sizeof(object->link) - 1;` | Initialize the signed byte-copy countdown to the complete member extent minus one. |
| `map_load.c` / `map_restore_floor_state` | 81 | `1` | `} while (--k != -1);` | Negative countdown endpoint preserves zero-count skipping and the complete byte/record traversal. |
| `map_load.c` / `map_restore_floor_state` | 85 | `0` | `for (i = 0; i < KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {` | Zero-based traversal of the explicitly named pool/group capacity. |
| `map_load.c` / `map_restore_floor_state` | 89, 108 | `15 × 2` | `object->cell_x * KF_MAP_TILE_SIZE + ((rand() * KF_MAP_TILE_SIZE) >> 15);` | Scale the SDK's 15-bit random output into a local X position in the 2000-unit cell. |
| `map_load.c` / `map_restore_floor_state` | 91, 110 | `15 × 2` | `object->cell_z * KF_MAP_TILE_SIZE + ((rand() * KF_MAP_TILE_SIZE) >> 15);` | Scale the SDK's 15-bit random output into a local Z position in the 2000-unit cell. |
| `map_load.c` / `map_restore_floor_state` | 94, 118 | `0 × 2` | `object->rotation.z = 0;` | Angular origin initializes the indicated rotation lane; no separate state encoding. |
| `map_load.c` / `map_restore_floor_state` | 95 | `0` | `object->rotation.y = 0;` | Angular origin initializes the indicated rotation lane; no separate state encoding. |
| `map_load.c` / `map_restore_floor_state` | 96, 116 | `0 × 2` | `object->rotation.x = 0;` | Angular origin initializes the indicated rotation lane; no separate state encoding. |
| `map_load.c` / `map_restore_floor_state` | 98 | `8` | `*(u16 *)&object->link \|= *in++ << 8;` | Restore that high byte while preserving the previously read low byte. |
| `map_load.c` / `map_restore_floor_state` | 99, 121 | `0 × 2` | `object->link.spawn_sequence = 0;` | Initialize the transient drop sequence to zero. |
| `map_load.c` / `map_restore_floor_state` | 100, 122 | `0 × 2` | `object->link.vertical_velocity = 0;` | Start the transient drop with zero vertical velocity. |
| `map_load.c` / `map_restore_floor_state` | 104 | `0, 2` | `for (i = 0; i < 2 * KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; i++, object++) {` | Zero-based record traversal; multiplier 2 spans the adjacent definition-drop and placement-drop groups. |
| `map_load.c` / `map_restore_floor_state` | 113 | `43` | `if (object->object_id < 43) {` | Authored model-ID boundary below which restored drop models receive quarter-turn pitch; narrower item-category meaning is unproven. |
| `map_load.c` / `map_restore_floor_state` | 115 | `48` | `} else if (object->object_id < 48) {` | Authored second model-ID boundary: IDs 43..47 receive zero pitch; higher IDs retain the prior pitch. |
| `map_load.c` / `map_restore_floor_state` | 120 | `0` | `*(u16 *)&object->link = 0;` | Initialize both bytes of the transient drop's link prefix to zero, not the absent-link encoding 255. |
| `map_load.c` / `map_restore_floor_state` | 127 | `1` | `case 1:` | Dungeon floor number directly selects that floor's script. |
| `map_load.c` / `map_restore_floor_state` | 128 | `2, 1` | `if (MAP_WORLD_STATE_BYTES[2] == 1) {` | Floor-1 prefix byte 2 records the passage-copy trigger; one is its set state. |
| `map_load.c` / `map_restore_floor_state` | 129 | `1` | `map_apply_copy_region(1);` | Authored copy-region index for that floor-1 passage. |
| `map_load.c` / `map_restore_floor_state` | 131 | `1, 2` | `if (MAP_WORLD_STATE_BYTES[1] != 2) {` | Floor-1 prefix byte 1 must reach stage 2 before the actor at the fixed tile is retained; exact script stage is preserved. |
| `map_load.c` / `map_restore_floor_state` | 132 | `7, 0x28` | `index = actor_pool_find_at_tile(7, 0x28);` | Authored tile X=7, Z=40 locates the actor controlled by that floor-1 script. |
| `map_load.c` / `map_restore_floor_state` | 133 | `1` | `if (index != -1) {` | Do not index the actor pool when the tile query returns its negative miss sentinel. |
| `map_load.c` / `map_restore_floor_state` | 137, 142, 150 | `1 × 3` | `if (DAT_8009f845 == 1) {` | Set persistent floor-5 transition flag controls the linked object on each of floors 1..3. |
| `map_load.c` / `map_restore_floor_state` | 138, 143, 151 | `0x33 × 3` | `map_object_pool_clear_link(0x33);` | Authored link ID removed by that persistent transition; no proper object identity is inferred. |
| `map_load.c` / `map_restore_floor_state` | 141 | `2` | `case 2:` | Dungeon floor number directly selects that floor's script. |
| `map_load.c` / `map_restore_floor_state` | 145 | `3` | `if (player_state.progress_state.highest_floor >= 3) {` | Visiting floor 3 retires the first floor-2 event. |
| `map_load.c` / `map_restore_floor_state` | 146 | `0` | `map_event_pool[0].state = KF_MAP_EVENT_DISABLED;` | First floor-2 event slot selected by that progression rule. |
| `map_load.c` / `map_restore_floor_state` | 149 | `3` | `case 3:` | Dungeon floor number directly selects that floor's script. |
| `map_load.c` / `map_restore_floor_state` | 153 | `4` | `if (DAT_8009eafc == 4) {` | Exact persisted floor-3 script stage that enables two map-copy regions; narrower story identity remains unsupported. |
| `map_load.c` / `map_restore_floor_state` | 154 | `2` | `map_apply_copy_region(2);` | First authored region index enabled by floor-3 script stage 4. |
| `map_load.c` / `map_restore_floor_state` | 155 | `3` | `map_apply_copy_region(3);` | Second authored region index enabled by floor-3 script stage 4. |
| `map_load.c` / `map_restore_floor_state` | 158 | `4` | `case 4:` | Dungeon floor number directly selects that floor's script. |
| `map_load.c` / `map_restore_floor_state` | 160 | `5` | `case 5:` | Dungeon floor number directly selects that floor's script. |
| `map_load.c` / `map_restore_floor_state` | 161 | `1` | `if (DAT_8009f844 == 1) {` | Set transfer flag enables the floor-5 event corresponding to the disabled floor-2 event. |
| `map_load.c` / `map_restore_floor_state` | 162 | `1` | `map_event_pool[1].state = KF_MAP_EVENT_ACTIVE;` | Second floor-5 slot is the transfer destination (character ID 11). |
| `map_load.c` / `map_restore_floor_state` | 164 | `0` | `if (DAT_8009f846 == 0) {` | Before the floor-5 reveal flag is set, keep the five actor animation slots unavailable. |
| `map_load.c` / `map_restore_floor_state` | 165 | `7` | `actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_MELEE] = KF_ACTOR_ANIMATION_NONE;` | Authored actor definition 7; slot names carry the decoded action role, and the inverse reveal script writes the same definition. |
| `map_load.c` / `map_restore_floor_state` | 166 | `7` | `actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT0] = KF_ACTOR_ANIMATION_NONE;` | Authored actor definition 7; slot names carry the decoded action role, and the inverse reveal script writes the same definition. |
| `map_load.c` / `map_restore_floor_state` | 167 | `7` | `actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT1] = KF_ACTOR_ANIMATION_NONE;` | Authored actor definition 7; slot names carry the decoded action role, and the inverse reveal script writes the same definition. |
| `map_load.c` / `map_restore_floor_state` | 168 | `7` | `actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_EFFECT2] = KF_ACTOR_ANIMATION_NONE;` | Authored actor definition 7; slot names carry the decoded action role, and the inverse reveal script writes the same definition. |
| `map_load.c` / `map_restore_floor_state` | 169 | `7` | `actor_state.definitions[7].action_animations[KF_ACTOR_ANIM_SLOT_MULTI_HIT_ATTACK] = KF_ACTOR_ANIMATION_NONE;` | Authored actor definition 7; slot names carry the decoded action role, and the inverse reveal script writes the same definition. |
| `map_load.c` / `map_restore_floor_state` | 171 | `4` | `map_apply_copy_region(4);` | Authored floor-5 map-copy region applied after the reveal flag is set. |
| `map_load.c` / `map_restore_floor_state` | 173 | `0 × 4, 0xa, 0xb` | `if (item_stock[0][0xa] != 0 \|\| item_stock[0][0xb] != 0` | Stock row 0, item IDs 10/11 and nonzero possession checks; preserve these authored items without assigning unsupported proper names. |
| `map_load.c` / `map_restore_floor_state` | 174 | `1` | `\|\| DAT_8009f845 == 1) {` | The set transition flag also clears the floor-5 linked object. |
| `map_load.c` / `map_restore_floor_state` | 175 | `0x34` | `map_object_pool_clear_link(0x34);` | Authored link ID cleared by either item possession or the transition flag. |
| `map_load.c` / `map_restore_floor_state` | 177 | `0` | `if (boss_defeat_complete != 0) {` | Nonzero persisted completion flag; this is a boolean test. |
| `map_load.c` / `map_restore_floor_state` | 178 | `0xd` | `map_object_pool_trigger_link(0xd);` | Authored link ID triggered after completion. |
| `map_load.c` / `map_restore_floor_state` | 179 | `0` | `actor_pool_begin_death_by_definition(0);` | Authored actor-definition selector in the completion cleanup set {0,2,3,4}; no proper creature identity is inferred. |
| `map_load.c` / `map_restore_floor_state` | 180 | `2` | `actor_pool_begin_death_by_definition(2);` | Authored actor-definition selector in the completion cleanup set {0,2,3,4}; no proper creature identity is inferred. |
| `map_load.c` / `map_restore_floor_state` | 181 | `3` | `actor_pool_begin_death_by_definition(3);` | Authored actor-definition selector in the completion cleanup set {0,2,3,4}; no proper creature identity is inferred. |
| `map_load.c` / `map_restore_floor_state` | 182 | `4` | `actor_pool_begin_death_by_definition(4);` | Authored actor-definition selector in the completion cleanup set {0,2,3,4}; no proper creature identity is inferred. |
| `map_load.c` / `map_refresh_dialogue_stages` | 194 | `1` | `u16 index = KF_MAP_EVENT_CAPACITY - 1;` | Inclusive countdown starts at capacity minus one; eight slots are visited. |
| `map_load.c` / `map_refresh_dialogue_stages` | 201 | `0` | `} while (index-- != 0);` | Post-decrement zero endpoint visits the final slot before ending the fixed-size walk. |
