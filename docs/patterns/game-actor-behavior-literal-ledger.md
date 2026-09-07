# Actor-behavior remainder literal ledger

Complete account of **171 numeric occurrences** in `src/game/actor_behavior.c`.
The [behavior review](game-actor-behavior-constants.md) records the names, units,
evidence and verification. Comments, strings, identifier digits, enums and retail
claims are excluded. Repeated tokens have separate rows; signs stay in expressions.

The [floor enum review](game-floor-enum-domain.md) propagates floor identifiers
through the current source; this ledger reflects its named comparisons and
explicit numeric boundaries.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `actor_update_awareness` | 282 | `0` | `0,` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_awareness` | 283 | `0` | `0);` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_awareness` | 284 | `1` | `if (distance == -1) {` | Established negative-one miss from distance/overlap queries. |
| `actor_update_awareness` | 295 | `0` | `0)` | Zero vertical extent for this height-ignored overlap query. |
| `actor_update_awareness` | 296 | `1` | `== -1) {` | Established negative-one miss from distance/overlap queries. |
| `actor_update_awareness` | 306 | `0` | `if (distance < ACTOR_NEAR_SPAWN_EXCLUSION_RANGE && player_state.allow_near_actor_spawn == 0) {` | False disables the near-player spawn exception. |
| `actor_update_awareness` | 317 | `0` | `0)` | Zero vertical extent for this height-ignored overlap query. |
| `actor_update_awareness` | 318 | `1` | `!= -1) {` | Established negative-one miss from distance/overlap queries. |
| `actor_update_awareness` | 336 | `0` | `0,` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_awareness` | 337 | `0` | `0);` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_awareness` | 338 | `1` | `if (distance == -1) {` | Established negative-one miss from distance/overlap queries. |
| `actor_update_awareness` | 351 | `0` | `0,` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_awareness` | 352 | `0` | `0)` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_awareness` | 353 | `1` | `== -1) {` | Established negative-one miss from distance/overlap queries. |
| `actor_move_xz_with_collision` | 387 | `0` | `if (stop_on_collision == 0) {` | False permits axis-specific sliding retries. |
| `actor_move_xz_with_collision` | 397 | `0` | `if (delta->z >= 0) {` | Zero sign boundary selects the cardinal sliding direction. |
| `actor_move_xz_with_collision` | 400 | `0` | `actor->movement_yaw = 0;` | Zero movement component or yaw coordinate; initialization or stopping after player collision. |
| `actor_move_xz_with_collision` | 413 | `0` | `if (delta->x < 0) {` | Zero sign boundary selects the cardinal sliding direction. |
| `actor_move_xz_with_collision` | 429 | `1` | `return 1;` | True reports blocked movement. |
| `actor_move_xz_with_collision` | 432 | `0` | `if (drop < 0) {` | Zero floor-relative displacement separates drops, level movement and steps. |
| `actor_move_xz_with_collision` | 443 | `0` | `actor->vertical_velocity = 0;` | Clear vertical velocity at initialization, landing or transition. |
| `actor_move_xz_with_collision` | 447 | `0` | `actor->vertical_velocity = 0;` | Clear vertical velocity at initialization, landing or transition. |
| `actor_move_xz_with_collision` | 449 | `0` | `} else if (drop > 0) {` | Zero floor-relative displacement separates drops, level movement and steps. |
| `actor_move_xz_with_collision` | 468 | `0` | `return 0;` | False reports successful movement. |
| `actor_move_along_heading` | 482 | `1` | `actor->rotation.y = angle_approach(actor->rotation.y, actor->movement_yaw, (rate + rate + rate) >> 1);` | Halve the three-term turn-rate sum for the sliding 3/2 steering rate. |
| `actor_move_along_heading` | 488 | `0` | `if (direction < 0) {` | Negative direction reverses the computed heading vector. |
| `actor_spawn_action_effect` | 518 | `1` | `repeat = 1;` | One ordinary emission or two paired emissions; direct loop cardinality. |
| `actor_spawn_action_effect` | 520 | `2` | `repeat = 2;` | One ordinary emission or two paired emissions; direct loop cardinality. |
| `actor_spawn_action_effect` | 523 | `0` | `for (i = 0; i < repeat; i++) {` | Begin the emission loop at its first iteration. |
| `actor_spawn_action_effect` | 533 | `22` | `case 22:` | Input kind 22 uses model 14 and the shared physical-projectile path; specific asset identity unresolved. |
| `actor_spawn_action_effect` | 534 | `23` | `case 23:` | Input kind 23 selects alternate Lightning Bolt billboard 17, then becomes kind 4; particular resource identity unresolved. |
| `actor_spawn_action_effect` | 535 | `24` | `case 24:` | Input kind 24 selects alternate homing model 16, then becomes kind 20; particular resource identity unresolved. |
| `actor_spawn_action_effect` | 539 | `2` | `if (repeat == 2) {` | One ordinary emission or two paired emissions; direct loop cardinality. |
| `actor_spawn_action_effect` | 540 | `0` | `if (i == 0) {` | First paired emission chooses the positive local-X offset. |
| `actor_spawn_action_effect` | 557 | `1` | `if (distance == -1) {` | Established negative-one miss from distance/overlap queries. |
| `actor_spawn_action_effect` | 558 | `0` | `angles.x = 0;` | Zero pitch/roll coordinate before any kind-specific aiming adjustment. |
| `actor_spawn_action_effect` | 559 | `23` | `if (effect_code == 23) {` | Input kind 23 selects alternate Lightning Bolt billboard 17, then becomes kind 4; particular resource identity unresolved. |
| `actor_spawn_action_effect` | 575 | `23` | `if (effect_code == 23) {` | Input kind 23 selects alternate Lightning Bolt billboard 17, then becomes kind 4; particular resource identity unresolved. |
| `actor_spawn_action_effect` | 589 | `1` | `distance = 1;` | Minimum movement count when the standoff-adjusted distance is at most one speed step. |
| `actor_spawn_action_effect` | 602 | `0` | `angles.z = 0;` | Zero pitch/roll coordinate before any kind-specific aiming adjustment. |
| `actor_spawn_action_effect` | 608 | `22` | `if (effect_code == KF_EFFECT_KIND_LIGHT_NEEDLE \|\| effect_code == 22) {` | Input kind 22 uses model 14 and the shared physical-projectile path; specific asset identity unresolved. |
| `actor_spawn_action_effect` | 610 | `0x20` | `definition->effect_owner_id, 0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class, distinct from its named collision-target bits; independent class identity remains unresolved. |
| `actor_spawn_action_effect` | 611 | `1` | `effect_code, &position, &direction, &angles, 1);` | True requests sound in constructor kinds that consume this variadic slot; ignored slots keep their original argument. |
| `actor_spawn_action_effect` | 612 | `24` | `} else if (effect_code == 24) {` | Input kind 24 selects alternate homing model 16, then becomes kind 20; particular resource identity unresolved. |
| `actor_spawn_action_effect` | 617 | `0x20` | `definition->effect_owner_id, 0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER, 24,` | Effect damage-credit class, distinct from its named collision-target bits; independent class identity remains unresolved. |
| `actor_spawn_action_effect` | 617 | `24` | `definition->effect_owner_id, 0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER, 24,` | Input kind 24 selects alternate homing model 16, then becomes kind 20; particular resource identity unresolved. |
| `actor_spawn_action_effect` | 618 | `1` | `&position, &direction, &burst_angles, KF_EFFECT_HOMING_PLAYER, 1);` | True requests sound in constructor kinds that consume this variadic slot; ignored slots keep their original argument. |
| `actor_spawn_action_effect` | 621 | `0x20` | `definition->effect_owner_id, 0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class, distinct from its named collision-target bits; independent class identity remains unresolved. |
| `actor_spawn_action_effect` | 625 | `0x20` | `definition->effect_owner_id, 0x20 \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,` | Effect damage-credit class, distinct from its named collision-target bits; independent class identity remains unresolved. |
| `actor_spawn_action_effect` | 626 | `1` | `effect_code, &position, &direction, distance, 1);` | True requests sound in constructor kinds that consume this variadic slot; ignored slots keep their original argument. |
| `actor_prepare_charge_toward_player` | 644 | `0` | `if (angle_within_tolerance(actor->rotation.y, (s16)actor->movement_yaw, KF_ACTOR_AIM_TOLERANCE) == 0) {` | False cone test keeps the actor current yaw rather than targeting the player. |
| `actor_apply_horizontal_movement` | 676 | `16` | `if ((result >> 16) != (KF_COLLISION_PLAYER >> 16)) {` | Extract the encoded collision result high halfword, from both the result and its named class as applicable. |
| `actor_apply_horizontal_movement` | 676 | `16` | `if ((result >> 16) != (KF_COLLISION_PLAYER >> 16)) {` | Extract the encoded collision result high halfword, from both the result and its named class as applicable. |
| `actor_apply_horizontal_movement` | 677 | `1` | `actor->movement_x = -actor->movement_x >> 1;` | Arithmetic halving after velocity negation on horizontal collision; preserve signed rounding order. |
| `actor_apply_horizontal_movement` | 678 | `1` | `actor->movement_z = -actor->movement_z >> 1;` | Arithmetic halving after velocity negation on horizontal collision; preserve signed rounding order. |
| `actor_apply_horizontal_movement` | 680 | `0` | `actor->movement_z = 0;` | Zero movement component or yaw coordinate; initialization or stopping after player collision. |
| `actor_apply_horizontal_movement` | 681 | `0` | `actor->movement_x = 0;` | Zero movement component or yaw coordinate; initialization or stopping after player collision. |
| `actor_update_effect_action` | 702 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_effect_action` | 716 | `0` | `0,` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_effect_action` | 717 | `0` | `0));` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_apply_random_movement` | 734 | `1` | `if (rand() < (RAND_MAX + 1) / 2) {` | Add one to count all 32768 SDK outcomes, then divide by two for the equal-size sign-selection intervals. |
| `actor_apply_random_movement` | 734 | `2` | `if (rand() < (RAND_MAX + 1) / 2) {` | Add one to count all 32768 SDK outcomes, then divide by two for the equal-size sign-selection intervals. |
| `actor_apply_random_movement` | 745 | `1` | `if (rand() < (RAND_MAX + 1) / 2) {` | Add one to count all 32768 SDK outcomes, then divide by two for the equal-size sign-selection intervals. |
| `actor_apply_random_movement` | 745 | `2` | `if (rand() < (RAND_MAX + 1) / 2) {` | Add one to count all 32768 SDK outcomes, then divide by two for the equal-size sign-selection intervals. |
| `actor_apply_random_movement` | 756 | `1` | `if (rand() < (RAND_MAX + 1) / 2) {` | Add one to count all 32768 SDK outcomes, then divide by two for the equal-size sign-selection intervals. |
| `actor_apply_random_movement` | 756 | `2` | `if (rand() < (RAND_MAX + 1) / 2) {` | Add one to count all 32768 SDK outcomes, then divide by two for the equal-size sign-selection intervals. |
| `actor_apply_random_movement` | 781 | `2` | `actor->action_progress = 2;` | Drift collision writes a nonzero marker; no consumer distinguishes this exact nonzero value in this action. |
| `actor_update_boss_death_sequence` | 825 | `1` | `actor_play_sound_at_phase(&boss_death_phase_sounds[1], ACTOR_BOSS_DEATH_SOUND_PHASE1);` | Authored sound-record index 1, 2 or 3; the packed fourth record is consumed and is not padding. |
| `actor_update_boss_death_sequence` | 826 | `2` | `actor_play_sound_at_phase(&boss_death_phase_sounds[2], ACTOR_BOSS_DEATH_SOUND_PHASE2);` | Authored sound-record index 1, 2 or 3; the packed fourth record is consumed and is not padding. |
| `actor_update_boss_death_sequence` | 827 | `3` | `actor_play_sound_at_phase(&boss_death_phase_sounds[3], ACTOR_BOSS_DEATH_SOUND_PHASE3);` | Authored sound-record index 1, 2 or 3; the packed fourth record is consumed and is not padding. |
| `actor_update_boss_death_sequence` | 833 | `13` | `map_object_pool_trigger_link(13);` | Authored link 13 fired at boss-death completion. |
| `actor_update_boss_death_sequence` | 834 | `0` | `actor_pool_begin_death_by_definition(0);` | Authored actor-definition cleanup selector from the set 0,2,3,4. |
| `actor_update_boss_death_sequence` | 835 | `2` | `actor_pool_begin_death_by_definition(2);` | Authored actor-definition cleanup selector from the set 0,2,3,4. |
| `actor_update_boss_death_sequence` | 836 | `3` | `actor_pool_begin_death_by_definition(3);` | Authored actor-definition cleanup selector from the set 0,2,3,4. |
| `actor_update_boss_death_sequence` | 837 | `4` | `actor_pool_begin_death_by_definition(4);` | Authored actor-definition cleanup selector from the set 0,2,3,4. |
| `actor_update_boss_death_sequence` | 839 | `0` | `if (actor->animation_phase % (definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_DEATH] * ACTOR_BOSS_DEATH_EFFECT_PERIOD) == 0) {` | Zero remainder tests exact phase divisibility for this effect/sound cadence. |
| `actor_update_boss_death_sequence` | 845 | `0` | `0, KF_EFFECT_USE_PLAYER_MAGIC \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER, 0x2c,` | Stored effect ID zero for the boss-death visual; separate from type and kind. |
| `actor_update_boss_death_sequence` | 845 | `0x2c` | `0, KF_EFFECT_USE_PLAYER_MAGIC \| KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER, 0x2c,` | Input kind 44 selects alternate radial-blast model 17, then becomes kind 18; particular resource identity unresolved. |
| `actor_update_boss_death_sequence` | 846 | `0` | `&position, &direction, 0);` | False suppresses the optional construction sound for the boss-death visual. |
| `actor_update_boss_death_sequence` | 847 | `0` | `if (actor->animation_phase % (definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_DEATH] * ACTOR_BOSS_DEATH_SOUND_PERIOD) == 0) {` | Zero remainder tests exact phase divisibility for this effect/sound cadence. |
| `actor_update_current_action` | 874 | `1` | `collision_adjust_cell_occupancy(actor->cell_x, actor->cell_z, -1);` | Remove exactly one actor before movement, then add it back after non-removal paths. |
| `actor_update_current_action` | 880 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 889 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 895 | `1` | `actor_move_along_heading(1, 0);` | Direction sign: positive one moves forward, negative one reverses both horizontal components. |
| `actor_update_current_action` | 895 | `0` | `actor_move_along_heading(1, 0);` | Boolean collision option: zero permits sliding retries; one stops on collision. |
| `actor_update_current_action` | 904 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 911 | `1` | `if (actor_move_along_heading(1, 1) != 0) {` | Direction sign: positive one moves forward, negative one reverses both horizontal components. |
| `actor_update_current_action` | 911 | `1` | `if (actor_move_along_heading(1, 1) != 0) {` | Boolean collision option: zero permits sliding retries; one stops on collision. |
| `actor_update_current_action` | 911 | `0` | `if (actor_move_along_heading(1, 1) != 0) {` | Nonzero return reports a blocked move. |
| `actor_update_current_action` | 922 | `1` | `if (actor_move_along_heading(-1, 1) != 0 \|\| actor->action_progress < ACTOR_PURSUIT_BACKOFF_END) {` | Direction sign: positive one moves forward, negative one reverses both horizontal components. |
| `actor_update_current_action` | 922 | `1` | `if (actor_move_along_heading(-1, 1) != 0 \|\| actor->action_progress < ACTOR_PURSUIT_BACKOFF_END) {` | Boolean collision option: zero permits sliding retries; one stops on collision. |
| `actor_update_current_action` | 922 | `0` | `if (actor_move_along_heading(-1, 1) != 0 \|\| actor->action_progress < ACTOR_PURSUIT_BACKOFF_END) {` | Nonzero return reports a blocked move. |
| `actor_update_current_action` | 935 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 947 | `0` | `0,` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_current_action` | 948 | `0` | `0));` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_current_action` | 955 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 959 | `7` | `if (player_state.progress_state.current_floor == KF_FLOOR_5 && actor->definition_id == 7) {` | Authored floor/actor-definition pair: 5/7 selects boss death, 4/5 selects transformation. |
| `actor_update_current_action` | 965 | `0` | `if (debris != 0) {` | Skip spawning a gold drop when its computed amount is zero. |
| `actor_update_current_action` | 967 | `1` | `debris, (struct KfVec3i *)&actor->position, -(definition->collision_height >> 1));` | Half the collision height places drops at the actor volume center. |
| `actor_update_current_action` | 976 | `1` | `-(definition->collision_height >> 1));` | Half the collision height places drops at the actor volume center. |
| `actor_update_current_action` | 983 | `1` | `-(definition->collision_height >> 1));` | Half the collision height places drops at the actor volume center. |
| `actor_update_current_action` | 997 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 1003 | `1` | `if (actor_move_along_heading(-1, 1) != 0) {` | Direction sign: positive one moves forward, negative one reverses both horizontal components. |
| `actor_update_current_action` | 1003 | `1` | `if (actor_move_along_heading(-1, 1) != 0) {` | Boolean collision option: zero permits sliding retries; one stops on collision. |
| `actor_update_current_action` | 1003 | `0` | `if (actor_move_along_heading(-1, 1) != 0) {` | Nonzero return reports a blocked move. |
| `actor_update_current_action` | 1011 | `0` | `0,` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_current_action` | 1012 | `0` | `0));` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_current_action` | 1021 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 1027 | `0` | `actor_try_attack_player(0, definition->awareness_distance, 0, KF_ACTOR_AIM_TOLERANCE);` | Zero minimum attack range; no lower exclusion. |
| `actor_update_current_action` | 1027 | `0` | `actor_try_attack_player(0, definition->awareness_distance, 0, KF_ACTOR_AIM_TOLERANCE);` | Zero yaw offset; attack along the current facing. |
| `actor_update_current_action` | 1037 | `0` | `0,` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_current_action` | 1038 | `0` | `0));` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_current_action` | 1042 | `5` | `if (player_state.progress_state.current_floor == KF_FLOOR_4 && actor->definition_id == 5) {` | Authored floor/actor-definition pair: 5/7 selects boss death, 4/5 selects transformation. |
| `actor_update_current_action` | 1063 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 1065 | `1` | `if (map_cell_attribute_height_table[attribute - 1] > ACTOR_JUMP_HEIGHT_THRESHOLD) {` | Retail one-row bias in the attribute-height lookup; preserve even for attribute zero. |
| `actor_update_current_action` | 1084 | `0` | `if (actor->vertical_velocity >= 0) {` | End of upward motion; reaching nonnegative Y velocity triggers the second charge preparation. |
| `actor_update_current_action` | 1091 | `0` | `actor_try_attack_player(0, definition->special_attack_range, 0, KF_ACTOR_AIM_TOLERANCE);` | Zero minimum attack range; no lower exclusion. |
| `actor_update_current_action` | 1091 | `0` | `actor_try_attack_player(0, definition->special_attack_range, 0, KF_ACTOR_AIM_TOLERANCE);` | Zero yaw offset; attack along the current facing. |
| `actor_update_current_action` | 1098 | `0` | `actor->vertical_velocity = 0;` | Clear vertical velocity at initialization, landing or transition. |
| `actor_update_current_action` | 1107 | `16` | `if (result != KF_COLLISION_NONE && (result >> 16) == 0xfff1) {` | Extract the encoded collision result high halfword, from both the result and its named class as applicable. |
| `actor_update_current_action` | 1107 | `0xfff1` | `if (result != KF_COLLISION_NONE && (result >> 16) == 0xfff1) {` | Retail compares the high result halfword against the low-half ceiling code; keep this exceptional comparison without correcting it. |
| `actor_update_current_action` | 1109 | `0` | `actor->vertical_velocity = 0;` | Clear vertical velocity at initialization, landing or transition. |
| `actor_update_current_action` | 1118 | `0` | `0,` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_current_action` | 1119 | `0` | `0));` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_current_action` | 1128 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 1134 | `0` | `actor_try_attack_player(0, definition->special_attack_range, 0, KF_ACTOR_AIM_TOLERANCE);` | Zero minimum attack range; no lower exclusion. |
| `actor_update_current_action` | 1134 | `0` | `actor_try_attack_player(0, definition->special_attack_range, 0, KF_ACTOR_AIM_TOLERANCE);` | Zero yaw offset; attack along the current facing. |
| `actor_update_current_action` | 1144 | `0` | `0,` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_current_action` | 1145 | `0` | `0));` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_current_action` | 1152 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 1155 | `0` | `actor->position.vx, KF_COLLISION_IGNORE_HEIGHT, actor->position.vz, definition->collision_radius, 0, ACTOR_WALK_COLLISION_FLAGS);` | Zero vertical extent while the query ignores height. |
| `actor_update_current_action` | 1162 | `0` | `if (actor_pool_find_overlap(target.x, KF_COLLISION_IGNORE_HEIGHT, target.z, definition->collision_radius, 0)` | Zero vertical extent while the query ignores height. |
| `actor_update_current_action` | 1163 | `1` | `== -1) {` | Established negative-one miss from distance/overlap queries. |
| `actor_update_current_action` | 1176 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 1177 | `0` | `actor->movement_y = 0;` | Zero movement component or yaw coordinate; initialization or stopping after player collision. |
| `actor_update_current_action` | 1178 | `0` | `actor->movement_z = 0;` | Zero movement component or yaw coordinate; initialization or stopping after player collision. |
| `actor_update_current_action` | 1179 | `0` | `actor->movement_x = 0;` | Zero movement component or yaw coordinate; initialization or stopping after player collision. |
| `actor_update_current_action` | 1180 | `0` | `actor->movement_yaw = 0;` | Zero movement component or yaw coordinate; initialization or stopping after player collision. |
| `actor_update_current_action` | 1182 | `0` | `actor->vertical_velocity = 0;` | Clear vertical velocity at initialization, landing or transition. |
| `actor_update_current_action` | 1185 | `1` | `if (rand() < (RAND_MAX + 1) / 2) {` | Add one to count all 32768 SDK outcomes, then divide by two for the equal-size sign-selection intervals. |
| `actor_update_current_action` | 1185 | `2` | `if (rand() < (RAND_MAX + 1) / 2) {` | Add one to count all 32768 SDK outcomes, then divide by two for the equal-size sign-selection intervals. |
| `actor_update_current_action` | 1200 | `0` | `actor_update_effect_action(0);` | Positional effect/attachment index 0,1,2; named action cases select the corresponding definition entry. |
| `actor_update_current_action` | 1203 | `1` | `actor_update_effect_action(1);` | Positional effect/attachment index 0,1,2; named action cases select the corresponding definition entry. |
| `actor_update_current_action` | 1206 | `2` | `actor_update_effect_action(2);` | Positional effect/attachment index 0,1,2; named action cases select the corresponding definition entry. |
| `actor_update_current_action` | 1213 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 1236 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 1248 | `1` | `actor_move_along_heading(1, 0);` | Direction sign: positive one moves forward, negative one reverses both horizontal components. |
| `actor_update_current_action` | 1248 | `0` | `actor_move_along_heading(1, 0);` | Boolean collision option: zero permits sliding retries; one stops on collision. |
| `actor_update_current_action` | 1255 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 1262 | `0` | `actor_try_attack_player(KF_ACTOR_MULTI_HIT_FORWARD_MIN_RANGE, KF_ACTOR_MULTI_HIT_MAX_RANGE, 0, KF_ACTOR_AIM_TOLERANCE);` | Zero yaw offset; attack along the current facing. |
| `actor_update_current_action` | 1265 | `0` | `actor_try_attack_player(0, KF_ACTOR_MULTI_HIT_MAX_RANGE, -KF_ANGLE_EIGHTH_TURN, KF_ACTOR_AIM_TOLERANCE);` | Zero minimum attack range; no lower exclusion. |
| `actor_update_current_action` | 1268 | `0` | `actor_try_attack_player(0, KF_ACTOR_MULTI_HIT_MAX_RANGE, KF_ANGLE_EIGHTH_TURN, KF_ACTOR_AIM_TOLERANCE);` | Zero minimum attack range; no lower exclusion. |
| `actor_update_current_action` | 1278 | `0` | `0,` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_current_action` | 1279 | `0` | `0));` | Zero height or vertical padding in the height-ignored actor-distance query. |
| `actor_update_current_action` | 1296 | `0` | `actor->vertical_velocity = 0;` | Clear vertical velocity at initialization, landing or transition. |
| `actor_update_current_action` | 1322 | `16` | `if ((hit >> 16) == (KF_COLLISION_PLAYER >> 16)) {` | Extract the encoded collision result high halfword, from both the result and its named class as applicable. |
| `actor_update_current_action` | 1322 | `16` | `if ((hit >> 16) == (KF_COLLISION_PLAYER >> 16)) {` | Extract the encoded collision result high halfword, from both the result and its named class as applicable. |
| `actor_update_current_action` | 1323 | `0` | `player_apply_damage(0, ACTOR_JUMP_CONTACT_STRIKING_DAMAGE, 0, 0, 0, 0, KF_FIXED12_ONE, KF_PLAYER_DAMAGE_MULTIPLIER_ONE);` | Absent damage component or status payload; the only nonzero component here is the named striking input. |
| `actor_update_current_action` | 1323 | `0` | `player_apply_damage(0, ACTOR_JUMP_CONTACT_STRIKING_DAMAGE, 0, 0, 0, 0, KF_FIXED12_ONE, KF_PLAYER_DAMAGE_MULTIPLIER_ONE);` | Absent damage component or status payload; the only nonzero component here is the named striking input. |
| `actor_update_current_action` | 1323 | `0` | `player_apply_damage(0, ACTOR_JUMP_CONTACT_STRIKING_DAMAGE, 0, 0, 0, 0, KF_FIXED12_ONE, KF_PLAYER_DAMAGE_MULTIPLIER_ONE);` | Absent damage component or status payload; the only nonzero component here is the named striking input. |
| `actor_update_current_action` | 1323 | `0` | `player_apply_damage(0, ACTOR_JUMP_CONTACT_STRIKING_DAMAGE, 0, 0, 0, 0, KF_FIXED12_ONE, KF_PLAYER_DAMAGE_MULTIPLIER_ONE);` | Absent damage component or status payload; the only nonzero component here is the named striking input. |
| `actor_update_current_action` | 1323 | `0` | `player_apply_damage(0, ACTOR_JUMP_CONTACT_STRIKING_DAMAGE, 0, 0, 0, 0, KF_FIXED12_ONE, KF_PLAYER_DAMAGE_MULTIPLIER_ONE);` | Absent damage component or status payload; the only nonzero component here is the named striking input. |
| `actor_update_current_action` | 1327 | `0` | `actor->animation_phase = 0;` | Reset the selected animation cycle to its initial phase. |
| `actor_update_current_action` | 1328 | `16` | `} else if ((hit >> 16) == (KF_COLLISION_TERRAIN >> 16)) {` | Extract the encoded collision result high halfword, from both the result and its named class as applicable. |
| `actor_update_current_action` | 1328 | `16` | `} else if ((hit >> 16) == (KF_COLLISION_TERRAIN >> 16)) {` | Extract the encoded collision result high halfword, from both the result and its named class as applicable. |
| `actor_update_current_action` | 1329 | `0xffff` | `switch (hit & 0xffff) {` | Extract the encoded terrain-detail low halfword. |
| `actor_update_current_action` | 1330 | `0xffff` | `case (KF_COLLISION_BELOW_FLOOR & 0xffff):` | Extract the encoded terrain-detail low halfword. |
| `actor_update_current_action` | 1333 | `0xffff` | `case (KF_COLLISION_CEILING & 0xffff):` | Extract the encoded terrain-detail low halfword. |
| `actor_update_current_action` | 1338 | `16` | `} else if ((hit >> 16) == (KF_COLLISION_ACTOR >> 16)) {` | Extract the encoded collision result high halfword, from both the result and its named class as applicable. |
| `actor_update_current_action` | 1338 | `16` | `} else if ((hit >> 16) == (KF_COLLISION_ACTOR >> 16)) {` | Extract the encoded collision result high halfword, from both the result and its named class as applicable. |
| `actor_update_current_action` | 1343 | `1` | `collision_adjust_cell_occupancy(actor->cell_x, actor->cell_z, 1);` | Remove exactly one actor before movement, then add it back after non-removal paths. |
