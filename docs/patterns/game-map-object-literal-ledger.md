# Retained GAME map-object literals

Complete current occurrence ledger for `src/game/map_object.c` and
`src/game/map_object_pool.c`: **145 numeric/character occurrences**.
The [motion review](game-map-object-motion-constants.md) supplies the original
reasons; [model identities](game-object-model-domain.md) and
[behavior/action typing](game-map-object-state-domains.md) account for later
source changes. Every current occurrence, including duplicates and authored
initializers, has its own row. Claims, comments, string contents, identifier
digits and named definitions are excluded.

A retained value with unresolved identity remains naming debt. In particular,
behavior 3 is absent from the retail definition table, effect kind 14 has no
established independent name, and damage-credit class 0x20 remains under review.
The presence of a ledger reason does not declare those domains complete.

## `src/game/map_object.c`

67 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `map_object_pool_find_interaction_from` | 77 | `0` | `offset.vy = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_pool_find_interaction_from` | 85 | `1` | `!= -1) {` | Established negative-one distance-query miss result. |
| `map_object_pool_find_interaction_from` | 90 | `0` | `offset.vy = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_pool_find_interaction_from` | 98 | `1` | `!= -1) {` | Established negative-one distance-query miss result. |
| `map_object_pool_find_interaction_from` | 103 | `1` | `!= -1) {` | Established negative-one distance-query miss result. |
| `map_object_pool_find_interaction_from` | 107 | `1` | `return -1;` | Established negative-one distance-query miss result. |
| `map_object_start_action_if_idle` | 115 | `0` | `object->action_timer = 0;` | Start this selected action at its initial timer. |
| `map_object_effect_pool_acquire` | 123 | `0` | `KfMapObject *oldest = 0;` | Null object pointer initialization or presence check. |
| `map_object_effect_pool_acquire` | 124 | `0` | `s32 oldest_age = 0;` | Initial greatest age; strict improvement preserves the first winner and can leave no winner. |
| `map_object_effect_pool_acquire` | 132 | `0` | `if (age < 0) {` | Negative difference detects halfword sequence wrap. |
| `map_object_effect_pool_acquire` | 133 | `0x10000` | `age += 0x10000;` | Halfword modulus 65536 corrects a negative sequence-age difference. |
| `map_object_effect_pool_acquire` | 140 | `0` | `} while (--count != 0);` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| `map_object_spawn_effect` | 170 | `0` | `object->rotation.z = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_spawn_effect` | 171 | `0` | `object->rotation.x = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_spawn_effect` | 176 | `0` | `object->link.vertical_velocity = 0;` | Initial vertical rest before the selected drop action accelerates. |
| `map_object_spawn_effect` | 179 | `0` | `object->link.vertical_velocity = 0;` | Initial vertical rest before the selected drop action accelerates. |
| `map_object_spawn_effect` | 182 | `0` | `object->link.vertical_velocity = 0;` | Initial vertical rest before the selected drop action accelerates. |
| `map_object_spawn_actor_debris` | 206 | `0` | `object->rotation.z = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_spawn_actor_debris` | 207 | `0` | `object->rotation.x = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_pool_trigger_link` | 222 | `1` | `u16 count = KF_MAP_OBJECT_CAPACITY - 1;` | Inclusive countdown starts at the last slot index. |
| `map_object_pool_trigger_link` | 243 | `0` | `} while (count-- != 0);` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| `map_object_pool_clear_link` | 251 | `1` | `u16 count = KF_MAP_OBJECT_CAPACITY - 1;` | Inclusive countdown starts at the last slot index. |
| `map_object_pool_clear_link` | 261 | `0` | `} while (count-- != 0);` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| `map_object_pool_update` | 286 | `0` | `for (count = KF_MAP_OBJECT_CAPACITY; count != 0; object++, count--) {` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| `map_object_pool_update` | 296 | `0` | `pair = 0;` | Null object pointer initialization or presence check. |
| `map_object_pool_update` | 301 | `0` | `if (pair != 0) {` | Null object pointer initialization or presence check. |
| `map_object_pool_update` | 304 | `0` | `if (timer == 0) {` | First swing-opening update plays the sound. |
| `map_object_pool_update` | 313 | `1` | `if (timer == MAP_SWING_DOOR_OPEN_UPDATES - 1) {` | Last zero-based opening update, when the collision edge opens. |
| `map_object_pool_update` | 322 | `1` | `if (map_object_probe_forward(object, object->rotation.y - KF_ANGLE_QUARTER_TURN) != -1) {` | Established negative-one distance-query miss result. |
| `map_object_pool_update` | 336 | `0` | `if (pair != 0) {` | Null object pointer initialization or presence check. |
| `map_object_pool_update` | 345 | `0` | `if (elapsed == 0) {` | First lift-opening update plays the sound. |
| `map_object_pool_update` | 349 | `1` | `if (elapsed == MAP_LIFT_DOOR_OPEN_UPDATES - 1) {` | Last zero-based opening update, when the collision edge opens. |
| `map_object_pool_update` | 358 | `1` | `if (map_object_probe_forward(object, object->rotation.y) != -1) {` | Established negative-one distance-query miss result. |
| `map_object_pool_update` | 370 | `0` | `if (object->action_timer == 0) {` | Zero timer selects falling before floor contact. |
| `map_object_pool_update` | 380 | `1` | `object->action_timer = 1;` | One selects tipping after floor contact; same halfword now carries angular velocity. |
| `map_object_pool_update` | 400 | `1` | `object->action_timer = 1;` | Completion write before the spin action becomes idle. |
| `map_object_pool_update` | 409 | `0` | `if (object->action_timer == 0) {` | Zero timer selects positive bounce pitch direction. |
| `map_object_pool_update` | 420 | `0` | `object->rotation.x = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_pool_update` | 423 | `1` | `object->link.vertical_velocity = -(object->link.vertical_velocity >> 1);` | Arithmetic division by two before sign reversal at a bounce. |
| `map_object_pool_update` | 424 | `0` | `object->action_timer = object->action_timer == 0;` | Boolean inversion switches bounce pitch direction. |
| `map_object_pool_update` | 431 | `0` | `if (object->action_timer == 0) {` | Zero countdown permits a new range check and emission. |
| `map_object_pool_update` | 433 | `0` | `object->position_x, KF_COLLISION_IGNORE_HEIGHT, object->position_z, MAP_EMITTER_PLAYER_RANGE, 0)` | Zero vertical extent of the height-ignored distance/collision query. |
| `map_object_pool_update` | 434 | `1` | `== -1) {` | Established negative-one distance-query miss result. |
| `map_object_pool_update` | 439 | `0` | `direction.y = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_pool_update` | 444 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| `map_object_pool_update` | 445 | `0xe` | `0xe,` | Effect kind 14 emitted by object 137; independent behavioral identity unresolved. |
| `map_object_pool_update` | 452 | `0` | `direction.y = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_pool_update` | 460 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| `map_object_pool_update` | 468 | `0` | `direction.y = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_pool_update` | 476 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| `map_object_pool_update` | 481 | `1` | `1);` | True requests Wind Cutter sound through the seventh constructor argument. |
| `map_object_pool_update` | 488 | `0` | `direction.y = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_pool_update` | 492 | `0` | `case 0:` | Zero yaw, the angular coordinate origin for this cardinal geometry branch. |
| `map_object_pool_update` | 504 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| `map_object_pool_update` | 519 | `0` | `if (object->link.link_id == KF_MAP_LINK_NONE && object->action_timer == 0) {` | Zero means the one-shot effect-release request has not been sent. |
| `map_object_pool_update` | 521 | `1` | `object->action_timer = 1;` | One records that the release request was sent. |
| `map_object_pool_update` | 529 | `1` | `effect_pool_records[object->link.action_parameter].visual.animation_phase = (KF_FIXED12_ONE - 1);` | Greatest animation phase below Q12 unity; also the reverse departure endpoint. |
| `map_object_pool_update` | 535 | `0` | `if (record->visual.animation_phase == 0) {` | Zero interpolation endpoint: departure sound test or reset after reverse underflow. |
| `map_object_pool_update` | 541 | `1` | `record->visual.animation_phase = (KF_FIXED12_ONE - 1);` | Greatest animation phase below Q12 unity; also the reverse departure endpoint. |
| `map_object_pool_update` | 551 | `1` | `if (record->visual.animation_phase == (KF_FIXED12_ONE - 1)) {` | Greatest animation phase below Q12 unity; also the reverse departure endpoint. |
| `map_object_pool_update` | 557 | `0` | `record->visual.animation_phase = 0;` | Zero interpolation endpoint: departure sound test or reset after reverse underflow. |
| `map_object_pool_update` | 563 | `0` | `if (object->link.link_id == KF_MAP_LINK_NONE && object->action_timer == 0) {` | Zero means the one-shot region copy has not been processed. |
| `map_object_pool_update` | 567 | `1` | `object->action_timer = 1;` | One records completion even when the copy parameter was absent. |
| `map_object_pool_update` | 574 | `0` | `if (object->action_timer == 0) {` | Zero selects the initial reveal displacement. |
| `map_object_pool_update` | 576 | `1` | `object->action_timer = 1;` | First settling timer; updates 1 through 5 restore the overshoot. |
| `map_object_pool_update` | 588 | `2` | `map_apply_copy_region(2);` | Authored copy-region table index applied when floor 3 completes four reveals. |
| `map_object_pool_update` | 589 | `3` | `map_apply_copy_region(3);` | Authored copy-region table index applied when floor 3 completes four reveals. |

## `src/game/map_object_pool.c`

78 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 25 | `55` | `{55, 33, 50, 39, 3, 3},` | Authored copy-region source X in map cells; explicit table data. |
| `initializers` | 25 | `33` | `{55, 33, 50, 39, 3, 3},` | Authored copy-region source Z in map cells; explicit table data. |
| `initializers` | 25 | `50` | `{55, 33, 50, 39, 3, 3},` | Authored copy-region destination X in map cells; explicit table data. |
| `initializers` | 25 | `39` | `{55, 33, 50, 39, 3, 3},` | Authored copy-region destination Z in map cells; explicit table data. |
| `initializers` | 25 | `3` | `{55, 33, 50, 39, 3, 3},` | Authored copy-region width in map cells; explicit table data. |
| `initializers` | 25 | `3` | `{55, 33, 50, 39, 3, 3},` | Authored copy-region height in map cells; explicit table data. |
| `initializers` | 26 | `47` | `{47, 16, 30, 20, 3, 3},` | Authored copy-region source X in map cells; explicit table data. |
| `initializers` | 26 | `16` | `{47, 16, 30, 20, 3, 3},` | Authored copy-region source Z in map cells; explicit table data. |
| `initializers` | 26 | `30` | `{47, 16, 30, 20, 3, 3},` | Authored copy-region destination X in map cells; explicit table data. |
| `initializers` | 26 | `20` | `{47, 16, 30, 20, 3, 3},` | Authored copy-region destination Z in map cells; explicit table data. |
| `initializers` | 26 | `3` | `{47, 16, 30, 20, 3, 3},` | Authored copy-region width in map cells; explicit table data. |
| `initializers` | 26 | `3` | `{47, 16, 30, 20, 3, 3},` | Authored copy-region height in map cells; explicit table data. |
| `initializers` | 27 | `58` | `{58, 44, 15, 48, 3, 3},` | Authored copy-region source X in map cells; explicit table data. |
| `initializers` | 27 | `44` | `{58, 44, 15, 48, 3, 3},` | Authored copy-region source Z in map cells; explicit table data. |
| `initializers` | 27 | `15` | `{58, 44, 15, 48, 3, 3},` | Authored copy-region destination X in map cells; explicit table data. |
| `initializers` | 27 | `48` | `{58, 44, 15, 48, 3, 3},` | Authored copy-region destination Z in map cells; explicit table data. |
| `initializers` | 27 | `3` | `{58, 44, 15, 48, 3, 3},` | Authored copy-region width in map cells; explicit table data. |
| `initializers` | 27 | `3` | `{58, 44, 15, 48, 3, 3},` | Authored copy-region height in map cells; explicit table data. |
| `initializers` | 28 | `64` | `{64, 44, 37, 45, 3, 3},` | Authored copy-region source X in map cells; explicit table data. |
| `initializers` | 28 | `44` | `{64, 44, 37, 45, 3, 3},` | Authored copy-region source Z in map cells; explicit table data. |
| `initializers` | 28 | `37` | `{64, 44, 37, 45, 3, 3},` | Authored copy-region destination X in map cells; explicit table data. |
| `initializers` | 28 | `45` | `{64, 44, 37, 45, 3, 3},` | Authored copy-region destination Z in map cells; explicit table data. |
| `initializers` | 28 | `3` | `{64, 44, 37, 45, 3, 3},` | Authored copy-region width in map cells; explicit table data. |
| `initializers` | 28 | `3` | `{64, 44, 37, 45, 3, 3},` | Authored copy-region height in map cells; explicit table data. |
| `initializers` | 29 | `0` | `{0, 0, 36, 4, 7, 1},` | Authored copy-region source X in map cells; explicit table data. |
| `initializers` | 29 | `0` | `{0, 0, 36, 4, 7, 1},` | Authored copy-region source Z in map cells; explicit table data. |
| `initializers` | 29 | `36` | `{0, 0, 36, 4, 7, 1},` | Authored copy-region destination X in map cells; explicit table data. |
| `initializers` | 29 | `4` | `{0, 0, 36, 4, 7, 1},` | Authored copy-region destination Z in map cells; explicit table data. |
| `initializers` | 29 | `7` | `{0, 0, 36, 4, 7, 1},` | Authored copy-region width in map cells; explicit table data. |
| `initializers` | 29 | `1` | `{0, 0, 36, 4, 7, 1},` | Authored copy-region height in map cells; explicit table data. |
| `map_apply_copy_region` | 50 | `0` | `while (height-- != 0) {` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| `map_apply_copy_region` | 54 | `0` | `while (width-- != 0) {` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| `map_object_mark_collision_edge` | 83 | `3` | `case KF_ENUM_DECODE(KfMapObjectBehavior, 3):` | Encoded behavior 3 has the two-cell collision profile and lift-door depth bias, but no row in the 141-definition retail table; independent behavior/action identity remains unresolved. |
| `map_object_mark_collision_edge` | 86 | `0x000` | `case 0x000:` | Zero yaw, the angular coordinate origin for this cardinal geometry branch. |
| `map_object_mark_collision_edge` | 103 | `0x000` | `case 0x000:` | Zero yaw, the angular coordinate origin for this cardinal geometry branch. |
| `map_object_mark_collision_edge` | 104 | `1` | `map_collision_grid[cell_z][cell_x + 1] =` | One-cell neighbor offset in the cardinal door-edge geometry. |
| `map_object_mark_collision_edge` | 105 | `1` | `map_collision_grid[cell_z - 1][cell_x + 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| `map_object_mark_collision_edge` | 105 | `1` | `map_collision_grid[cell_z - 1][cell_x + 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| `map_object_mark_collision_edge` | 108 | `1` | `map_collision_grid[cell_z + 1][cell_x] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| `map_object_mark_collision_edge` | 109 | `1` | `map_collision_grid[cell_z + 1][cell_x + 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| `map_object_mark_collision_edge` | 109 | `1` | `map_collision_grid[cell_z + 1][cell_x + 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| `map_object_mark_collision_edge` | 112 | `1` | `map_collision_grid[cell_z][cell_x - 1] =` | One-cell neighbor offset in the cardinal door-edge geometry. |
| `map_object_mark_collision_edge` | 113 | `1` | `map_collision_grid[cell_z + 1][cell_x - 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| `map_object_mark_collision_edge` | 113 | `1` | `map_collision_grid[cell_z + 1][cell_x - 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| `map_object_mark_collision_edge` | 116 | `1` | `map_collision_grid[cell_z - 1][cell_x] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| `map_object_mark_collision_edge` | 117 | `1` | `map_collision_grid[cell_z - 1][cell_x - 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| `map_object_mark_collision_edge` | 117 | `1` | `map_collision_grid[cell_z - 1][cell_x - 1] = value;` | One-cell neighbor offset in the cardinal door-edge geometry. |
| `map_object_probe_forward` | 137 | `0` | `point_x, KF_COLLISION_IGNORE_HEIGHT, point_z, MAP_DOOR_CLOSING_PROBE_RADIUS, 0,` | Zero vertical extent of the height-ignored distance/collision query. |
| `map_object_probe_forward` | 142 | `0x000` | `case 0x000:` | Zero yaw, the angular coordinate origin for this cardinal geometry branch. |
| `map_object_pool_clear` | 164 | `1` | `u16 index = KF_MAP_OBJECT_CAPACITY - 1;` | Inclusive countdown starts at the last slot index. |
| `map_object_pool_clear` | 172 | `1` | `link_words[1] = 0;` | Word indices 1/0 and zero stores clear the two aligned link words in retail order. |
| `map_object_pool_clear` | 172 | `0` | `link_words[1] = 0;` | Word indices 1/0 and zero stores clear the two aligned link words in retail order. |
| `map_object_pool_clear` | 173 | `0` | `link_words[0] = 0;` | Word indices 1/0 and zero stores clear the two aligned link words in retail order. |
| `map_object_pool_clear` | 173 | `0` | `link_words[0] = 0;` | Word indices 1/0 and zero stores clear the two aligned link words in retail order. |
| `map_object_pool_clear` | 175 | `0` | `} while (index-- != 0);` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| `map_object_pool_clear` | 176 | `0` | `map_object_effect_sequence_180 = 0;` | Reset the transient group allocation sequence to its initial value. |
| `map_object_pool_clear` | 177 | `0` | `map_object_effect_sequence_170 = 0;` | Reset the transient group allocation sequence to its initial value. |
| `map_object_pool_clear` | 178 | `0` | `map_object_effect_sequence_160 = 0;` | Reset the transient group allocation sequence to its initial value. |
| `map_object_definitions_load` | 190 | `0` | `} while (--count != 0);` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| `map_object_pool_load` | 203 | `0` | `u16 ended = 0;` | Boolean end-of-placement-list flag; after the sentinel, remaining slots are marked free. |
| `map_object_pool_load` | 207 | `2` | `u32 effect_output[2];` | Existing two-word constructor output scratch extent. |
| `map_object_pool_load` | 210 | `1` | `remaining = KF_MAP_OBJECT_CAPACITY - 1;` | Inclusive countdown starts at the last slot index. |
| `map_object_pool_load` | 212 | `1` | `if (ended == 1) {` | Boolean end-of-placement-list flag; after the sentinel, remaining slots are marked free. |
| `map_object_pool_load` | 220 | `0` | `object->rotation.z = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_pool_load` | 221 | `0` | `object->rotation.x = 0;` | Zero angle or vector component; no extra rotation or vertical displacement/velocity. |
| `map_object_pool_load` | 231 | `0` | `if (definition->collision_radius != 0) {` | Zero collision radius disables occupancy or skips a non-colliding definition. |
| `map_object_pool_load` | 232 | `1` | `collision_adjust_cell_occupancy(object->cell_x, object->cell_z, 1);` | Increment occupancy by exactly one loaded colliding object. |
| `map_object_pool_load` | 238 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| `map_object_pool_load` | 254 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| `map_object_pool_load` | 265 | `0x20` | `0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class; shared protocol identity remains under review, independent of collision target bits. |
| `map_object_pool_load` | 276 | `0` | `0, KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER, KF_EFFECT_KIND_MAP_SWITCH, &object->position_x,` | Stored effect ID zero for the switch visual. |
| `map_object_pool_load` | 300 | `1` | `ended = 1;` | Boolean end-of-placement-list flag; after the sentinel, remaining slots are marked free. |
| `map_object_pool_load` | 304 | `0` | `if (remaining-- == 0) {` | Zero exhausted-count termination; preserves pre/postdecrement ordering. |
| `map_object_distance_to_point` | 329 | `1` | `return -1;` | Established negative-one distance-query miss result. |
| `map_object_pool_find_near_point` | 339 | `0` | `for (index = 0; index < KF_MAP_OBJECT_CAPACITY; index++, object++) {` | Start scanning at the first pool slot. |
| `map_object_pool_find_near_point` | 344 | `0` | `if (radius == 0) {` | Zero collision radius disables occupancy or skips a non-colliding definition. |
| `map_object_pool_find_near_point` | 348 | `1` | `!= -1) {` | Established negative-one distance-query miss result. |
| `map_object_pool_find_near_point` | 352 | `1` | `return -1;` | Established negative-one distance-query miss result. |
