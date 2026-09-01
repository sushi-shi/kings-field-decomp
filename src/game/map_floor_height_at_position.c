#include <kf/address.h>
#include <kf/semantic_types.h>

extern s32 map_floor_height_for_cell_position(
    u16 cell_index, s32 point_x, s32 point_z);

ADDRESS(0x8001a44c, 0x9c)
s32 map_floor_height_at_position(const struct KfVec4i *position)
{
    s32 point_z = position->z;
    s32 point_x = position->x;

    return map_floor_height_for_cell_position(
        (point_z / 2000) * 100 + point_x / 2000,
        point_x,
        point_z);
}
