#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

extern s32 rand(void);

#define ACTOR_ACTION_NONE 0xff
#define MAP_TILE_SIZE 2000

/* actor behavior/AI run (0x8002e2e8..0x80030817); shared jump-table rodata. */
RODATA(0x800124d4, 0x264)

ADDRESS(0x8002e2e8, 0x3c0)
void actor_select_next_action(s32 player_distance)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    u8 action = actor->action;
    u8 chosen = ACTOR_ACTION_NONE;
    s32 recently_active;
    s32 awareness;
    s32 near_range;

    if (actor->action_timer == 0xf0) {
        return;
    }
    if (actor->action_timer != ACTOR_ACTION_NONE) {
        if (action == 0x7f) {
            return;
        }
        if (action == 5 || action == 6) {
            return;
        }
        if (action == 0x20) {
            return;
        }
    }
    near_range = definition->unknown_00[0] << 8;
    awareness = definition->awareness_distance;
    if (definition->action_animations[0x16 - 11] != ACTOR_ACTION_NONE
        && actor_try_select_facing_action(0x16, player_distance, 0x50) != ACTOR_ACTION_NONE) {
        chosen = 0x16;
    } else if (definition->action_animations[0x10 - 11] != ACTOR_ACTION_NONE
               && actor_try_select_ground_action(0x10, player_distance, definition->unknown_34)
                   != ACTOR_ACTION_NONE) {
        chosen = 0x10;
    } else if (definition->action_animations[0x11 - 11] != ACTOR_ACTION_NONE
               && actor_try_select_action_distance_facing(
                      0x11, player_distance, definition->unknown_34, definition->unknown_36)
                   != ACTOR_ACTION_NONE) {
        chosen = 0x11;
    } else if (definition->action_animations[0x13 - 11] != ACTOR_ACTION_NONE
               && actor_try_select_profiled_action(
                      0x13,
                      player_distance,
                      definition->action_parameters[0],
                      definition->action_parameters[3])
                   != ACTOR_ACTION_NONE) {
        chosen = 0x13;
    } else if (definition->action_animations[0x14 - 11] != ACTOR_ACTION_NONE
               && actor_try_select_profiled_action(
                      0x14,
                      player_distance,
                      definition->action_parameters[1],
                      definition->action_parameters[4])
                   != ACTOR_ACTION_NONE) {
        chosen = 0x14;
    } else if (definition->action_animations[0x15 - 11] != ACTOR_ACTION_NONE
               && actor_try_select_profiled_action(
                      0x15,
                      player_distance,
                      definition->action_parameters[2],
                      definition->action_parameters[5])
                   != ACTOR_ACTION_NONE) {
        chosen = 0x15;
    } else if (definition->action_animations[KF_ACTOR_ACTION_INDEX(4)] != ACTOR_ACTION_NONE
               && actor_try_select_action_distance_facing(
                      4, player_distance, definition->unknown_00[2], definition->awareness_distance)
                   != ACTOR_ACTION_NONE) {
        chosen = 4;
    } else if (definition->action_animations[0x12 - 11] != ACTOR_ACTION_NONE) {
        chosen = 0x12;
    } else {
        recently_active = action == 3 || action == 4 || action == 16 || action == 17
            || action == 19 || action == 20 || action == 21;
        if (definition->action_animations[KF_ACTOR_ACTION_INDEX(3)] != ACTOR_ACTION_NONE) {
            if (recently_active || !(awareness < player_distance)) {
                if (!(awareness * 2 < player_distance) && !(rand() < 5462)) {
                    chosen = 3;
                    goto choose;
                }
            }
        }
        if (definition->action_animations[KF_ACTOR_ACTION_INDEX(3)] != ACTOR_ACTION_NONE) {
            if (recently_active || action == 5 || action == 2) {
                near_range = near_range * 6;
            }
            if (!(near_range < player_distance)) {
                chosen = 2;
                goto choose;
            }
        }
        if (definition->action_animations[KF_ACTOR_ACTION_INDEX(2)] != ACTOR_ACTION_NONE) {
            switch (action) {
            case 1:
                if (rand() < 1092) {
                    chosen = 0;
                    goto choose;
                }
                break;
            case 0:
                if (!(rand() < 8193)) {
                    chosen = 0;
                    goto choose;
                }
                break;
            }
            if (definition->action_animations[KF_ACTOR_ACTION_INDEX(3)] == ACTOR_ACTION_NONE) {
                chosen = 0;
                goto choose;
            }
        } else if (definition->action_animations[KF_ACTOR_ACTION_INDEX(3)] == ACTOR_ACTION_NONE) {
            goto choose;
        }
        chosen = 1;
        if (actor->slot_state == 3) {
            chosen = 0x21;
        }
    }
choose:
    if (chosen != actor->action || actor->action_timer == ACTOR_ACTION_NONE) {
        actor_set_action(actor, chosen);
    }
}

/*
 * Advances the current actor's lifecycle from the player's distance:
 * dormant actors wake within 28000 units (with a spawn roll and an
 * overlap check), active ones pick actions within 32000, and retreating
 * ones go dormant again once the player is out of range.
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
    case 0:
        distance = actor_distance_to_point(
            actor,
            actor_state.player_position.vx,
            0xffff,
            actor_state.player_position.vz,
            0x6d60,
            0,
            0);
        if (distance == -1) {
            return;
        }
        kind = slot_state;
        if (kind == 2) {
            if ((actor->unknown_07 << 7) > rand()) {
                if (actor_pool_find_overlap(
                        actor->tile_x * MAP_TILE_SIZE + actor->local_x,
                        0xffff,
                        actor->tile_z * MAP_TILE_SIZE + actor->local_z,
                        definition->collision_radius,
                        0)
                    == -1) {
                    actor_initialize_current();
                    if (definition->action_animations[KF_ACTOR_ACTION_INDEX(3)] != ACTOR_ACTION_NONE) {
                        actor_set_action(actor, 0x20);
                    } else {
                        actor_select_next_action(distance);
                    }
                }
            }
        } else {
            if (distance < 26000 && player_state.unknown_0d == 0) {
                actor->lifecycle = 2;
                return;
            }
            if ((actor->unknown_07 << 7) > rand() || kind == 1 || kind == 3) {
                if (actor_pool_find_overlap(
                        actor->tile_x * MAP_TILE_SIZE + actor->local_x,
                        0xffff,
                        actor->tile_z * MAP_TILE_SIZE + actor->local_z,
                        definition->collision_radius,
                        0)
                    != -1) {
                    actor->lifecycle = 2;
                    return;
                }
                actor_initialize_current();
                actor_select_next_action(distance);
            } else {
                actor->lifecycle = 2;
            }
        }
        break;
    case 1:
        distance = actor_distance_to_point(
            actor,
            actor_state.player_position.vx,
            0xffff,
            actor_state.player_position.vz,
            0x7d00,
            0,
            0);
        if (distance == -1) {
            actor->lifecycle = 0;
        } else {
            actor_select_next_action(distance);
        }
        break;
    case 2:
        if (actor_distance_to_point(
                actor,
                actor_state.player_position.vx,
                0xffff,
                actor_state.player_position.vz,
                0x7d00,
                0,
                0)
            == -1) {
            actor->lifecycle = 0;
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
        0x8040);
    if (result != -1 && (result != 0x1fff0 || actor->vertical_state == 3)) {
    blocked:
        if (stop_on_collision == 0) {
            if (actor->collision_state != 2) {
                if (collision_query_world(
                        actor->position.vx,
                        target.vy,
                        target.vz,
                        definition->collision_radius,
                        definition->collision_height,
                        0x8040)
                    == -1) {
                    if (delta->z >= 0) {
                        actor->movement_yaw = 0x800;
                    } else {
                        actor->movement_yaw = 0;
                    }
                    actor->position.vz = target.vz;
                    actor->cell_z = target.vz / MAP_TILE_SIZE;
                    actor->collision_state = 1;
                } else if (collision_query_world(
                               target.vx,
                               target.vy,
                               actor->position.vz,
                               definition->collision_radius,
                               definition->collision_height,
                               0x8040)
                           == -1) {
                    if (delta->x < 0) {
                        actor->movement_yaw = 0x400;
                    } else {
                        actor->movement_yaw = 0xc00;
                    }
                    actor->position.vx = target.vx;
                    actor->cell_x = target.vx / MAP_TILE_SIZE;
                    actor->collision_state = 1;
                } else {
                    actor->movement_yaw = (actor->movement_yaw + 0x800) & 0xfff;
                    actor->collision_state = 2;
                }
            } else if (actor->movement_yaw == actor->rotation.y) {
                actor->movement_yaw = (actor->movement_yaw + 64) & 0xfff;
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
            if (actor->vertical_state == 0) {
                actor->vertical_state = 3;
                actor->vertical_velocity = 0;
            }
        } else if (actor->vertical_state == 0) {
            actor->vertical_state = 2;
            actor->vertical_velocity = 0;
        }
    } else if (drop > 0) {
        if (!(drop < 1001)) {
            goto blocked;
        }
        if (drop >= 400) {
            if (actor->vertical_state == 0) {
                actor->vertical_state = 1;
                actor->vertical_velocity = -300;
            }
        } else if (actor->vertical_state == 0) {
            actor->vertical_state = 1;
            actor->vertical_velocity = -120;
        }
    }
    actor->collision_state = 0;
    actor->position.vx = target.vx;
    actor->position.vz = target.vz;
    actor->cell_x = target.vx / MAP_TILE_SIZE;
    actor->cell_z = target.vz / MAP_TILE_SIZE;
    return 0;
}


ADDRESS(0x8002ed00, 0xd4)
s32 actor_move_along_heading(s32 direction, s32 stop_on_collision)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    struct KfVecXZs delta;
    u32 rate;

    if (actor->collision_state == 1) {
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
    struct KfVec3s direction;
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
    if (effect_code & 0x20) {
        repeat = 2;
    }
    effect_code &= 0x1f;
    for (i = 0; i < repeat; i++) {
        switch (effect_code) {
        case 5:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
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
            angles.y = -actor->rotation.y & 0xfff;
            angles.z = actor->rotation.z;
            matrix_set_rotation_yxz(&angles, &matrix);
            ApplyMatrix(&matrix, &offset, &position);
            position.vx += actor->position.vx;
            position.vy += actor->position.vy;
            position.vz += actor->position.vz;
            facing = (0x800 - actor->rotation.y) & 0xfff;
            distance = player_distance_to_point_in_cone(
                (struct KfVec3i *)&position, facing, 50000, 0x155);
            if (distance == -1) {
                angles.x = 0;
                if (effect_code == 23) {
                    scale = 800;
                    angles.x = -32;
                    distance = 20;
                } else if (effect_code == 9 || effect_code == 10) {
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
                    if (effect_code == 10) {
                        scale = 250;
                        distance -= 4500;
                    /* Retail shares one step-count clamp between codes 10 and 9. */
                    clamp_steps:
                        if (distance <= scale) {
                            distance = 1;
                        } else {
                            distance = distance / scale;
                        }
                    } else if (effect_code == 9) {
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
                    definition->unknown_82, 0x23, effect_code, &position, &direction, &angles, 1);
            } else if (effect_code == 24) {
                burst_angles.x = actor->rotation.x;
                burst_angles.y = facing;
                burst_angles.z = actor->rotation.z;
                effect_pool_construct(
                    definition->unknown_82, 0x23, 24, &position, &direction, &burst_angles, 0xfe, 1);
            } else if (effect_code == 10) {
                effect_pool_construct(
                    definition->unknown_82, 0x23, 10, &position, &direction, 3, distance, 0xbb8);
            } else {
                effect_pool_construct(
                    definition->unknown_82, 0x23, effect_code, &position, &direction, distance, 1);
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
    if (angle_within_tolerance(actor->rotation.y, (s16)actor->movement_yaw, 0x155) == 0) {
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
        0x8060);
    if (result != -1) {
        if ((result >> 16) != 0x80) {
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
    actor->cell_x = actor->position.vx / 2000;
    actor->cell_z = actor->position.vz / 2000;
}

/* Runs effect action ACTION (0..7): starts its animation, spawns its effect once, then picks the next action. */
ADDRESS(0x8002f468, 0xf0)
void actor_update_effect_action(s32 action)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    s32 index = action + 8;

    if (actor->action_timer == 0) {
        actor->action_timer = 0xf0;
        actor->animation_id = definition->action_animations[index];
        actor->animation_phase = 0;
    }
    actor_advance_animation_clamped(actor, definition->action_animation_steps[index]);
    if (actor_animation_crossed_phase(actor, definition->action_animation_phases[index])) {
        actor_spawn_action_effect(definition->action_parameters[action], action);
    }
    if (actor->animation_phase >= 4095) {
        actor->action_timer = 0xff;
        actor_select_next_action(actor_distance_to_point(
            actor,
            actor_state.player_position.vx,
            0xffff,
            actor_state.player_position.vz,
            0x7d00,
            0,
            0));
    }
}

/*
 * Nudges each velocity component by STEP in a random direction, clamped to
 * LIMIT, then applies the whole vector; a blocked move sets a short action
 * timer and reflects each axis that collides on its own.
 */
ADDRESS(0x8002f558, 0x374)
void actor_apply_random_movement(s16 step, s16 limit)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    VECTOR target;
    s32 result;

    if (rand() < 16384) {
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
    if (rand() < 16384) {
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
    if (rand() < 16384) {
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
        0x8060);
    if (result == -1) {
        actor->position = target;
    } else {
        actor->action_timer = 2;
        result = collision_query_world(
            target.vx,
            actor->position.vy,
            actor->position.vz,
            definition->collision_radius,
            definition->collision_height,
            0x8060);
        if (result != -1) {
            actor->movement_x = -actor->movement_x;
        } else if (collision_query_world(
                       actor->position.vx,
                       target.vy,
                       actor->position.vz,
                       definition->collision_radius,
                       definition->collision_height,
                       0x8060)
                   != -1) {
            actor->movement_y = -actor->movement_y;
        }
        if (collision_query_world(
                actor->position.vx,
                actor->position.vy,
                target.vz,
                definition->collision_radius,
                definition->collision_height,
                0x8060)
            != -1) {
            actor->movement_z = -actor->movement_z;
        }
    }
    actor->cell_x = actor->position.vx / 2000;
    actor->cell_z = actor->position.vz / 2000;
}


ADDRESS(0x8002f8cc, 0x1bc)
void actor_update_boss_death_sequence(void)
{
    KfActor *actor = actor_state.current;
    KfActorDefinition *definition = actor_state.current_definition;
    u32 effect_output[2];
    struct KfVec3i position;

    actor_play_sound_at_phase(&boss_death_phase_sounds[1], 500);
    actor_play_sound_at_phase(&boss_death_phase_sounds[2], 1000);
    actor_play_sound_at_phase(&boss_death_phase_sounds[3], 1500);
    if (actor->animation_phase >= 4095) {
        actor->animation_phase = 0xfff;
        actor->action_timer = 0;
        actor->lifecycle = 3;
        boss_defeat_complete = 1;
        map_object_pool_trigger_link(13);
        actor_pool_begin_death_by_definition(0);
        actor_pool_begin_death_by_definition(2);
        actor_pool_begin_death_by_definition(3);
        actor_pool_begin_death_by_definition(4);
    }
    if (actor->animation_phase % (definition->action_animation_steps[KF_ACTOR_ACTION_INDEX(6)] * 2) == 0) {
        position.x = actor->position.vx + (rand() & 0x1fff) - 4096;
        position.z = actor->position.vz + (rand() & 0x1fff) - 4096;
        position.y = actor->position.vy - (rand() & 0xfff);
        effect_pool_construct(0, 0x13, 0x2c, &position, effect_output, 0);
        if (actor->animation_phase % (definition->action_animation_steps[KF_ACTOR_ACTION_INDEX(6)] * 4) == 0) {
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
    case 0:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            actor->animation_id = definition->action_animations[0];
            actor->animation_phase = 0;
        }
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[0]);
        break;
    case 1:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            if (actor->animation_id != definition->action_animations[1]) {
                actor->animation_id = definition->action_animations[1];
                actor->animation_phase = 0;
            }
            actor->movement_yaw = rand() >> 3;
        } else if (actor->collision_state == 0 && rand() < 2048) {
            actor->movement_yaw = rand() >> 3;
        }
        actor_move_along_heading(1, 0);
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[1]);
        break;
    case 2:
        switch (actor->action_timer) {
        case 0:
            actor->action_timer = 1;
            if (actor->animation_id != definition->action_animations[1]) {
                actor->animation_id = definition->action_animations[1];
                actor->animation_phase = 0;
            }
            actor->movement_yaw = vector_xz_to_angle(
                actor_state.player_position.vx - actor->position.vx,
                actor_state.player_position.vz - actor->position.vz);
            break;
        case 1:
            if (actor_move_along_heading(1, 1) != 0) {
                actor->action_timer = (rand() >> 11) + 13;
                goto vertical;
            }
            if (rand() < 4096) {
                actor->movement_yaw = vector_xz_to_angle(
                    actor_state.player_position.vx - actor->position.vx,
                    actor_state.player_position.vz - actor->position.vz);
            }
            break;
        default:
            if (actor_move_along_heading(-1, 1) != 0 || actor->action_timer < 11) {
                actor->action_timer = 1;
            } else {
                actor->action_timer--;
            }
            break;
        }
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[1]);
        break;
    case 5:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            actor->animation_id = definition->action_animations[3];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[3]);
        actor_play_sound_at_phase(&definition->sounds[1], definition->action_animation_phases[3]);
        if (actor->animation_phase >= 4095) {
            actor->action_timer = 0xff;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.vx,
                0xffff,
                actor_state.player_position.vz,
                32000,
                0,
                0));
        }
        break;
    case 6:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            actor->animation_id = definition->action_animations[4];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[4]);
        actor_play_sound_at_phase(&definition->sounds[2], definition->action_animation_phases[4]);
        if (player_state.progress_state.current_floor == 5 && actor->definition_id == 7) {
            actor_update_boss_death_sequence();
            return;
        }
        if (actor_animation_crossed_phase(actor, 0x800)) {
            debris = ((u32)rand() * definition->unknown_96) >> 15;
            if (debris != 0) {
                map_object_spawn_actor_debris(
                    debris, (struct KfVec3i *)&actor->position, -(definition->collision_height >> 1));
            }
            if (actor->slot_state == 0 || actor->slot_state == 2) {
                if (definition->action_parameters[6] != 0x63 && definition->action_parameters[6] != 0xff
                    && (rand() >> 7) <= definition->action_parameters[7]) {
                    map_object_spawn_effect(
                        1,
                        definition->action_parameters[6],
                        (struct KfVec3i *)&actor->position,
                        -(definition->collision_height >> 1));
                }
            } else if (actor->unknown_09 != 0x63) {
                map_object_spawn_effect(
                    0,
                    actor->unknown_09,
                    (struct KfVec3i *)&actor->position,
                    -(definition->collision_height >> 1));
            }
        }
        if (actor->animation_phase >= 4095) {
            actor->action = 0x7f;
            actor->animation_phase = 0xfff;
            actor->action_timer = 0;
        }
        break;
    case 3:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            if (actor->animation_id != definition->action_animations[1]) {
                actor->animation_id = definition->action_animations[1];
                actor->animation_phase = 0;
            }
            actor->movement_yaw = vector_xz_to_angle(
                actor_state.player_position.vx - actor->position.vx,
                actor_state.player_position.vz - actor->position.vz);
        }
        if (actor_move_along_heading(-1, 1) != 0) {
            actor->action = 0xff;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.vx,
                0xffff,
                actor_state.player_position.vz,
                32000,
                0,
                0));
        } else {
            actor_advance_animation_wrapped(actor, definition->action_animation_steps[1]);
        }
        break;
    case 4:
        if (actor->action_timer == 0) {
            actor->action_timer = 0xf0;
            actor->animation_id = definition->action_animations[2];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[2]);
        actor_play_sound_at_phase(&definition->sounds[0], definition->action_animation_phases[2]);
        if (actor->animation_phase >= 2700
            && actor->animation_phase < definition->action_animation_steps[2] + 2700) {
            actor_try_attack_player(0, definition->awareness_distance, 0, 0x155);
        }
        if (actor->animation_phase >= 4095) {
            actor->action_timer = 0xff;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.vx,
                0xffff,
                actor_state.player_position.vz,
                32000,
                0,
                0));
        }
        break;
    case 127:
        if (player_state.progress_state.current_floor == 4 && actor->definition_id == 5) {
            func_80036d3c(actor);
            actor_initialize(actor);
            return;
        }
        if (actor->action_timer >= 7) {
            if (actor->slot_state == 2) {
                actor->lifecycle = 0;
            } else if (actor->slot_state == 1 || actor->slot_state == 3) {
                actor->lifecycle = 3;
            } else {
                actor->lifecycle = 2;
            }
            return;
        }
        actor->action_timer++;
        break;
    case 16:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            actor->animation_id = definition->action_animations[5];
            actor->animation_phase = 0;
            attribute = map_cell_attribute_grid[actor->cell_z][actor->cell_x];
            if (map_cell_attribute_height_table[attribute - 1] > -5000) {
                actor->vertical_state = 4;
                actor->vertical_velocity = -220;
                actor->animation_step = 140;
            } else {
                actor->vertical_state = 4;
                actor->vertical_velocity = -300;
                actor->animation_step = 110;
            }
            actor_prepare_charge_toward_player();
        }
        actor_apply_horizontal_movement();
        actor->animation_phase += actor->animation_step;
        if (actor->animation_phase >= 4095) {
            actor->animation_phase = 0xfff;
        }
        actor_play_sound_at_phase(&definition->sounds[0], definition->action_animation_phases[5]);
        switch (actor->action_timer) {
        case 1:
            if (actor->vertical_velocity >= 0) {
                actor_prepare_charge_toward_player();
                actor->action_timer = 2;
            }
            break;
        case 2:
            if (actor_animation_crossed_phase(actor, 0xd48)) {
                actor_try_attack_player(0, definition->unknown_36, 0, 0x155);
                actor->action_timer = 3;
            }
            break;
        case 3:
            if (actor->vertical_state != 4) {
                actor->vertical_state = 0;
                actor->vertical_velocity = 0;
                result = collision_query_world(
                    actor->position.vx,
                    actor->position.vy,
                    actor->position.vz,
                    definition->collision_radius,
                    definition->collision_height,
                    0x8060);
                if (result != -1 && (result >> 16) == 0xfff1) {
                    actor->vertical_state = 2;
                    actor->vertical_velocity = 0;
                }
                actor->action_timer = 0xff;
                actor_select_next_action(actor_distance_to_point(
                    actor,
                    actor_state.player_position.vx,
                    0xffff,
                    actor_state.player_position.vz,
                    32000,
                    0,
                    0));
            }
            break;
        }
        break;
    case 17:
        if (actor->action_timer == 0) {
            actor->action_timer = 0xf0;
            actor->animation_id = definition->action_animations[6];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[6]);
        actor_play_sound_at_phase(&definition->sounds[0], definition->action_animation_phases[6]);
        if (actor->animation_phase >= 3000
            && actor->animation_phase < definition->action_animation_steps[6] + 3000) {
            actor_try_attack_player(0, definition->unknown_36, 0, 0x155);
        }
        if (actor->animation_phase >= 4095) {
            actor->action_timer = 0xff;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.vx,
                0xffff,
                actor_state.player_position.vz,
                32000,
                0,
                0));
        }
        break;
    case 32:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            actor->animation_id = definition->action_animations[1];
            actor->animation_phase = 0;
        }
        result = collision_query_world(
            actor->position.vx, 0xffff, actor->position.vz, definition->collision_radius, 0, 0x8040);
        if (result != -1) {
            target.x = actor->position.vx;
            target.z = actor->position.vz;
            angle_to_forward_xz(actor->rotation.y, &direction);
            vector2s_scale_shift11(definition->move_speed, &direction);
            vector3i_add_xz(&target, &direction);
            if (actor_pool_find_overlap(target.x, 0xffff, target.z, definition->collision_radius, 0)
                == -1) {
                actor->position.vx = target.x;
                actor->position.vz = target.z;
            }
        } else {
            actor->action_timer = 0xff;
            actor_select_next_action(32000);
        }
        break;
    case 18:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            actor->animation_id = definition->action_animations[7];
            actor->animation_phase = 0;
            actor->movement_y = 0;
            actor->movement_z = 0;
            actor->movement_x = 0;
            actor->movement_yaw = 0;
            actor->vertical_state = 0;
            actor->vertical_velocity = 0;
        }
        actor_apply_random_movement(1, 100);
        if (rand() < 16384) {
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
        actor->rotation.y = (actor->rotation.y + actor->movement_yaw) & 0xfff;
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[7]);
        break;
    case 19:
        actor_update_effect_action(0);
        break;
    case 20:
        actor_update_effect_action(1);
        break;
    case 21:
        actor_update_effect_action(2);
        break;
    case 33:
        if (actor->action_timer == 0) {
            actor->action_timer = 1;
            if (actor->animation_id != definition->action_animations[1]) {
                actor->animation_id = definition->action_animations[1];
                actor->animation_phase = 0;
            }
            /* Retail reads the home position before this branch assigns it. */
            actor->movement_yaw =
                vector_xz_to_angle(home_x - actor->position.vx, home_z - actor->position.vz);
        } else if (actor->collision_state == 0) {
            home_x = actor->tile_x * 2000 + actor->local_x;
            home_z = actor->tile_z * 2000 + actor->local_z;
            if (actor->position.vx - home_x > -200 && actor->position.vx - home_x < 200
                && actor->position.vz - home_z > -200 && actor->position.vz - home_z < 200) {
                actor->movement_yaw = actor->heading_quadrant << 10;
                actor->rotation.y = angle_approach(
                    actor->rotation.y, actor->movement_yaw, definition->turn_rate);
                if (actor->animation_id != definition->action_animations[2]) {
                    break;
                }
                if (actor->movement_yaw == actor->rotation.y
                    && actor_animation_crossed_phase(actor, 0x4b0)) {
                    actor->animation_phase = 0;
                    actor->animation_id = definition->action_animations[2];
                    break;
                }
                actor_advance_animation_wrapped(actor, definition->action_animation_steps[1]);
                break;
            }
            if (rand() < 2048) {
                actor->movement_yaw = vector_xz_to_angle(
                    home_x - actor->position.vx, home_z - actor->position.vz);
            }
        }
        actor_move_along_heading(1, 0);
        actor_advance_animation_wrapped(actor, definition->action_animation_steps[1]);
        break;
    case 22:
        if (actor->action_timer == 0) {
            actor->action_timer = 0xf0;
            actor->animation_id = definition->action_animations[11];
            actor->animation_phase = 0;
        }
        actor_advance_animation_clamped(actor, definition->action_animation_steps[11]);
        actor_play_sound_at_phase(&definition->sounds[0], definition->action_animation_phases[11]);
        if (actor_animation_crossed_phase(actor, 0x8c0) || actor_animation_crossed_phase(actor, 0xa80)
            || actor_animation_crossed_phase(actor, 0xc80)
            || actor_animation_crossed_phase(actor, 0xe00)) {
            actor_try_attack_player(8000, 11000, 0, 0x155);
        }
        if (actor_animation_crossed_phase(actor, 0xdac)) {
            actor_try_attack_player(0, 11000, -512, 0x155);
        }
        if (actor_animation_crossed_phase(actor, 0xed8)) {
            actor_try_attack_player(0, 11000, 512, 0x155);
        }
        if (actor->animation_phase >= 4095) {
            actor->action_timer = 0xff;
            actor_select_next_action(actor_distance_to_point(
                actor,
                actor_state.player_position.vx,
                0xffff,
                actor_state.player_position.vz,
                32000,
                0,
                0));
        }
        break;
    }
vertical:
    switch (actor->vertical_state) {
    case 0:
        break;
    case 1:
        floor_height = map_floor_height_at_position(&actor->position);
        next_y = actor->vertical_velocity + actor->position.vy;
        if (next_y > floor_height) {
            goto fall;
        }
    land:
        actor->position.vy = floor_height;
        actor->vertical_state = 0;
        actor->vertical_velocity = 0;
        break;
    fall:
        actor->position.vy = next_y;
        actor->vertical_velocity += 20;
        break;
    case 2:
    case 3:
        floor_height = map_floor_height_at_position(&actor->position);
        next_y = actor->vertical_velocity + actor->position.vy;
        if (next_y >= floor_height) {
            goto land;
        }
        goto fall;
    case 4:
        next_y = actor->vertical_velocity + actor->position.vy;
        hit = collision_query_world(
            actor->position.vx,
            next_y,
            actor->position.vz,
            definition->collision_radius,
            definition->collision_height,
            0x8060);
        if (hit == -1) {
            goto fall;
        }
        if ((hit >> 16) == 0x80) {
            player_apply_damage(0, 15, 0, 0, 0, 0, 0x1000, 10);
        stagger:
            actor->vertical_state = 4;
            actor->vertical_velocity = -120;
            actor->animation_phase = 0;
        } else if ((hit >> 16) == 1) {
            switch (hit & 0xffff) {
            case 0xfff0:
                floor_height = map_floor_height_at_position(&actor->position);
                goto land;
            case 0xfff1:
                actor->vertical_state = 4;
                actor->vertical_velocity = 100;
                break;
            }
        } else if ((hit >> 16) == 0x10) {
            goto stagger;
        }
        break;
    }
    collision_adjust_cell_occupancy(actor->cell_x, actor->cell_z, 1);
}
