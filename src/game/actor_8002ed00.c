#include <kf/address.h>
#include <kf/semantic_types.h>

RODATA(0x800124d4, 0x50)

extern KfActorState actor_state;

extern s16 angle_approach(s16 current, s16 target, s32 step);
extern void matrix_set_rotation_yxz(const struct KfEulerAngles *angles, MATRIX *matrix);
extern s32 player_distance_to_point_in_cone(
    const struct KfVec3i *point, s16 facing, s32 max_distance, s32 angle_tolerance);
extern s32 vector_xz_to_angle(s32 x, s32 z);
extern void pitch_yaw_to_forward_vector(const struct KfPitchYaw *angles, struct KfVec3s *direction);
extern void vector3s_scale_shift12(s16 scale, s16 *vector);
/* Effect spawner called with six to eight arguments; declared without a prototype. */
extern u8 *effect_pool_construct();
extern void angle_to_forward_xz(s16 angle, struct KfVecXZs *direction);
extern void vector2s_scale_shift11(s16 scale, s16 *vector);
extern s32 actor_move_xz_with_collision(const struct KfVecXZs *delta, s32 stop_on_collision);

/* Turns the current actor toward its movement yaw and steps along it; a negative DIRECTION walks backwards. */
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
    vector2s_scale_shift11(definition->move_speed, (s16 *)&delta);
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
            vector3s_scale_shift12(scale, (s16 *)&direction);
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
