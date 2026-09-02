#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfActorState actor_state;

/* Psy-Q LIBC: int rand(void). */
extern s32 rand(void);
extern s32 vector_xz_to_angle(s32 x, s32 z);
extern s32 angle_within_tolerance(s32 angle, s32 target, s16 tolerance);
extern s32 fixed_vector2_length(s32 x, s32 y);
extern void angle_to_forward_xz(s16 angle, struct KfVecXZs *direction);
extern void vector2s_scale_shift11(s16 scale, s16 *vector);
extern u32 collision_query_world(
    s32 point_x, s32 point_y, s32 point_z, s32 radius, s32 height, u32 flags);
extern void actor_advance_animation_clamped(KfActor *actor, s16 delta);
extern s32 actor_animation_crossed_phase(const KfActor *actor, u16 phase);
extern void actor_spawn_action_effect(u8 effect_code, s32 attachment_index);
extern s32 actor_distance_to_point(const KfActor *actor, s32 point_x, s32 point_y, s32 point_z, s32 max_distance, s32 actor_height, s32 point_height);
extern void actor_select_next_action(s32 player_distance);

/* Aims the current actor at the player and sets a charge velocity from the distance. */
ADDRESS(0x8002f228, 0xf4)
void actor_prepare_charge_toward_player(void)
{
    KfActor *actor = actor_state.current;
    struct KfVecXZs delta;
    s32 length;

    actor->movement_yaw = vector_xz_to_angle(
        actor_state.player_position.x - actor->position.x,
        actor_state.player_position.z - actor->position.z);
    if (angle_within_tolerance(actor->rotation.y, (s16)actor->movement_yaw, 0x155) == 0) {
        actor->movement_yaw = actor->rotation.y;
    }
    length = fixed_vector2_length(
        actor_state.player_position.x - actor->position.x,
        actor_state.player_position.z - actor->position.z);
    angle_to_forward_xz(actor->movement_yaw, &delta);
    vector2s_scale_shift11(length >> 2, (s16 *)&delta);
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

    target.x = actor->movement_x + actor->position.x;
    target.z = actor->movement_z + actor->position.z;
    definition = actor_state.current_definition;
    result = collision_query_world(
        target.x,
        actor->position.y,
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
        actor->position.x = target.x;
        actor->position.z = target.z;
    }
    actor->cell_x = actor->position.x / 2000;
    actor->cell_z = actor->position.z / 2000;
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
            actor_state.player_position.x,
            0xffff,
            actor_state.player_position.z,
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
    struct KfVec4i target;
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
    target.x = actor->movement_x + actor->position.x;
    target.z = actor->movement_z + actor->position.z;
    target.y = actor->movement_y + actor->position.y;
    result = collision_query_world(
        target.x,
        target.y,
        target.z,
        definition->collision_radius,
        definition->collision_height,
        0x8060);
    if (result == -1) {
        actor->position = target;
    } else {
        actor->action_timer = 2;
        result = collision_query_world(
            target.x,
            actor->position.y,
            actor->position.z,
            definition->collision_radius,
            definition->collision_height,
            0x8060);
        if (result != -1) {
            actor->movement_x = -actor->movement_x;
        } else if (collision_query_world(
                       actor->position.x,
                       target.y,
                       actor->position.z,
                       definition->collision_radius,
                       definition->collision_height,
                       0x8060)
                   != -1) {
            actor->movement_y = -actor->movement_y;
        }
        if (collision_query_world(
                actor->position.x,
                actor->position.y,
                target.z,
                definition->collision_radius,
                definition->collision_height,
                0x8060)
            != -1) {
            actor->movement_z = -actor->movement_z;
        }
    }
    actor->cell_x = actor->position.x / 2000;
    actor->cell_z = actor->position.z / 2000;
}
