#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfActorState actor_state;
extern KfPlayerState player_state;

/* Psy-Q LIBC: int rand(void). */
extern s32 rand(void);
extern void actor_set_action(KfActor *actor, u8 action);
extern void actor_initialize_current(void);
extern s32 actor_distance_to_point(
    const KfActor *actor,
    s32 point_x,
    s32 point_y,
    s32 point_z,
    s32 max_distance,
    s32 actor_height,
    s32 point_height);
extern s32 actor_pool_find_overlap(s32 x, s32 y, s32 z, s32 extra_radius, s32 point_height);
extern u8 actor_try_select_facing_action(u8 action, s32 distance, u16 chance);
extern u8 actor_try_select_ground_action(u8 action, s32 distance, u16 chance);
extern u8 actor_try_select_action_distance_facing(
    u8 action, s32 distance, u16 chance, u16 distance_scale);
extern u8 actor_try_select_profiled_action(
    u8 action, s32 distance, u8 profile_index, u16 chance);
extern void vector3i_add_xz(VECTOR *destination, const struct KfVecXZs *delta);
extern u32 collision_query_world(
    s32 point_x, s32 point_y, s32 point_z, s32 radius, s32 height, u32 flags);
extern s32 map_floor_height_at_position(struct KfVec3i *position);

#define ACTOR_ACTION_NONE 0xff
#define MAP_TILE_SIZE 2000

/*
 * Picks the current actor's next action from its definition's per-action
 * tables and the distance to the player.  Special actions 0x10..0x16 use
 * table entries action - 11; the basic actions use action - 2.
 */
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
    vector3i_add_xz(&target, delta);
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
    drop = target.vy - map_floor_height_at_position((struct KfVec3i *)&target);
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
