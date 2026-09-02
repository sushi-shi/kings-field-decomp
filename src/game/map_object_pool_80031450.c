#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfMapObject map_object_pool[190];
extern KfMapObjectDefinition map_object_definitions[160];
extern s32 SquareRoot0(s32 value);

extern s32 map_object_distance_to_point(
    const KfMapObject *object, s32 point_x, s32 point_z, s32 max_distance);

ADDRESS(0x80031450, 0xa8)
s32 map_object_distance_to_point(
    const KfMapObject *object, s32 point_x, s32 point_z, s32 max_distance)
{
    s32 delta_x = object->position_x - point_x;
    s32 delta_z;
    s32 distance;

    if (delta_x >= -max_distance && delta_x <= max_distance) {
        delta_z = object->position_z - point_z;
        if (delta_z >= -max_distance && delta_z <= max_distance) {
            delta_x >>= 3;
            delta_z >>= 3;
            distance = SquareRoot0(delta_x * delta_x + delta_z * delta_z) << 3;
            if (distance <= max_distance) {
                return distance;
            }
        }
    }
    return -1;
}

ADDRESS(0x800314f8, 0xcc)
s32 map_object_pool_find_near_point(s32 point_x, s32 point_z, s32 radius_padding)
{
    KfMapObject *object = map_object_pool;
    s16 index;
    u16 radius;

    for (index = 0; index < 190; index++, object++) {
        if (object->object_id == 0xff) {
            continue;
        }
        radius = map_object_definitions[object->object_id].collision_radius;
        if (radius == 0) {
            continue;
        }
        if (map_object_distance_to_point(object, point_x, point_z, radius + radius_padding)
            != -1) {
            return index;
        }
    }
    return -1;
}
