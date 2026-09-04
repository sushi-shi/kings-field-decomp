#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>
#include <kf/magic.h>

extern KfEffectRecord *effect_pool_construct(u8 id, u8 type, u8 kind, VECTOR *position,
    SVECTOR *direction, ...);
extern void effect_pool_set_current(u8 *object);

ADDRESS(0x8003a244, 0x30)
void effect_pool_reset(void)
{
    KfEffectRecord *record = effect_pool_records;
    u16 i;

    for (i = 0; i < 48; i++) {
        record->type = 0xff;
        record++;
    }
}

ADDRESS(0x8003a274, 0x2c)
void magic_load_records(const u32 *source)
{
    u32 *destination = (u32 *)magic_records;
    s32 count;

    for (count = 120; count != 0; count--) {
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
    struct KfVec3s direction;
    SVECTOR offset;
    struct KfEulerAngles angles;
    VECTOR world_pos;
    MATRIX matrix;
    s32 distance;
    SVECTOR rotation;
    KfActor *target;
    s32 scale;
    s16 *threshold;

    switch (player_state.selected_magic_id) {
    case 4:
    case 5:
    case 7:
    case 8:
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
            player_state.camera_rotation.vy, 0x4e20, 0x155, &distance);
        actor_state.player_target = target;
        if (target == 0) {
            if (player_state.selected_magic_id == 4) {
                scale = 800;
                angles.x = -128;
                distance = 20;
            } else {
                scale = 600;
                angles.x = player_state.camera_rotation.vx;
            }
        } else {
            if (player_state.selected_magic_id == 4) {
                scale = 600;
                threshold = (s16 *)DAT_80055878 + 0x1f;
                if (threshold[map_cell_attribute_grid[target->cell_z][target->cell_x]]
                        < -4999) {
                    angles.x = vector_xz_to_angle(
                        world_pos.vy + 5000 - target->position.vy, -distance);
                } else {
                    angles.x = vector_xz_to_angle(
                        world_pos.vy + 3000 - target->position.vy, -distance);
                }
                distance = distance / 800;
                scale = 800;
            } else {
                scale = 600;
                angles.x = player_state.camera_rotation.vx;
            }
        }
        if (player_state.selected_magic_id == 7) {
            scale = 800;
        }
        angles.y = player_state.camera_rotation.vy;
        angles.z = player_state.camera_rotation.vz;
        pitch_yaw_to_forward_vector((struct KfPitchYaw *)&angles, &direction);
        vector3s_scale_shift12(scale, &direction);
        if (player_state.selected_magic_id == 8) {
            rotation.vx = player_state.camera_rotation.vx;
            rotation.vy = player_state.camera_rotation.vy;
            rotation.vz = player_state.camera_rotation.vz;
            effect_pool_construct(0xa, 0x11, player_state.selected_magic_id, &world_pos,
                          (SVECTOR *)&direction, &rotation, 1);
        } else {
            effect_pool_construct(0xa, 0x11, player_state.selected_magic_id, &world_pos,
                          (SVECTOR *)&direction, distance, 1);
        }
        break;
    case 6:
        target = actor_pool_find_target_in_cone(
            (struct KfVec3i *)&player_state.camera_position,
            player_state.camera_rotation.vy, 0x4e20, 0x155, (s32 *)&distance);
        if (target != 0) {
            effect_pool_construct(0xa, 0x13, player_state.selected_magic_id,
                          &target->position,
                          (SVECTOR *)&player_state.camera_rotation, 0);
        } else {
            VECTOR spawn;
            s32 cell_x;
            s32 cell_z;

            spawn.vx = player_state.camera_position.vx
                       - (rsin(player_state.camera_rotation.vy) * 6000 >> 12);
            spawn.vz = player_state.camera_position.vz
                       + (rcos(player_state.camera_rotation.vy) * 6000 >> 12);
            cell_z = spawn.vz / 2000;
            cell_x = spawn.vx / 2000;
            spawn.vy = -(map_floor_height_grid[cell_z][cell_x] * 100);
            effect_pool_construct(0xa, 0x13, player_state.selected_magic_id, &spawn,
                          (SVECTOR *)&player_state.camera_rotation, 0);
        }
        break;
    }
}

ADDRESS(0x8003a760, 0x7c)
void effect_pool_sweep(void)
{
    KfEffectRecord *record = effect_pool_records;
    u16 i = 47;

    do {
        if (record->type != 0xff) {
            effect_pool_set_current((u8 *)record);
            effect_update_dispatch();
        }
        record++;
    } while (i-- != 0);
}
