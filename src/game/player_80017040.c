#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
extern u16 vector_xz_to_angle(s32 x, s32 z);
/* Psy-Q LIBGTE: long SquareRoot0(long a); */
extern s32 SquareRoot0(s32 value);
extern s32 player_distance_to_point(
    s32 point_x, s32 point_y, s32 point_z, s32 max_distance, s32 point_height);

ADDRESS(0x80017040, 0xc8)
s32 player_distance_to_point_in_cone(
    const struct KfVec3i *point, s16 facing, s32 max_distance, s32 angle_tolerance)
{
    s32 distance;
    s16 delta;

    distance = player_distance_to_point(point->x, 0xffff, point->z, max_distance, 0);
    if (distance == -1) {
        return distance;
    }
    delta = (vector_xz_to_angle(
                 player_state.camera_position.x - point->x,
                 point->z - player_state.camera_position.z)
             - facing) & 0xfff;
    if (delta > 2048) {
        delta = 0x1000 - delta;
    }
    if (angle_tolerance < delta) {
        distance = -1;
    }
    return distance;
}

/* point_height is reused as the vertical tolerance, as retail keeps it in $a3. */
ADDRESS(0x80017108, 0xf4)
s32 player_distance_to_point(
    s32 point_x, s32 point_y, s32 point_z, s32 max_distance, s32 point_height)
{
    s32 dx;
    s32 dz;
    s32 center;
    s32 dy;
    s32 distance;

    dx = player_state.camera_position.x - point_x;
    if (dx < -max_distance || max_distance < dx) {
        return -1;
    }
    dz = player_state.camera_position.z - point_z;
    if (dz < -max_distance || max_distance < dz) {
        return -1;
    }
    dx >>= 3;
    if (point_y != 0xffff) {
        point_height >>= 1;
        center = point_y - point_height;
        point_height += 850;
        center += 850;
        dy = player_state.floor_height - center;
        if (dy < -point_height || point_height < dy) {
            return -1;
        }
    }
    dz >>= 3;
    distance = SquareRoot0(dx * dx + dz * dz) << 3;
    if (max_distance < distance) {
        return -1;
    }
    return distance;
}
