#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfMapObjectState map_object_state;

extern const KfMapCopyRegion map_copy_regions[4];
extern u8 map_floor_height_grid[100][100];
extern u8 map_collision_grid[100][100];
extern u8 map_cell_attribute_grid[100][100];
/* Two unnamed 100x100 map layers copied alongside the named grids. */
extern u8 map_cell_orientation_grid[100][100];
extern u8 map_collision_flag_grid[100][100];
extern u16 map_object_effect_sequence_160;
extern u16 map_object_effect_sequence_170;
extern u16 map_object_effect_sequence_180;
extern u32 collision_query_world(
    s32 point_x, s32 point_y, s32 point_z, s32 radius, s32 height, u32 flags);

ADDRESS(0x80030a98, 0x1e4)
void map_apply_copy_region(u8 region_id)
{
    const KfMapCopyRegion *region;
    u8 height;
    u8 width;
    s32 source_z;
    s32 destination_z;
    s32 source_x;
    s32 destination_x;

    if (region_id == 0xff) {
        return;
    }
    region = &map_copy_regions[region_id];
    height = region->height;
    source_z = region->source_z;
    destination_z = region->destination_z;
    while (height-- != 0) {
        width = region->width;
        destination_x = region->destination_x;
        source_x = region->source_x;
        while (width-- != 0) {
            map_cell_attribute_grid[destination_z][destination_x] =
                map_cell_attribute_grid[source_z][source_x];
            map_floor_height_grid[destination_z][destination_x] =
                map_floor_height_grid[source_z][source_x];
            map_cell_orientation_grid[destination_z][destination_x] = map_cell_orientation_grid[source_z][source_x];
            map_collision_grid[destination_z][destination_x] =
                map_collision_grid[source_z][source_x];
            map_collision_flag_grid[destination_z][destination_x] = map_collision_flag_grid[source_z][source_x];
            destination_x++;
            source_x++;
        }
        source_z++;
        destination_z++;
    }
}

ADDRESS(0x80030c7c, 0x23c)
void map_object_mark_collision_edge(const KfMapObject *object, u8 value, u16 yaw)
{
    u8 cell_x = object->cell_x;
    u8 cell_z;
    const KfMapObjectDefinition *definition;

    definition = &map_object_state.definitions[object->object_id];
    yaw &= 0xfff;
    cell_z = object->cell_z;
    switch (definition->behavior_type) {
    case 2:
    case 3:
        map_collision_grid[cell_z][cell_x] = value;
        switch (yaw) {
        case 0x000:
            cell_z++;
            break;
        case 0x400:
            cell_x++;
            break;
        case 0x800:
            cell_z--;
            break;
        case 0xc00:
            cell_x--;
            break;
        }
        map_collision_grid[cell_z][cell_x] = value;
        break;
    case 0:
        switch (yaw) {
        case 0x000:
            map_collision_grid[cell_z - 1][cell_x + 1] = value;
            map_collision_grid[cell_z][cell_x + 1] = value;
            break;
        case 0x400:
            map_collision_grid[cell_z + 1][cell_x] = value;
            map_collision_grid[cell_z + 1][cell_x + 1] = value;
            break;
        case 0x800:
            map_collision_grid[cell_z + 1][cell_x - 1] = value;
            map_collision_grid[cell_z][cell_x - 1] = value;
            break;
        case 0xc00:
            map_collision_grid[cell_z - 1][cell_x] = value;
            map_collision_grid[cell_z - 1][cell_x - 1] = value;
            break;
        }
        break;
    }
}

ADDRESS(0x80030eb8, 0xc4)
s32 map_object_probe_forward(const KfMapObject *object, u16 yaw)
{
    const KfMapObjectDefinition *definition = &map_object_state.definitions[object->object_id];
    s32 point_x = object->position_x;
    s32 point_z = object->position_z;
    s32 result;

    yaw &= 0xfff;
    if (definition->behavior_type == 0) {
        switch (yaw) {
        case 0x000:
            point_x += 2000;
            break;
        case 0x400:
            point_z += 2000;
            break;
        case 0x800:
            point_x -= 2000;
            break;
        case 0xc00:
            point_z -= 2000;
            break;
        default:
            return result;
        }
    } else if (definition->behavior_type != 2) {
        return result;
    }
    result = collision_query_world(point_x, 0xffff, point_z, 3000, 0, 0x21);
    return result;
}

ADDRESS(0x80030f7c, 0x60)
void map_object_pool_clear(void)
{
    KfMapObject *object = map_object_state.objects;
    u16 index = 189;

    do {
        object->object_id = 0xff;
        object->action = 0xff;
        object->link.vertical_velocity = 0;
        object->link.unknown_06[0] = 0;
        object->link.unknown_06[1] = 0;
        object->link.link_id = 0;
        object->link.action_parameter = 0;
        object->link.spawn_sequence = 0;
        object++;
    } while (index-- != 0);
    map_object_effect_sequence_180 = 0;
    map_object_effect_sequence_170 = 0;
    map_object_effect_sequence_160 = 0;
}
