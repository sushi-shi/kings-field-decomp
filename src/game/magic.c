#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_collision.h>
#include <kf/game_player.h>
#include <kf/game.h>
#include <kf/magic.h>

ADDRESS(0x8003a244, 0x30)
void effect_pool_reset(void)
{
    KfEffectRecord *record = effect_pool_records;
    u16 i;

    for (i = 0; i < KF_EFFECT_CAPACITY; i++) {
        record->type = KF_EFFECT_SLOT_FREE;
        record++;
    }
}

ADDRESS(0x8003a274, 0x2c)
void magic_load_records(const u32 *source)
{
    u32 *destination = (u32 *)magic_records;
    s32 count;

    for (count = sizeof effect_state.magic / sizeof *source; count != 0; count--) {
        *destination++ = *source++;
    }
}

/* magic_cast dispatch table (selected_magic_id 4..8). */
RODATA(0x80012dc0, 0x14)

/* Spawns the player's ranged magic effect for the current spell
 * (selected_magic_id 4..8), aiming at a target actor in the view cone. */
ADDRESS(0x8003a2a0, 0x4c0)
void magic_cast(void)
{
    switch (player_state.selected_magic_id) {
    case KF_MAGIC_LIGHTNING_BOLT:
    case KF_MAGIC_FIRE_BALL:
    case KF_MAGIC_WIND_CUTTER:
    case KF_MAGIC_LIGHT_NEEDLE: {
        SVECTOR direction;
        SVECTOR offset;
        struct KfEulerAngles angles;
        VECTOR world_pos;
        MATRIX matrix;
        s32 distance;
        KfActor *target;
        s32 scale;

        offset.vx = -200;
        offset.vy = 200;
        offset.vz = 400;
        angles.x = -player_state.camera_rotation.vx;
        angles.y = player_state.camera_rotation.vy;
        angles.z = -player_state.camera_rotation.vz;
        matrix_set_rotation_yxz(&angles, &matrix);
        ApplyMatrix(&matrix, &offset, &world_pos);
        world_pos.vx += player_state.camera_position.vx;
        world_pos.vy += player_state.camera_position.vy;
        world_pos.vz += player_state.camera_position.vz;
        target = actor_pool_find_target_in_cone(
            (struct KfVec3i *)&player_state.camera_position,
            player_state.camera_rotation.vy, 0x4e20, KF_ACTOR_AIM_TOLERANCE, &distance);
        actor_state.player_target = target;
        if (target == 0) {
            scale = 600;
            if (player_state.selected_magic_id == KF_MAGIC_LIGHTNING_BOLT) {
                scale = 800;
                angles.x = -128;
                distance = 20;
            } else {
                angles.x = player_state.camera_rotation.vx;
            }
        } else {
            scale = 600;
            if (player_state.selected_magic_id == KF_MAGIC_LIGHTNING_BOLT) {
                if (map_cell_attribute_height_table[
                        map_cell_attribute_grid[target->cell_z][target->cell_x] - 1]
                        >= -4999) {
                    angles.x = vector_xz_to_angle(
                        world_pos.vy + 3000 - target->position.vy, -distance);
                } else {
                    angles.x = vector_xz_to_angle(
                        world_pos.vy + 5000 - target->position.vy, -distance);
                }
                scale = 800;
                distance = distance / scale;
            } else {
                angles.x = player_state.camera_rotation.vx;
            }
        }
        if (player_state.selected_magic_id == KF_MAGIC_WIND_CUTTER) {
            scale = 800;
        }
        angles.y = player_state.camera_rotation.vy;
        angles.z = player_state.camera_rotation.vz;
        pitch_yaw_to_forward_vector((struct KfPitchYaw *)&angles, &direction);
        vector3s_scale_shift12(scale, &direction);
        if (player_state.selected_magic_id == KF_MAGIC_LIGHT_NEEDLE) {
            SVECTOR rotation;

            rotation.vx = player_state.camera_rotation.vx;
            rotation.vy = player_state.camera_rotation.vy;
            rotation.vz = player_state.camera_rotation.vz;
            effect_pool_construct(
                0xa, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS,
                KF_ENUM_ENCODE(u8, player_state.selected_magic_id), &world_pos, &direction, &rotation, 1);
        } else {
            effect_pool_construct(
                0xa, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS,
                KF_ENUM_ENCODE(u8, player_state.selected_magic_id), &world_pos, &direction, distance, 1);
        }
        break;
    }
    case KF_MAGIC_FIRE_WALL: {
        s32 distance;
        KfActor *target;

        target = actor_pool_find_target_in_cone(
            (struct KfVec3i *)&player_state.camera_position,
            player_state.camera_rotation.vy, 0x4e20, KF_ACTOR_AIM_TOLERANCE, &distance);
        if (target != 0) {
            effect_pool_construct(
                0xa, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                KF_ENUM_ENCODE(u8, player_state.selected_magic_id), &target->position,
                (SVECTOR *)&player_state.camera_rotation, KF_EFFECT_GROUND_BRANCH_ROOT);
        } else {
            VECTOR spawn;
            s32 cell_x;
            s32 cell_z;

            spawn.vx = player_state.camera_position.vx
                       - (rsin(player_state.camera_rotation.vy) * 6000 >> KF_FIXED12_BITS);
            spawn.vz = player_state.camera_position.vz
                       + (rcos(player_state.camera_rotation.vy) * 6000 >> KF_FIXED12_BITS);
            cell_z = spawn.vz / KF_MAP_TILE_SIZE;
            cell_x = spawn.vx / KF_MAP_TILE_SIZE;
            spawn.vy = -(map_floor_height_grid[cell_z][cell_x] * KF_MAP_HEIGHT_STEP);
            effect_pool_construct(
                0xa, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                KF_ENUM_ENCODE(u8, player_state.selected_magic_id), &spawn, (SVECTOR *)&player_state.camera_rotation,
                KF_EFFECT_GROUND_BRANCH_ROOT);
        }
        break;
    }
    }
}

ADDRESS(0x8003a760, 0x7c)
void effect_pool_sweep(void)
{
    KfEffectRecord *record = effect_pool_records;
    u16 i = KF_EFFECT_CAPACITY - 1;

    do {
        if (record->type != KF_EFFECT_SLOT_FREE) {
            effect_pool_set_current(record);
            effect_update_dispatch();
        }
        record++;
    } while (i-- != 0);
}
