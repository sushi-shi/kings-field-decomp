#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
extern KfActorState actor_state;
extern u8 map_cell_attribute_grid[100][100];
extern u8 map_floor_height_grid[100][100];
/* The view_rotation_offset table; the cell-attribute threshold table the player
 * targeting code reads follows it in the original translation unit and is
 * reached as an offset (+0x3e) from this base. */
extern SVECTOR DAT_80055878[8];

extern void matrix_set_rotation_yxz(const struct KfEulerAngles *angles, MATRIX *matrix);
extern KfActor *actor_pool_find_target_in_cone(const struct KfVec3i *origin,
    s32 facing, u32 max_distance, s32 angle_tolerance, s32 *distance_out);
extern s32 vector_xz_to_angle(s32 x, s32 z);
extern void pitch_yaw_to_forward_vector(const struct KfPitchYaw *angles,
    struct KfVec3s *direction);
extern void vector3s_scale_shift12(s16 scale, s16 *vector);
extern KfEffectRecord *effect_pool_construct(u8 id, u8 type, u8 kind, VECTOR *position,
    SVECTOR *direction, ...);

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
        vector3s_scale_shift12(scale, (s16 *)&direction);
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
