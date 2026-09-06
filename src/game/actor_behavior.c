#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_actor.h>
#include <kf/game_collision.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

enum {
    ACTOR_JUMP_RISING = 1,
    ACTOR_JUMP_ATTACK_PENDING = 2,
    ACTOR_JUMP_WAIT_FOR_LANDING = 3
};

enum {
    ACTOR_ACTIVATION_RANGE = 28000,
    ACTOR_NEAR_SPAWN_EXCLUSION_RANGE = 26000,
    ACTOR_ACTIVE_RANGE = 32000,
    /* World units per update squared; negative Y points upward. */
    ACTOR_GRAVITY = 20,
    /* Both query policies reject flag-grid bit 7; its broader role is unresolved. */
    ACTOR_WALK_COLLISION_FLAGS = (0x80 << KF_COLLISION_CELL_FLAG_SHIFT)
        | KF_COLLISION_SKIP_MAP_EVENTS,
    ACTOR_VELOCITY_COLLISION_FLAGS = ACTOR_WALK_COLLISION_FLAGS
        | KF_COLLISION_SKIP_MAP_OBJECTS
};

/* actor behavior/AI run (0x8002e2e8..0x80030817); shared jump-table rodata. */
RODATA(0x800124d4, 0x264)

ADDRESS(0x8002e2e8, 0x3c0)
void actor_select_next_action(s32 player_distance)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    KfActorAction action = actor->action;
    KfActorAction chosen = KF_ACTOR_ACTION_NONE;
    s32 recently_active;
    s32 awareness;
    s32 near_range;

    if (actor->action_progress == KF_ACTOR_PROGRESS_LOCKED) {
        return;
    }
    if (actor->action_progress != KF_ACTOR_PROGRESS_COMPLETE) {
        if (action == KF_ACTOR_ACTION_POST_DEATH) {
            return;
        }
        if (action == KF_ACTOR_ACTION_HIT_REACTION || action == KF_ACTOR_ACTION_DYING) {
            return;
        }
        if (action == KF_ACTOR_ACTION_EXIT_BLOCKED_PLACEMENT) {
            return;
        }
    }
    near_range = definition->pursuit_distance_scale << 8;
    awareness = definition->awareness_distance;
    if (definition->action_animations[KF_ACTOR_ANIM_SLOT_MULTI_HIT_ATTACK] != KF_ACTOR_ANIMATION_NONE
        && actor_try_select_facing_action(KF_ACTOR_ACTION_MULTI_HIT_ATTACK, player_distance, 0x50)
            != KF_ACTOR_ACTION_NONE) {
        chosen = KF_ACTOR_ACTION_MULTI_HIT_ATTACK;
    } else if (definition->action_animations[KF_ACTOR_ANIM_SLOT_JUMP_ATTACK] != KF_ACTOR_ANIMATION_NONE
               && actor_try_select_ground_action(
                      KF_ACTOR_ACTION_JUMP_ATTACK, player_distance, definition->special_attack_chance)
                   != KF_ACTOR_ACTION_NONE) {
        chosen = KF_ACTOR_ACTION_JUMP_ATTACK;
    } else if (definition->action_animations[KF_ACTOR_ANIM_SLOT_SPECIAL_ATTACK] != KF_ACTOR_ANIMATION_NONE
               && actor_try_select_action_distance_facing(
                      KF_ACTOR_ACTION_SPECIAL_ATTACK,
                      player_distance,
                      definition->special_attack_chance,
                      definition->special_attack_range)
                   != KF_ACTOR_ACTION_NONE) {
        chosen = KF_ACTOR_ACTION_SPECIAL_ATTACK;
    } else if (definition->action_animations[KF_ACTOR_ANIM_SLOT_EFFECT0] != KF_ACTOR_ANIMATION_NONE
               && actor_try_select_profiled_action(
                      KF_ACTOR_ACTION_EFFECT0,
                      player_distance,
                      definition->action_parameters[KF_ACTOR_PARAM_EFFECT0],
                      definition->action_parameters[KF_ACTOR_PARAM_EFFECT0_CHANCE])
                   != KF_ACTOR_ACTION_NONE) {
        chosen = KF_ACTOR_ACTION_EFFECT0;
    } else if (definition->action_animations[KF_ACTOR_ANIM_SLOT_EFFECT1] != KF_ACTOR_ANIMATION_NONE
               && actor_try_select_profiled_action(
                      KF_ACTOR_ACTION_EFFECT1,
                      player_distance,
                      definition->action_parameters[KF_ACTOR_PARAM_EFFECT1],
                      definition->action_parameters[KF_ACTOR_PARAM_EFFECT1_CHANCE])
                   != KF_ACTOR_ACTION_NONE) {
        chosen = KF_ACTOR_ACTION_EFFECT1;
    } else if (definition->action_animations[KF_ACTOR_ANIM_SLOT_EFFECT2] != KF_ACTOR_ANIMATION_NONE
               && actor_try_select_profiled_action(
                      KF_ACTOR_ACTION_EFFECT2,
                      player_distance,
                      definition->action_parameters[KF_ACTOR_PARAM_EFFECT2],
                      definition->action_parameters[KF_ACTOR_PARAM_EFFECT2_CHANCE])
                   != KF_ACTOR_ACTION_NONE) {
        chosen = KF_ACTOR_ACTION_EFFECT2;
    } else if (definition->action_animations[KF_ACTOR_ANIM_SLOT_MELEE] != KF_ACTOR_ANIMATION_NONE
               && actor_try_select_action_distance_facing(
                      KF_ACTOR_ACTION_MELEE_ATTACK,
                      player_distance,
                      definition->melee_attack_chance,
                      definition->awareness_distance)
                   != KF_ACTOR_ACTION_NONE) {
        chosen = KF_ACTOR_ACTION_MELEE_ATTACK;
    } else if (definition->action_animations[KF_ACTOR_ANIM_SLOT_DRIFT] != KF_ACTOR_ANIMATION_NONE) {
        chosen = KF_ACTOR_ACTION_DRIFT;
    } else {
        recently_active = action == KF_ACTOR_ACTION_RETREAT
            || action == KF_ACTOR_ACTION_MELEE_ATTACK
            || action == KF_ACTOR_ACTION_JUMP_ATTACK
            || action == KF_ACTOR_ACTION_SPECIAL_ATTACK
            || action == KF_ACTOR_ACTION_EFFECT0
            || action == KF_ACTOR_ACTION_EFFECT1
            || action == KF_ACTOR_ACTION_EFFECT2;
        if (definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE] != KF_ACTOR_ANIMATION_NONE) {
            if (recently_active || !(awareness < player_distance)) {
                if (!(awareness * 2 < player_distance) && !(rand() < 5462)) {
                    chosen = KF_ACTOR_ACTION_RETREAT;
                    goto choose;
                }
            }
        }
        if (definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE] != KF_ACTOR_ANIMATION_NONE) {
            if (recently_active || action == KF_ACTOR_ACTION_HIT_REACTION || action == KF_ACTOR_ACTION_PURSUE) {
                near_range = near_range * 6;
            }
            if (!(near_range < player_distance)) {
                chosen = KF_ACTOR_ACTION_PURSUE;
                goto choose;
            }
        }
        if (definition->action_animations[KF_ACTOR_ANIM_SLOT_IDLE] != KF_ACTOR_ANIMATION_NONE) {
            switch (action) {
            case KF_ACTOR_ACTION_WANDER:
                if (rand() < 1092) {
                    chosen = KF_ACTOR_ACTION_IDLE;
                    goto choose;
                }
                break;
            case KF_ACTOR_ACTION_IDLE:
                if (!(rand() < 8193)) {
                    chosen = KF_ACTOR_ACTION_IDLE;
                    goto choose;
                }
                break;
            }
            if (definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE] == KF_ACTOR_ANIMATION_NONE) {
                chosen = KF_ACTOR_ACTION_IDLE;
                goto choose;
            }
        } else if (definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE] == KF_ACTOR_ANIMATION_NONE) {
            goto choose;
        }
        chosen = KF_ACTOR_ACTION_WANDER;
        if (actor->slot_state == KF_ACTOR_SLOT_HOMEBOUND) {
            chosen = KF_ACTOR_ACTION_RETURN_HOME;
        }
    }
choose:
    if (chosen != actor->action || actor->action_progress == KF_ACTOR_PROGRESS_COMPLETE) {
        actor_set_action(actor, chosen);
    }
}

/*
 * Advances the current actor's lifecycle from the player's distance:
 * dormant actors wake within 28000 units (with a spawn roll and an
 * overlap check), active ones pick actions within 32000, and suppressed
 * ones wait for the player to leave that range before becoming dormant.
 */
ADDRESS(0x8002e6a8, 0x2ac)
void actor_update_awareness(void)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    u8 slot_state = actor->slot_state;
    s32 distance;
    s32 kind;

    switch (actor->lifecycle) {
    case KF_ACTOR_LIFECYCLE_DORMANT:
        distance = actor_distance_to_point(
            actor,
            actor_state.player_position.vx,
            KF_COLLISION_IGNORE_HEIGHT,
            actor_state.player_position.vz,
            ACTOR_ACTIVATION_RANGE,
            0,
            0);
        if (distance == -1) {
            return;
        }
        kind = slot_state;
        if (kind == KF_ACTOR_SLOT_RESPAWNING) {
            if ((actor->spawn_chance << 7) > rand()) {
                if (actor_pool_find_overlap(
                        actor->tile_x * KF_MAP_TILE_SIZE + actor->local_x,
                        KF_COLLISION_IGNORE_HEIGHT,
                        actor->tile_z * KF_MAP_TILE_SIZE + actor->local_z,
                        definition->collision_radius,
                        0)
                    == -1) {
                    actor_initialize_current();
                    if (definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE] != KF_ACTOR_ANIMATION_NONE) {
                        actor_set_action(actor, KF_ACTOR_ACTION_EXIT_BLOCKED_PLACEMENT);
                    } else {
                        actor_select_next_action(distance);
                    }
                }
            }
        } else {
            if (distance < ACTOR_NEAR_SPAWN_EXCLUSION_RANGE && player_state.allow_near_actor_spawn == 0) {
                actor->lifecycle = KF_ACTOR_LIFECYCLE_WAIT_FOR_RANGE_EXIT;
                return;
            }
            if ((actor->spawn_chance << 7) > rand()
                || kind == KF_ACTOR_SLOT_PERSISTENT || kind == KF_ACTOR_SLOT_HOMEBOUND) {
                if (actor_pool_find_overlap(
                        actor->tile_x * KF_MAP_TILE_SIZE + actor->local_x,
                        KF_COLLISION_IGNORE_HEIGHT,
                        actor->tile_z * KF_MAP_TILE_SIZE + actor->local_z,
                        definition->collision_radius,
                        0)
                    != -1) {
                    actor->lifecycle = KF_ACTOR_LIFECYCLE_WAIT_FOR_RANGE_EXIT;
                    return;
                }
                actor_initialize_current();
                actor_select_next_action(distance);
            } else {
                actor->lifecycle = KF_ACTOR_LIFECYCLE_WAIT_FOR_RANGE_EXIT;
            }
        }
        break;
    case KF_ACTOR_LIFECYCLE_ACTIVE:
        distance = actor_distance_to_point(
            actor,
            actor_state.player_position.vx,
            KF_COLLISION_IGNORE_HEIGHT,
            actor_state.player_position.vz,
            ACTOR_ACTIVE_RANGE,
            0,
            0);
        if (distance == -1) {
            actor->lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
        } else {
            actor_select_next_action(distance);
        }
        break;
    case KF_ACTOR_LIFECYCLE_WAIT_FOR_RANGE_EXIT:
        if (actor_distance_to_point(
                actor,
                actor_state.player_position.vx,
                KF_COLLISION_IGNORE_HEIGHT,
                actor_state.player_position.vz,
                ACTOR_ACTIVE_RANGE,
                0,
                0)
            == -1) {
            actor->lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
        }
        break;
    }
}

/*
 * Moves the current actor by DELTA on the ground plane.  A blocked move is
 * retried along one axis and turns the actor; on an open cell the drop or
 * rise to the floor starts a jump, fall, or step.  Returns 1 when blocked.
 */
ADDRESS(0x8002e954, 0x3ac)
s32 actor_move_xz_with_collision(const struct KfVecXZs *delta, s32 stop_on_collision)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    VECTOR target;
    s32 result;
    s32 drop;
    s32 threshold;

    target = actor->position;
    vector3i_add_xz((struct KfVec3i *)&target, delta);
    result = collision_query_world(
        target.vx,
        target.vy,
        target.vz,
        definition->collision_radius,
        definition->collision_height,
        ACTOR_WALK_COLLISION_FLAGS);
    if (result != KF_COLLISION_NONE
        && (result != KF_COLLISION_BELOW_FLOOR || actor->vertical_state == KF_ACTOR_VERTICAL_LONG_DROP)) {
    blocked:
        if (stop_on_collision == 0) {
            if (actor->collision_state != KF_ACTOR_COLLISION_BLOCKED) {
                if (collision_query_world(
                        actor->position.vx,
                        target.vy,
                        target.vz,
                        definition->collision_radius,
                        definition->collision_height,
                        ACTOR_WALK_COLLISION_FLAGS)
                    == KF_COLLISION_NONE) {
                    if (delta->z >= 0) {
                        actor->movement_yaw = KF_ANGLE_HALF_TURN;
                    } else {
                        actor->movement_yaw = 0;
                    }
                    actor->position.vz = target.vz;
                    actor->cell_z = target.vz / KF_MAP_TILE_SIZE;
                    actor->collision_state = KF_ACTOR_COLLISION_SLIDING;
                } else if (collision_query_world(
                               target.vx,
                               target.vy,
                               actor->position.vz,
                               definition->collision_radius,
                               definition->collision_height,
                               ACTOR_WALK_COLLISION_FLAGS)
                           == KF_COLLISION_NONE) {
                    if (delta->x < 0) {
                        actor->movement_yaw = KF_ANGLE_QUARTER_TURN;
                    } else {
                        actor->movement_yaw = KF_ANGLE_THREE_QUARTER_TURN;
                    }
                    actor->position.vx = target.vx;
                    actor->cell_x = target.vx / KF_MAP_TILE_SIZE;
                    actor->collision_state = KF_ACTOR_COLLISION_SLIDING;
                } else {
                    actor->movement_yaw = (actor->movement_yaw + KF_ANGLE_HALF_TURN) & KF_ANGLE_WRAP_MASK;
                    actor->collision_state = KF_ACTOR_COLLISION_BLOCKED;
                }
            } else if (actor->movement_yaw == actor->rotation.y) {
                actor->movement_yaw = (actor->movement_yaw + KF_ANGLE_FULL_TURN / 64) & KF_ANGLE_WRAP_MASK;
            }
        }
        return 1;
    }
    drop = target.vy - map_floor_height_at_position(&target);
    if (drop < 0) {
        if (drop < -600) {
            threshold = 0x4000;
            if (drop < -3000) {
                threshold = 0x400;
            }
            if (threshold < rand()) {
                goto blocked;
            }
            if (actor->vertical_state == KF_ACTOR_VERTICAL_NONE) {
                actor->vertical_state = KF_ACTOR_VERTICAL_LONG_DROP;
                actor->vertical_velocity = 0;
            }
        } else if (actor->vertical_state == KF_ACTOR_VERTICAL_NONE) {
            actor->vertical_state = KF_ACTOR_VERTICAL_FALL;
            actor->vertical_velocity = 0;
        }
    } else if (drop > 0) {
        if (!(drop < 1001)) {
            goto blocked;
        }
        if (drop >= 400) {
            if (actor->vertical_state == KF_ACTOR_VERTICAL_NONE) {
                actor->vertical_state = KF_ACTOR_VERTICAL_STEP_UP;
                actor->vertical_velocity = -300;
            }
        } else if (actor->vertical_state == KF_ACTOR_VERTICAL_NONE) {
            actor->vertical_state = KF_ACTOR_VERTICAL_STEP_UP;
            actor->vertical_velocity = -120;
        }
    }
    actor->collision_state = KF_ACTOR_COLLISION_CLEAR;
    actor->position.vx = target.vx;
    actor->position.vz = target.vz;
    actor->cell_x = target.vx / KF_MAP_TILE_SIZE;
    actor->cell_z = target.vz / KF_MAP_TILE_SIZE;
    return 0;
}


ADDRESS(0x8002ed00, 0xd4)
s32 actor_move_along_heading(s32 direction, s32 stop_on_collision)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    struct KfVecXZs delta;
    u32 rate;

    if (actor->collision_state == KF_ACTOR_COLLISION_SLIDING) {
        rate = definition->turn_rate;
        actor->rotation.y = angle_approach(actor->rotation.y, actor->movement_yaw, (rate + rate + rate) >> 1);
    } else {
        actor->rotation.y = angle_approach(actor->rotation.y, actor->movement_yaw, definition->turn_rate);
    }
    angle_to_forward_xz(actor->rotation.y, &delta);
    vector2s_scale_shift11(definition->move_speed, &delta);
    if (direction < 0) {
        delta.x = -delta.x;
        delta.z = -delta.z;
    }
    return actor_move_xz_with_collision(&delta, stop_on_collision);
}

/*
 * Spawns the effect of EFFECT_CODE at the actor's attachment point: the
 * offset is rotated by the actor's angles, aimed at the player when the
 * player is inside the cone, scaled per code, and handed to the spawner.
 * Bit 5 of the code spawns a mirrored pair.
 */
ADDRESS(0x8002edd4, 0x454)
void actor_spawn_action_effect(s32 effect_code, s32 attachment_index)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    SVECTOR direction;
    SVECTOR offset;
    struct KfEulerAngles angles;
    VECTOR position;
    MATRIX matrix;
    struct KfEulerAngles burst_angles;
    s32 repeat;
    s32 i;
    s16 facing;
    s32 distance;
    s32 scale;

    repeat = 1;
    if (effect_code & KF_ACTOR_EFFECT_PAIRED) {
        repeat = 2;
    }
    effect_code &= KF_ACTOR_EFFECT_KIND_MASK;
    for (i = 0; i < repeat; i++) {
        switch (effect_code) {
        case 5:
        case 7:
        case 8:
        case KF_EFFECT_KIND_ACTOR_SPAWNER:
        case KF_EFFECT_KIND_SCATTER_PROJECTILE:
        case KF_EFFECT_KIND_DARKNESS_PROJECTILE:
        case KF_EFFECT_KIND_CURSE_PROJECTILE:
        case KF_EFFECT_KIND_EMERGING_PROJECTILE:
        case 22:
        case 23:
        case 24:
            offset.vx = definition->attachment_offsets[attachment_index].x;
            offset.vy = definition->attachment_offsets[attachment_index].y;
            offset.vz = definition->attachment_offsets[attachment_index].z;
            if (repeat == 2) {
                if (i == 0) {
                    offset.vx = offset.vx + 1500;
                } else {
                    offset.vx = offset.vx - 1500;
                }
            }
            angles.x = actor->rotation.x;
            angles.y = -actor->rotation.y & KF_ANGLE_WRAP_MASK;
            angles.z = actor->rotation.z;
            matrix_set_rotation_yxz(&angles, &matrix);
            ApplyMatrix(&matrix, &offset, &position);
            position.vx += actor->position.vx;
            position.vy += actor->position.vy;
            position.vz += actor->position.vz;
            facing = (KF_ANGLE_HALF_TURN - actor->rotation.y) & KF_ANGLE_WRAP_MASK;
            distance = player_distance_to_point_in_cone(
                (struct KfVec3i *)&position, facing, 50000, KF_ACTOR_AIM_TOLERANCE);
            if (distance == -1) {
                angles.x = 0;
                if (effect_code == 23) {
                    scale = 800;
                    angles.x = -32;
                    distance = 20;
                } else if (effect_code == KF_EFFECT_KIND_ACTOR_SPAWNER
                           || effect_code == KF_EFFECT_KIND_SCATTER_PROJECTILE) {
                    scale = 250;
                    distance = 20;
                } else {
                    scale = 600;
                }
                angles.y = facing;
            } else {
                angles.y = vector_xz_to_angle(
                    actor_state.player_position.vx - position.vx,
                    position.vz - actor_state.player_position.vz);
                if (effect_code == 23) {
                    scale = 800;
                    angles.x = vector_xz_to_angle(
                        position.vy - (actor_state.player_position.vy - 1000), -distance);
                    distance = distance / scale;
                } else {
                    angles.x = vector_xz_to_angle(
                        position.vy - actor_state.player_position.vy, -distance);
                    if (effect_code == KF_EFFECT_KIND_SCATTER_PROJECTILE) {
                        scale = 250;
                        distance -= 4500;
                    /* Retail shares one step-count clamp between codes 10 and 9. */
                    clamp_steps:
                        if (distance <= scale) {
                            distance = 1;
                        } else {
                            distance = distance / scale;
                        }
                    } else if (effect_code == KF_EFFECT_KIND_ACTOR_SPAWNER) {
                        scale = 250;
                        distance -= 2000;
                        goto clamp_steps;
                    } else {
                        scale = 600;
                    }
                }
            }
            angles.z = 0;
            if (effect_code == 7) {
                scale = 800;
            }
            pitch_yaw_to_forward_vector((struct KfPitchYaw *)&angles, &direction);
            vector3s_scale_shift12(scale, &direction);
            if (effect_code == 8 || effect_code == 22) {
                effect_pool_construct(
                    definition->effect_owner_id, 0x20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                    effect_code, &position, &direction, &angles, 1);
            } else if (effect_code == 24) {
                burst_angles.x = actor->rotation.x;
                burst_angles.y = facing;
                burst_angles.z = actor->rotation.z;
                effect_pool_construct(
                    definition->effect_owner_id, 0x20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER, 24,
                    &position, &direction, &burst_angles, KF_EFFECT_HOMING_PLAYER, 1);
            } else if (effect_code == KF_EFFECT_KIND_SCATTER_PROJECTILE) {
                effect_pool_construct(
                    definition->effect_owner_id, 0x20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                    KF_EFFECT_KIND_SCATTER_PROJECTILE, &position, &direction, 3, distance, 0xbb8);
            } else {
                effect_pool_construct(
                    definition->effect_owner_id, 0x20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                    effect_code, &position, &direction, distance, 1);
            }
            break;
        }
    }
}


ADDRESS(0x8002f228, 0xf4)
void actor_prepare_charge_toward_player(void)
{
    KfActor *actor = actor_state.current;
    struct KfVecXZs delta;
    s32 length;

    actor->movement_yaw = vector_xz_to_angle(
        actor_state.player_position.vx - actor->position.vx,
        actor_state.player_position.vz - actor->position.vz);
    if (angle_within_tolerance(actor->rotation.y, (s16)actor->movement_yaw, KF_ACTOR_AIM_TOLERANCE) == 0) {
        actor->movement_yaw = actor->rotation.y;
    }
    length = fixed_vector2_length(
        actor_state.player_position.vx - actor->position.vx,
        actor_state.player_position.vz - actor->position.vz);
    angle_to_forward_xz(actor->movement_yaw, &delta);
    vector2s_scale_shift11(length >> 2, &delta);
    actor->movement_x = delta.x / 16;
    actor->movement_z = delta.z / 16;
}

/* Applies the horizontal velocity, bouncing or stopping on collision. */
ADDRESS(0x8002f31c, 0x14c)
void actor_apply_horizontal_movement(void)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition;
    struct KfVec3i target;
    s32 result;

    target.x = actor->movement_x + actor->position.vx;
    target.z = actor->movement_z + actor->position.vz;
    definition = actor_state.current_definition;
    result = collision_query_world(
        target.x,
        actor->position.vy,
        target.z,
        definition->collision_radius,
        definition->collision_height,
        ACTOR_VELOCITY_COLLISION_FLAGS);
    if (result != KF_COLLISION_NONE) {
        if ((result >> 16) != (KF_COLLISION_PLAYER >> 16)) {
            actor->movement_x = -actor->movement_x >> 1;
            actor->movement_z = -actor->movement_z >> 1;
        } else {
            actor->movement_z = 0;
            actor->movement_x = 0;
        }
    } else {
        actor->position.vx = target.x;
        actor->position.vz = target.z;
    }
    actor->cell_x = actor->position.vx / KF_MAP_TILE_SIZE;
    actor->cell_z = actor->position.vz / KF_MAP_TILE_SIZE;
}

/* ACTION indexes a configured effect; the dispatcher passes 0, 1 or 2. */
ADDRESS(0x8002f468, 0xf0)
void actor_update_effect_action(s32 action)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    s32 index = action + KF_ACTOR_ANIM_SLOT_EFFECT0;

    if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
        actor->action_progress = KF_ACTOR_PROGRESS_LOCKED;
        actor->animation_id = definition->action_animations[index];
        actor->animation_phase = 0;
    }
    actor_advance_animation_clamped(actor, definition->action_animation_steps[index]);
    if (actor_animation_crossed_phase(actor, definition->action_animation_phases[index])) {
        actor_spawn_action_effect(definition->action_parameters[action], action);
    }
    if (actor->animation_phase >= KF_ACTOR_ANIMATION_PHASE_MAX) {
        actor->action_progress = KF_ACTOR_PROGRESS_COMPLETE;
        actor_select_next_action(actor_distance_to_point(
            actor,
            actor_state.player_position.vx,
            KF_COLLISION_IGNORE_HEIGHT,
            actor_state.player_position.vz,
            ACTOR_ACTIVE_RANGE,
            0,
            0));
    }
}

/*
 * Nudges each velocity component by STEP in a random direction, clamped to
 * LIMIT, then applies the whole vector; a blocked move marks progress
 * nonzero and reflects the tested axes. Y is tested only if X was clear.
 */
ADDRESS(0x8002f558, 0x374)
void actor_apply_random_movement(s16 step, s16 limit)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    VECTOR target;
    s32 result;

    if (rand() < (RAND_MAX + 1) / 2) {
        actor->movement_x += step;
        if (actor->movement_x > limit) {
            actor->movement_x = limit;
        }
    } else {
        actor->movement_x -= step;
        if (actor->movement_x < -limit) {
            actor->movement_x = -limit;
        }
    }
    if (rand() < (RAND_MAX + 1) / 2) {
        actor->movement_z += step;
        if (actor->movement_z > limit) {
            actor->movement_z = limit;
        }
    } else {
        actor->movement_z -= step;
        if (actor->movement_z < -limit) {
            actor->movement_z = -limit;
        }
    }
    if (rand() < (RAND_MAX + 1) / 2) {
        actor->movement_y += step;
        if (actor->movement_y > limit) {
            actor->movement_y = limit;
        }
    } else {
        actor->movement_y -= step;
        if (actor->movement_y < -limit) {
            actor->movement_y = -limit;
        }
    }
    target.vx = actor->movement_x + actor->position.vx;
    target.vz = actor->movement_z + actor->position.vz;
    target.vy = actor->movement_y + actor->position.vy;
    result = collision_query_world(
        target.vx,
        target.vy,
        target.vz,
        definition->collision_radius,
        definition->collision_height,
        ACTOR_VELOCITY_COLLISION_FLAGS);
    if (result == KF_COLLISION_NONE) {
        actor->position = target;
    } else {
        /* Drift only tests whether progress is zero; 2 has no distinct use. */
        actor->action_progress = 2;
        result = collision_query_world(
            target.vx,
            actor->position.vy,
            actor->position.vz,
            definition->collision_radius,
            definition->collision_height,
            ACTOR_VELOCITY_COLLISION_FLAGS);
        if (result != KF_COLLISION_NONE) {
            actor->movement_x = -actor->movement_x;
        } else if (collision_query_world(
                       actor->position.vx,
                       target.vy,
                       actor->position.vz,
                       definition->collision_radius,
                       definition->collision_height,
                       ACTOR_VELOCITY_COLLISION_FLAGS)
                   != KF_COLLISION_NONE) {
            actor->movement_y = -actor->movement_y;
        }
        if (collision_query_world(
                actor->position.vx,
                actor->position.vy,
                target.vz,
                definition->collision_radius,
                definition->collision_height,
                ACTOR_VELOCITY_COLLISION_FLAGS)
            != KF_COLLISION_NONE) {
            actor->movement_z = -actor->movement_z;
        }
    }
    actor->cell_x = actor->position.vx / KF_MAP_TILE_SIZE;
    actor->cell_z = actor->position.vz / KF_MAP_TILE_SIZE;
}


ADDRESS(0x8002f8cc, 0x1bc)
void actor_update_boss_death_sequence(void)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    SVECTOR direction;
    VECTOR position;

    actor_play_sound_at_phase(&boss_death_phase_sounds[1], 500);
    actor_play_sound_at_phase(&boss_death_phase_sounds[2], 1000);
    actor_play_sound_at_phase(&boss_death_phase_sounds[3], 1500);
    if (actor->animation_phase >= KF_ACTOR_ANIMATION_PHASE_MAX) {
        actor->animation_phase = KF_ACTOR_ANIMATION_PHASE_MAX;
        actor->action_progress = KF_ACTOR_PROGRESS_INIT;
        actor->lifecycle = KF_ACTOR_LIFECYCLE_DISABLED;
        boss_defeat_complete = 1;
        map_object_pool_trigger_link(13);
        actor_pool_begin_death_by_definition(0);
        actor_pool_begin_death_by_definition(2);
        actor_pool_begin_death_by_definition(3);
        actor_pool_begin_death_by_definition(4);
    }
    if (actor->animation_phase % (definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_DEATH] * 2) == 0) {
        position.vx = actor->position.vx + (rand() & 0x1fff) - 4096;
        position.vz = actor->position.vz + (rand() & 0x1fff) - 4096;
        position.vy = actor->position.vy - (rand() & 0xfff);
        /* Retail leaves this kind's direction and the position pad unwritten. */
        effect_pool_construct(
            0, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER, 0x2c,
            &position, &direction, 0);
        if (actor->animation_phase % (definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_DEATH] * 4) == 0) {
            sound_ref_play(&boss_death_loop_sound, 100);
        }
    }
}

/*
 * Runs the bound actor's current action, then its vertical state. The
 * actor leaves the collision cell census while it moves and rejoins at the
 * end; the removal paths return early so a dead actor is not counted again.
 */
ADDRESS(0x8002fa88, 0xd90)
void actor_update_current_action(void)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    struct KfVecXZs direction;
    struct KfVec3i target;
    s32 result;
    s32 hit;
    s32 floor_height;
    s32 next_y;
    s32 home_x;
    s32 home_z;
    u16 debris;
    u8 attribute;

    collision_adjust_cell_occupancy(actor->cell_x, actor->cell_z, -1);
    switch (actor->action) {
    case KF_ACTOR_ACTION_IDLE:
        if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
            actor->action_progress = KF_ACTOR_PROGRESS_RUNNING;
            actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_IDLE];
            actor->animation_phase = 0;
        }
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_IDLE]);
        break;
    case KF_ACTOR_ACTION_WANDER:
        if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
            actor->action_progress = KF_ACTOR_PROGRESS_RUNNING;
            if (actor->animation_id != definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE]) {
                actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE];
                actor->animation_phase = 0;
            }
            actor->movement_yaw = rand() >> 3;
        } else if (actor->collision_state == KF_ACTOR_COLLISION_CLEAR && rand() < 2048) {
            actor->movement_yaw = rand() >> 3;
        }
        actor_move_along_heading(1, 0);
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_MOVE]);
        break;
    case KF_ACTOR_ACTION_PURSUE:
        switch (actor->action_progress) {
        case KF_ACTOR_PROGRESS_INIT:
            actor->action_progress = KF_ACTOR_PROGRESS_RUNNING;
            if (actor->animation_id != definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE]) {
                actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE];
                actor->animation_phase = 0;
            }
            actor->movement_yaw = vector_xz_to_angle(
                actor_state.player_position.vx - actor->position.vx,
                actor_state.player_position.vz - actor->position.vz);
            break;
        case KF_ACTOR_PROGRESS_RUNNING:
            if (actor_move_along_heading(1, 1) != 0) {
                actor->action_progress = (rand() >> 11) + 13;
                goto vertical;
            }
            if (rand() < 4096) {
                actor->movement_yaw = vector_xz_to_angle(
                    actor_state.player_position.vx - actor->position.vx,
                    actor_state.player_position.vz - actor->position.vz);
            }
            break;
        default:
            if (actor_move_along_heading(-1, 1) != 0 || actor->action_progress < 11) {
                actor->action_progress = KF_ACTOR_PROGRESS_RUNNING;
            } else {
                actor->action_progress--;
            }
            break;
        }
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_MOVE]);
        break;
    case KF_ACTOR_ACTION_HIT_REACTION:
        if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
            actor->action_progress = KF_ACTOR_PROGRESS_RUNNING;
            actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_HIT_REACTION];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_HIT_REACTION]);
        actor_play_sound_at_phase(&definition->sounds[KF_ACTOR_SOUND_HIT_REACTION], definition->action_animation_phases[KF_ACTOR_ANIM_SLOT_HIT_REACTION]);
        if (actor->animation_phase >= KF_ACTOR_ANIMATION_PHASE_MAX) {
            actor->action_progress = KF_ACTOR_PROGRESS_COMPLETE;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.vx,
                KF_COLLISION_IGNORE_HEIGHT,
                actor_state.player_position.vz,
                ACTOR_ACTIVE_RANGE,
                0,
                0));
        }
        break;
    case KF_ACTOR_ACTION_DYING:
        if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
            actor->action_progress = KF_ACTOR_PROGRESS_RUNNING;
            actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_DEATH];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_DEATH]);
        actor_play_sound_at_phase(&definition->sounds[KF_ACTOR_SOUND_DEATH], definition->action_animation_phases[KF_ACTOR_ANIM_SLOT_DEATH]);
        if (player_state.progress_state.current_floor == 5 && actor->definition_id == 7) {
            actor_update_boss_death_sequence();
            return;
        }
        if (actor_animation_crossed_phase(actor, 0x800)) {
            debris = ((u32)rand() * definition->gold_drop_limit) >> 15;
            if (debris != 0) {
                map_object_spawn_actor_debris(
                    debris, (struct KfVec3i *)&actor->position, -(definition->collision_height >> 1));
            }
            if (actor->slot_state == KF_ACTOR_SLOT_DYNAMIC || actor->slot_state == KF_ACTOR_SLOT_RESPAWNING) {
                if (definition->action_parameters[KF_ACTOR_PARAM_DROP_OBJECT] != 0x63 && definition->action_parameters[KF_ACTOR_PARAM_DROP_OBJECT] != 0xff
                    && (rand() >> 7) <= definition->action_parameters[KF_ACTOR_PARAM_DROP_CHANCE]) {
                    map_object_spawn_effect(
                        KF_MAP_OBJECT_DROP_FROM_DEFINITION,
                        definition->action_parameters[KF_ACTOR_PARAM_DROP_OBJECT],
                        (struct KfVec3i *)&actor->position,
                        -(definition->collision_height >> 1));
                }
            } else if (actor->death_drop_object_id != 0x63) {
                map_object_spawn_effect(
                    KF_MAP_OBJECT_DROP_FROM_PLACEMENT,
                    actor->death_drop_object_id,
                    (struct KfVec3i *)&actor->position,
                    -(definition->collision_height >> 1));
            }
        }
        if (actor->animation_phase >= KF_ACTOR_ANIMATION_PHASE_MAX) {
            actor->action = KF_ACTOR_ACTION_POST_DEATH;
            actor->animation_phase = KF_ACTOR_ANIMATION_PHASE_MAX;
            actor->action_progress = KF_ACTOR_PROGRESS_INIT;
        }
        break;
    case KF_ACTOR_ACTION_RETREAT:
        if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
            actor->action_progress = KF_ACTOR_PROGRESS_RUNNING;
            if (actor->animation_id != definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE]) {
                actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE];
                actor->animation_phase = 0;
            }
            actor->movement_yaw = vector_xz_to_angle(
                actor_state.player_position.vx - actor->position.vx,
                actor_state.player_position.vz - actor->position.vz);
        }
        if (actor_move_along_heading(-1, 1) != 0) {
            actor->action = KF_ACTOR_ACTION_NONE;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.vx,
                KF_COLLISION_IGNORE_HEIGHT,
                actor_state.player_position.vz,
                ACTOR_ACTIVE_RANGE,
                0,
                0));
        } else {
            actor_advance_animation_wrapped(actor, definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_MOVE]);
        }
        break;
    case KF_ACTOR_ACTION_MELEE_ATTACK:
        if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
            actor->action_progress = KF_ACTOR_PROGRESS_LOCKED;
            actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_MELEE];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_MELEE]);
        actor_play_sound_at_phase(&definition->sounds[KF_ACTOR_SOUND_ATTACK], definition->action_animation_phases[KF_ACTOR_ANIM_SLOT_MELEE]);
        if (actor->animation_phase >= 2700
            && actor->animation_phase < definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_MELEE] + 2700) {
            actor_try_attack_player(0, definition->awareness_distance, 0, KF_ACTOR_AIM_TOLERANCE);
        }
        if (actor->animation_phase >= KF_ACTOR_ANIMATION_PHASE_MAX) {
            actor->action_progress = KF_ACTOR_PROGRESS_COMPLETE;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.vx,
                KF_COLLISION_IGNORE_HEIGHT,
                actor_state.player_position.vz,
                ACTOR_ACTIVE_RANGE,
                0,
                0));
        }
        break;
    case KF_ACTOR_ACTION_POST_DEATH:
        if (player_state.progress_state.current_floor == 4 && actor->definition_id == 5) {
            actor_transform_definition5_to6(actor);
            actor_initialize(actor);
            return;
        }
        if (actor->action_progress >= 7) {
            if (actor->slot_state == KF_ACTOR_SLOT_RESPAWNING) {
                actor->lifecycle = KF_ACTOR_LIFECYCLE_DORMANT;
            } else if (actor->slot_state == KF_ACTOR_SLOT_PERSISTENT || actor->slot_state == KF_ACTOR_SLOT_HOMEBOUND) {
                actor->lifecycle = KF_ACTOR_LIFECYCLE_DISABLED;
            } else {
                actor->lifecycle = KF_ACTOR_LIFECYCLE_WAIT_FOR_RANGE_EXIT;
            }
            return;
        }
        actor->action_progress++;
        break;
    case KF_ACTOR_ACTION_JUMP_ATTACK:
        if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
            actor->action_progress = ACTOR_JUMP_RISING;
            actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_JUMP_ATTACK];
            actor->animation_phase = 0;
            attribute = map_cell_attribute_grid[actor->cell_z][actor->cell_x];
            if (map_cell_attribute_height_table[attribute - 1] > -5000) {
                actor->vertical_state = KF_ACTOR_VERTICAL_JUMP_ATTACK;
                actor->vertical_velocity = -220;
                actor->animation_step = 140;
            } else {
                actor->vertical_state = KF_ACTOR_VERTICAL_JUMP_ATTACK;
                actor->vertical_velocity = -300;
                actor->animation_step = 110;
            }
            actor_prepare_charge_toward_player();
        }
        actor_apply_horizontal_movement();
        actor->animation_phase += actor->animation_step;
        if (actor->animation_phase >= KF_ACTOR_ANIMATION_PHASE_MAX) {
            actor->animation_phase = KF_ACTOR_ANIMATION_PHASE_MAX;
        }
        actor_play_sound_at_phase(&definition->sounds[KF_ACTOR_SOUND_ATTACK], definition->action_animation_phases[KF_ACTOR_ANIM_SLOT_JUMP_ATTACK]);
        switch (actor->action_progress) {
        case ACTOR_JUMP_RISING:
            if (actor->vertical_velocity >= 0) {
                actor_prepare_charge_toward_player();
                actor->action_progress = ACTOR_JUMP_ATTACK_PENDING;
            }
            break;
        case ACTOR_JUMP_ATTACK_PENDING:
            if (actor_animation_crossed_phase(actor, 0xd48)) {
                actor_try_attack_player(0, definition->special_attack_range, 0, KF_ACTOR_AIM_TOLERANCE);
                actor->action_progress = ACTOR_JUMP_WAIT_FOR_LANDING;
            }
            break;
        case ACTOR_JUMP_WAIT_FOR_LANDING:
            if (actor->vertical_state != KF_ACTOR_VERTICAL_JUMP_ATTACK) {
                actor->vertical_state = KF_ACTOR_VERTICAL_NONE;
                actor->vertical_velocity = 0;
                result = collision_query_world(
                    actor->position.vx,
                    actor->position.vy,
                    actor->position.vz,
                    definition->collision_radius,
                    definition->collision_height,
                    ACTOR_VELOCITY_COLLISION_FLAGS);
                /* Retail compares the high half against the low-half ceiling code. */
                if (result != KF_COLLISION_NONE && (result >> 16) == 0xfff1) {
                    actor->vertical_state = KF_ACTOR_VERTICAL_FALL;
                    actor->vertical_velocity = 0;
                }
                actor->action_progress = KF_ACTOR_PROGRESS_COMPLETE;
                actor_select_next_action(actor_distance_to_point(
                    actor,
                    actor_state.player_position.vx,
                    KF_COLLISION_IGNORE_HEIGHT,
                    actor_state.player_position.vz,
                    ACTOR_ACTIVE_RANGE,
                    0,
                    0));
            }
            break;
        }
        break;
    case KF_ACTOR_ACTION_SPECIAL_ATTACK:
        if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
            actor->action_progress = KF_ACTOR_PROGRESS_LOCKED;
            actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_SPECIAL_ATTACK];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_SPECIAL_ATTACK]);
        actor_play_sound_at_phase(&definition->sounds[KF_ACTOR_SOUND_ATTACK], definition->action_animation_phases[KF_ACTOR_ANIM_SLOT_SPECIAL_ATTACK]);
        if (actor->animation_phase >= 3000
            && actor->animation_phase < definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_SPECIAL_ATTACK] + 3000) {
            actor_try_attack_player(0, definition->special_attack_range, 0, KF_ACTOR_AIM_TOLERANCE);
        }
        if (actor->animation_phase >= KF_ACTOR_ANIMATION_PHASE_MAX) {
            actor->action_progress = KF_ACTOR_PROGRESS_COMPLETE;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.vx,
                KF_COLLISION_IGNORE_HEIGHT,
                actor_state.player_position.vz,
                ACTOR_ACTIVE_RANGE,
                0,
                0));
        }
        break;
    case KF_ACTOR_ACTION_EXIT_BLOCKED_PLACEMENT:
        if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
            actor->action_progress = KF_ACTOR_PROGRESS_RUNNING;
            actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE];
            actor->animation_phase = 0;
        }
        result = collision_query_world(
            actor->position.vx, KF_COLLISION_IGNORE_HEIGHT, actor->position.vz, definition->collision_radius, 0, ACTOR_WALK_COLLISION_FLAGS);
        if (result != KF_COLLISION_NONE) {
            target.x = actor->position.vx;
            target.z = actor->position.vz;
            angle_to_forward_xz(actor->rotation.y, &direction);
            vector2s_scale_shift11(definition->move_speed, &direction);
            vector3i_add_xz(&target, &direction);
            if (actor_pool_find_overlap(target.x, KF_COLLISION_IGNORE_HEIGHT, target.z, definition->collision_radius, 0)
                == -1) {
                actor->position.vx = target.x;
                actor->position.vz = target.z;
            }
        } else {
            actor->action_progress = KF_ACTOR_PROGRESS_COMPLETE;
            actor_select_next_action(ACTOR_ACTIVE_RANGE);
        }
        break;
    case KF_ACTOR_ACTION_DRIFT:
        if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
            actor->action_progress = KF_ACTOR_PROGRESS_RUNNING;
            actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_DRIFT];
            actor->animation_phase = 0;
            actor->movement_y = 0;
            actor->movement_z = 0;
            actor->movement_x = 0;
            actor->movement_yaw = 0;
            actor->vertical_state = KF_ACTOR_VERTICAL_NONE;
            actor->vertical_velocity = 0;
        }
        actor_apply_random_movement(1, 100);
        if (rand() < (RAND_MAX + 1) / 2) {
            actor->movement_yaw++;
            if (actor->movement_yaw > 32) {
                actor->movement_yaw = 32;
            }
        } else {
            actor->movement_yaw--;
            if (actor->movement_yaw < -32) {
                actor->movement_yaw = -32;
            }
        }
        actor->rotation.y = (actor->rotation.y + actor->movement_yaw) & KF_ANGLE_WRAP_MASK;
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_DRIFT]);
        break;
    case KF_ACTOR_ACTION_EFFECT0:
        actor_update_effect_action(0);
        break;
    case KF_ACTOR_ACTION_EFFECT1:
        actor_update_effect_action(1);
        break;
    case KF_ACTOR_ACTION_EFFECT2:
        actor_update_effect_action(2);
        break;
    case KF_ACTOR_ACTION_RETURN_HOME:
        if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
            actor->action_progress = KF_ACTOR_PROGRESS_RUNNING;
            if (actor->animation_id != definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE]) {
                actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_MOVE];
                actor->animation_phase = 0;
            }
            /* Retail reads the home position before this branch assigns it. */
            actor->movement_yaw =
                vector_xz_to_angle(home_x - actor->position.vx, home_z - actor->position.vz);
        } else if (actor->collision_state == KF_ACTOR_COLLISION_CLEAR) {
            home_x = actor->tile_x * KF_MAP_TILE_SIZE + actor->local_x;
            home_z = actor->tile_z * KF_MAP_TILE_SIZE + actor->local_z;
            if (actor->position.vx - home_x > -200 && actor->position.vx - home_x < 200
                && actor->position.vz - home_z > -200 && actor->position.vz - home_z < 200) {
                actor->movement_yaw = actor->heading_quadrant * KF_ANGLE_QUARTER_TURN;
                actor->rotation.y = angle_approach(
                    actor->rotation.y, actor->movement_yaw, definition->turn_rate);
                if (actor->animation_id != definition->action_animations[KF_ACTOR_ANIM_SLOT_MELEE]) {
                    break;
                }
                if (actor->movement_yaw == actor->rotation.y
                    && actor_animation_crossed_phase(actor, 0x4b0)) {
                    actor->animation_phase = 0;
                    actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_MELEE];
                    break;
                }
                actor_advance_animation_wrapped(actor, definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_MOVE]);
                break;
            }
            if (rand() < 2048) {
                actor->movement_yaw = vector_xz_to_angle(
                    home_x - actor->position.vx, home_z - actor->position.vz);
            }
        }
        actor_move_along_heading(1, 0);
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_MOVE]);
        break;
    case KF_ACTOR_ACTION_MULTI_HIT_ATTACK:
        if (actor->action_progress == KF_ACTOR_PROGRESS_INIT) {
            actor->action_progress = KF_ACTOR_PROGRESS_LOCKED;
            actor->animation_id = definition->action_animations[KF_ACTOR_ANIM_SLOT_MULTI_HIT_ATTACK];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[KF_ACTOR_ANIM_SLOT_MULTI_HIT_ATTACK]);
        actor_play_sound_at_phase(&definition->sounds[KF_ACTOR_SOUND_ATTACK], definition->action_animation_phases[KF_ACTOR_ANIM_SLOT_MULTI_HIT_ATTACK]);
        if (actor_animation_crossed_phase(actor, 0x8c0) || actor_animation_crossed_phase(actor, 0xa80)
            || actor_animation_crossed_phase(actor, 0xc80)
            || actor_animation_crossed_phase(actor, 0xe00)) {
            actor_try_attack_player(8000, 11000, 0, KF_ACTOR_AIM_TOLERANCE);
        }
        if (actor_animation_crossed_phase(actor, 0xdac)) {
            actor_try_attack_player(0, 11000, -KF_ANGLE_EIGHTH_TURN, KF_ACTOR_AIM_TOLERANCE);
        }
        if (actor_animation_crossed_phase(actor, 0xed8)) {
            actor_try_attack_player(0, 11000, KF_ANGLE_EIGHTH_TURN, KF_ACTOR_AIM_TOLERANCE);
        }
        if (actor->animation_phase >= KF_ACTOR_ANIMATION_PHASE_MAX) {
            actor->action_progress = KF_ACTOR_PROGRESS_COMPLETE;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.vx,
                KF_COLLISION_IGNORE_HEIGHT,
                actor_state.player_position.vz,
                ACTOR_ACTIVE_RANGE,
                0,
                0));
        }
        break;
    }
vertical:
    switch (actor->vertical_state) {
    case KF_ACTOR_VERTICAL_NONE:
        break;
    case KF_ACTOR_VERTICAL_STEP_UP:
        floor_height = map_floor_height_at_position(&actor->position);
        next_y = actor->vertical_velocity + actor->position.vy;
        if (next_y > floor_height) {
            goto fall;
        }
    land:
        actor->position.vy = floor_height;
        actor->vertical_state = KF_ACTOR_VERTICAL_NONE;
        actor->vertical_velocity = 0;
        break;
    fall:
        actor->position.vy = next_y;
        actor->vertical_velocity += ACTOR_GRAVITY;
        break;
    case KF_ACTOR_VERTICAL_FALL:
    case KF_ACTOR_VERTICAL_LONG_DROP:
        floor_height = map_floor_height_at_position(&actor->position);
        next_y = actor->vertical_velocity + actor->position.vy;
        if (next_y >= floor_height) {
            goto land;
        }
        goto fall;
    case KF_ACTOR_VERTICAL_JUMP_ATTACK:
        next_y = actor->vertical_velocity + actor->position.vy;
        hit = collision_query_world(
            actor->position.vx,
            next_y,
            actor->position.vz,
            definition->collision_radius,
            definition->collision_height,
            ACTOR_VELOCITY_COLLISION_FLAGS);
        if (hit == KF_COLLISION_NONE) {
            goto fall;
        }
        if ((hit >> 16) == (KF_COLLISION_PLAYER >> 16)) {
            player_apply_damage(0, 15, 0, 0, 0, 0, KF_FIXED12_ONE, 10);
        stagger:
            actor->vertical_state = KF_ACTOR_VERTICAL_JUMP_ATTACK;
            actor->vertical_velocity = -120;
            actor->animation_phase = 0;
        } else if ((hit >> 16) == (KF_COLLISION_TERRAIN >> 16)) {
            switch (hit & 0xffff) {
            case (KF_COLLISION_BELOW_FLOOR & 0xffff):
                floor_height = map_floor_height_at_position(&actor->position);
                goto land;
            case (KF_COLLISION_CEILING & 0xffff):
                actor->vertical_state = KF_ACTOR_VERTICAL_JUMP_ATTACK;
                actor->vertical_velocity = 100;
                break;
            }
        } else if ((hit >> 16) == (KF_COLLISION_ACTOR >> 16)) {
            goto stagger;
        }
        break;
    }
    collision_adjust_cell_occupancy(actor->cell_x, actor->cell_z, 1);
}
