#include <kf/address.h>
#include <kf/semantic_types.h>

RODATA(0x80012738, 0x150)

extern KfMapObjectState map_object_state;
extern u8 map_floor_height_grid[100][100];
extern KfEffectRecord DAT_8009d040[];
/* Effect spawner called with five or six arguments; declared without a prototype. */
extern KfEffectRecord *func_80036f44();
extern void collision_adjust_cell_occupancy(u16 cell_x, u16 cell_z, s32 delta);
extern void map_object_start_action_if_idle(KfMapObject *object, u8 action);
extern void map_object_mark_collision_edge(const KfMapObject *object, u8 value, u16 yaw);

/*
 * Fills the 190 pool records from the sentinel-terminated placement list:
 * tile and local offsets become world positions, the link block is copied,
 * occupied cells join the collision census, a few object ids spawn their
 * effect and remember its slot, and every object marks its collision edge.
 */
ADDRESS(0x80031008, 0x448)
void map_object_pool_load(const KfMapObjectPlacement *placements)
{
    u16 remaining;
    u16 ended = 0;
    const KfMapObjectPlacement *placement = placements;
    KfMapObject *object = map_object_state.objects;
    KfMapObjectDefinition *definition;
    u32 effect_output[2];
    u8 object_id;

    remaining = 189;
    for (;;) {
        if (ended == 1) {
        fill:
            object->object_id = 0xff;
        } else if (placement->object_id != 0xff) {
            object_id = placement->object_id;
            object->object_id = object_id;
            object->cell_x = placement->tile_x;
            object->cell_z = placement->tile_z;
            object->rotation.z = 0;
            object->rotation.x = 0;
            object->rotation.y = placement->yaw & 0xfff;
            object->position_x = placement->tile_x * 2000 + placement->local_x;
            object->position_z = placement->tile_z * 2000 + placement->local_z;
            object->position_y = placement->local_y
                - map_floor_height_grid[placement->tile_z][placement->tile_x] * 100;
            object->action = 0xff;
            /* The link block moves as two aligned words. */
            memcpy((u32 *)&object->link, (const u32 *)&placement->link, sizeof object->link);
            definition = &map_object_state.definitions[object->object_id];
            if (definition->collision_radius != 0) {
                collision_adjust_cell_occupancy(object->cell_x, object->cell_z, 1);
            }
            switch (object_id) {
            case 136:
                object->link.action_parameter = func_80036f44(
                                                    *(u8 *)&object->link.spawn_sequence,
                                                    0x23,
                                                    0x11,
                                                    &object->position_x,
                                                    effect_output)
                    - DAT_8009d040;
                map_object_start_action_if_idle(object, 0x51);
                break;
            case 115:
            case 124:
            case 125:
            case 137:
                map_object_start_action_if_idle(object, 0x50);
                break;
            case 138:
                object->link.action_parameter = func_80036f44(
                                                    *(u8 *)&object->link.spawn_sequence,
                                                    0x23,
                                                    0xf,
                                                    &object->position_x,
                                                    effect_output,
                                                    &object->rotation)
                    - DAT_8009d040;
                map_object_start_action_if_idle(object, 0x51);
                break;
            case 139:
                object->link.action_parameter = func_80036f44(
                                                    *(u8 *)&object->link.spawn_sequence,
                                                    0x23,
                                                    0x10,
                                                    &object->position_x,
                                                    effect_output,
                                                    &object->rotation)
                    - DAT_8009d040;
                map_object_start_action_if_idle(object, 0x52);
                break;
            case 135:
                object->link.action_parameter =
                    func_80036f44(0, 3, 0x30, &object->position_x, effect_output, &object->rotation)
                    - DAT_8009d040;
                map_object_start_action_if_idle(object, 0x53);
                break;
            case 56:
            case 63:
            case 64:
            case 68:
            case 69:
                map_object_start_action_if_idle(object, 0xc);
                object->position_y += 10000;
                break;
            case 111:
            case 123:
                map_object_start_action_if_idle(object, 0xb);
                break;
            }
            if (definition->behavior_type == 10) {
                map_object_start_action_if_idle(object, 10);
            }
            map_object_mark_collision_edge(object, 0, object->rotation.y);
            placement++;
        } else {
            ended = 1;
            goto fill;
        }
        object++;
        if (remaining-- == 0) {
            break;
        }
    }
}


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
    KfMapObject *object = map_object_state.objects;
    s16 index;
    u16 radius;

    for (index = 0; index < 190; index++, object++) {
        if (object->object_id == 0xff) {
            continue;
        }
        radius = map_object_state.definitions[object->object_id].collision_radius;
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
