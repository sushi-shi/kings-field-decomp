#include <kf/address.h>
#include <kf/game_map.h>
#include <kf/game_collision.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

/*
 * Map copy and map-object pool/load band (GAME 0x80030a98..0x800315c4).
 * The following map-object runtime/action unit remains a separate WIP owner.
 */

RODATA(0x80012738, 0x150)

/*
 * Four rectangular map-cell copy regions {source_x, source_z, destination_x,
 * destination_z, width, height} applied by map_apply_copy_region.
 */
DATA(0x800561b0, 0x18)
KfMapCopyRegion map_copy_regions[4] = {
    {55, 33, 50, 39, 3, 3},
    {47, 16, 30, 20, 3, 3},
    {58, 44, 15, 48, 3, 3},
    {64, 44, 37, 45, 3, 3},
};

/* Two unnamed 100x100 map layers copied alongside the named grids. */

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
            map_collision_grid[cell_z][cell_x + 1] =
                map_collision_grid[cell_z - 1][cell_x + 1] = value;
            break;
        case 0x400:
            map_collision_grid[cell_z + 1][cell_x] = value;
            map_collision_grid[cell_z + 1][cell_x + 1] = value;
            break;
        case 0x800:
            map_collision_grid[cell_z][cell_x - 1] =
                map_collision_grid[cell_z + 1][cell_x - 1] = value;
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
    switch (definition->behavior_type) {
    case 2:
    probe:
        result = collision_query_world(point_x, 0xffff, point_z, 3000, 0, 0x21);
        break;
    case 0:
        switch (yaw) {
        case 0x000:
            point_x += 2000;
            goto probe;
        case 0x400:
            point_z += 2000;
            goto probe;
        case 0x800:
            point_x -= 2000;
            goto probe;
        case 0xc00:
            point_z -= 2000;
            goto probe;
        }
        break;
    }
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

ADDRESS(0x80030fdc, 0x2c)
void map_object_definitions_load(const KfMapObjectDefinition *definitions)
{
    const u32 *source = (const u32 *)definitions;
    u32 *destination = (u32 *)map_object_state.definitions;
    s32 count = 0x140;

    do {
        *destination++ = *source++;
    } while (--count != 0);
}

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
                object->link.action_parameter = effect_pool_construct(
                                                    *(u8 *)&object->link.spawn_sequence,
                                                    0x23,
                                                    0x11,
                                                    &object->position_x,
                                                    effect_output)
                    - effect_pool_records;
                map_object_start_action_if_idle(object, 0x51);
                break;
            case 115:
            case 124:
            case 125:
            case 137:
                map_object_start_action_if_idle(object, 0x50);
                break;
            case 138:
                object->link.action_parameter = effect_pool_construct(
                                                    *(u8 *)&object->link.spawn_sequence,
                                                    0x23,
                                                    0xf,
                                                    &object->position_x,
                                                    effect_output,
                                                    &object->rotation)
                    - effect_pool_records;
                map_object_start_action_if_idle(object, 0x51);
                break;
            case 139:
                object->link.action_parameter = effect_pool_construct(
                                                    *(u8 *)&object->link.spawn_sequence,
                                                    0x23,
                                                    0x10,
                                                    &object->position_x,
                                                    effect_output,
                                                    &object->rotation)
                    - effect_pool_records;
                map_object_start_action_if_idle(object, 0x52);
                break;
            case 135:
                object->link.action_parameter =
                    effect_pool_construct(0, 3, 0x30, &object->position_x, effect_output, &object->rotation)
                    - effect_pool_records;
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
