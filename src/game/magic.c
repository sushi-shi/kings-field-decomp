#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_collision.h>
#include <kf/game_player.h>
#include <kf/game.h>
#include <kf/magic.h>

enum {
    MAGIC_LAUNCH_OFFSET_X = -200,
    MAGIC_LAUNCH_OFFSET_Y = 200,
    MAGIC_LAUNCH_OFFSET_Z = 400,
    MAGIC_TARGET_MAX_DISTANCE = 20000,
    MAGIC_DEFAULT_SPEED = 600,
    LIGHTNING_SPEED = 800,
    WIND_CUTTER_SPEED = 800,
    LIGHTNING_UNTARGETED_PITCH = -128,
    LIGHTNING_UNTARGETED_UPDATES = 20,
    LIGHTNING_HEIGHT_CLASS_THRESHOLD = -4999,
    LIGHTNING_DEFAULT_TARGET_Y_OFFSET = 3000,
    LIGHTNING_LOWER_HEIGHT_Y_OFFSET = 5000,
    FIRE_WALL_UNTARGETED_DISTANCE = 3 * KF_MAP_TILE_SIZE
};

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
void magic_load_records(const KfMagicTable *table)
{
    const u32 *source = table->words;
    u32 *destination = effect_state.magic.words;
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
        s32 speed;

        offset.vx = MAGIC_LAUNCH_OFFSET_X;
        offset.vy = MAGIC_LAUNCH_OFFSET_Y;
        offset.vz = MAGIC_LAUNCH_OFFSET_Z;
        angles.x = -player_state.camera_rotation.vx;
        angles.y = player_state.camera_rotation.vy;
        angles.z = -player_state.camera_rotation.vz;
        matrix_set_rotation_yxz(&angles, &matrix);
        ApplyMatrix(&matrix, &offset, &world_pos);
        world_pos.vx += player_state.camera_position.vx;
        world_pos.vy += player_state.camera_position.vy;
        world_pos.vz += player_state.camera_position.vz;
        target = actor_pool_find_target_in_cone(
            &player_state.camera_position,
            player_state.camera_rotation.vy, MAGIC_TARGET_MAX_DISTANCE, KF_ACTOR_AIM_TOLERANCE, &distance);
        actor_state.player_target = target;
        if (target == 0) {
            speed = MAGIC_DEFAULT_SPEED;
            if (player_state.selected_magic_id == KF_MAGIC_LIGHTNING_BOLT) {
                speed = LIGHTNING_SPEED;
                angles.x = LIGHTNING_UNTARGETED_PITCH;
                distance = LIGHTNING_UNTARGETED_UPDATES;
            } else {
                angles.x = 0;
                angles.x += player_state.camera_rotation.vx;
            }
        } else {
            speed = MAGIC_DEFAULT_SPEED;
            if (player_state.selected_magic_id == KF_MAGIC_LIGHTNING_BOLT) {
                if (map_cell_attribute_height_table[
                        map_cell_attribute_grid.cells[target->cell_z][target->cell_x] - 1]
                        >= LIGHTNING_HEIGHT_CLASS_THRESHOLD) {
                    s32 aim_y = world_pos.vy + LIGHTNING_DEFAULT_TARGET_Y_OFFSET;
                    angles.x = vector_xz_to_angle(aim_y - target->position.vy, -distance);
                } else {
                    s32 aim_y = world_pos.vy + LIGHTNING_LOWER_HEIGHT_Y_OFFSET;
                    angles.x = vector_xz_to_angle(aim_y - target->position.vy, -distance);
                }
                speed = LIGHTNING_SPEED;
                distance = distance / speed;
            } else {
                angles.x = player_state.camera_rotation.vx;
            }
        }
        if (player_state.selected_magic_id == KF_MAGIC_WIND_CUTTER) {
            speed = WIND_CUTTER_SPEED;
        }
        angles.y = player_state.camera_rotation.vy;
        angles.z = player_state.camera_rotation.vz;
        pitch_yaw_to_forward_vector(&angles, &direction);
        vector3s_scale_shift12(speed, &direction);
        if (player_state.selected_magic_id == KF_MAGIC_LIGHT_NEEDLE) {
            SVECTOR rotation;

            rotation.vx = player_state.camera_rotation.vx;
            rotation.vy = player_state.camera_rotation.vy;
            rotation.vz = player_state.camera_rotation.vz;
            effect_pool_construct(
                KF_PLAYER_DAMAGE_MULTIPLIER_ONE, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS,
                KF_ENUM_DECODE(KfEffectKind, KF_ENUM_ENCODE(u8, player_state.selected_magic_id)), &world_pos, &direction, KF_EFFECT_ARGS_ROTATION_SOUND(&rotation, KF_EFFECT_SOUND_PLAY));
        } else {
            effect_pool_construct(
                KF_PLAYER_DAMAGE_MULTIPLIER_ONE, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS,
                KF_ENUM_DECODE(KfEffectKind, KF_ENUM_ENCODE(u8, player_state.selected_magic_id)), &world_pos, &direction, KF_EFFECT_ARGS_DURATION_SOUND(distance, KF_EFFECT_SOUND_PLAY));
        }
        break;
    }
    case KF_MAGIC_FIRE_WALL: {
        s32 distance;
        KfActor *target;

        target = actor_pool_find_target_in_cone(
            &player_state.camera_position,
            player_state.camera_rotation.vy, MAGIC_TARGET_MAX_DISTANCE, KF_ACTOR_AIM_TOLERANCE, &distance);
        if (target != 0) {
            effect_pool_construct(
                KF_PLAYER_DAMAGE_MULTIPLIER_ONE, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                KF_ENUM_DECODE(KfEffectKind, KF_ENUM_ENCODE(u8, player_state.selected_magic_id)), &target->position,
                &player_state.camera_rotation, KF_EFFECT_ARGS_BRANCH(KF_EFFECT_GROUND_BRANCH_ROOT));
        } else {
            VECTOR spawn;
            s32 cell_x;
            s32 cell_z;

            spawn.vx = player_state.camera_position.vx
                       - (rsin(player_state.camera_rotation.vy) * FIRE_WALL_UNTARGETED_DISTANCE >> KF_FIXED12_BITS);
            spawn.vz = player_state.camera_position.vz
                       + (rcos(player_state.camera_rotation.vy) * FIRE_WALL_UNTARGETED_DISTANCE >> KF_FIXED12_BITS);
            cell_z = spawn.vz / KF_MAP_TILE_SIZE;
            cell_x = spawn.vx / KF_MAP_TILE_SIZE;
            spawn.vy = -(map_floor_height_grid.cells[cell_z][cell_x] * KF_MAP_HEIGHT_STEP);
            effect_pool_construct(
                KF_PLAYER_DAMAGE_MULTIPLIER_ONE, KF_EFFECT_USE_PLAYER_MAGIC | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                KF_ENUM_DECODE(KfEffectKind, KF_ENUM_ENCODE(u8, player_state.selected_magic_id)), &spawn, &player_state.camera_rotation,
                KF_EFFECT_ARGS_BRANCH(KF_EFFECT_GROUND_BRANCH_ROOT));
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
