#include <kf/address.h>
#include <kf/map_data.h>
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
 * Rectangular map-cell copy regions {source_x, source_z, destination_x,
 * destination_z, width, height} applied by map_apply_copy_region.
 */
DATA(0x800561b0, 0x1e)
KfMapCopyRegion map_copy_regions[KF_MAP_COPY_REGION_COUNT] = {
    {55, 33, 50, 39, 3, 3},
    {47, 16, 30, 20, 3, 3},
    {58, 44, 15, 48, 3, 3},
    {64, 44, 37, 45, 3, 3},
    {0, 0, 36, 4, 7, 1},
};

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

    if (region_id == KF_MAP_COPY_REGION_NONE) {
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
            source_x++;
            destination_x++;
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
    yaw &= KF_ANGLE_WRAP_MASK;
    cell_z = object->cell_z;
    switch (definition->behavior_type) {
    case KF_MAP_OBJECT_BEHAVIOR_LIFT_DOOR:
    case 3:
        map_collision_grid[cell_z][cell_x] = value;
        switch (yaw) {
        case 0x000:
            cell_z++;
            break;
        case KF_ANGLE_QUARTER_TURN:
            cell_x++;
            break;
        case KF_ANGLE_HALF_TURN:
            cell_z--;
            break;
        case KF_ANGLE_THREE_QUARTER_TURN:
            cell_x--;
            break;
        }
        map_collision_grid[cell_z][cell_x] = value;
        break;
    case KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR:
        switch (yaw) {
        case 0x000:
            map_collision_grid[cell_z][cell_x + 1] =
                map_collision_grid[cell_z - 1][cell_x + 1] = value;
            break;
        case KF_ANGLE_QUARTER_TURN:
            map_collision_grid[cell_z + 1][cell_x] = value;
            map_collision_grid[cell_z + 1][cell_x + 1] = value;
            break;
        case KF_ANGLE_HALF_TURN:
            map_collision_grid[cell_z][cell_x - 1] =
                map_collision_grid[cell_z + 1][cell_x - 1] = value;
            break;
        case KF_ANGLE_THREE_QUARTER_TURN:
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

    yaw &= KF_ANGLE_WRAP_MASK;
    switch (definition->behavior_type) {
    case KF_MAP_OBJECT_BEHAVIOR_LIFT_DOOR:
    probe:
        result = collision_query_world(
            point_x, KF_COLLISION_IGNORE_HEIGHT, point_z, 3000, 0,
            KF_COLLISION_SKIP_TERRAIN | KF_COLLISION_SKIP_MAP_OBJECTS);
        break;
    case KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR:
        switch (yaw) {
        case 0x000:
            point_x += KF_MAP_TILE_SIZE;
            goto probe;
        case KF_ANGLE_QUARTER_TURN:
            point_z += KF_MAP_TILE_SIZE;
            goto probe;
        case KF_ANGLE_HALF_TURN:
            point_x -= KF_MAP_TILE_SIZE;
            goto probe;
        case KF_ANGLE_THREE_QUARTER_TURN:
            point_z -= KF_MAP_TILE_SIZE;
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
    u16 index = KF_MAP_OBJECT_CAPACITY - 1;

    do {
        u32 *link_words = (u32 *)&object->link;

        object->object_id = KF_MAP_OBJECT_FREE;
        object->action = KF_MAP_OBJECT_ACTION_IDLE;
        /* Same aligned eight-byte block used by placement loading. */
        link_words[1] = 0;
        link_words[0] = 0;
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
    s32 count = sizeof map_object_state.definitions / sizeof *source;

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

    remaining = KF_MAP_OBJECT_CAPACITY - 1;
    for (;;) {
        if (ended == 1) {
        fill:
            object->object_id = KF_MAP_OBJECT_FREE;
        } else if (placement->object_id != KF_MAP_OBJECT_FREE) {
            object_id = placement->object_id;
            object->object_id = object_id;
            object->cell_x = placement->tile_x;
            object->cell_z = placement->tile_z;
            object->rotation.z = 0;
            object->rotation.x = 0;
            object->rotation.y = placement->yaw & KF_ANGLE_WRAP_MASK;
            object->position_x = placement->tile_x * KF_MAP_TILE_SIZE + placement->local_x;
            object->position_z = placement->tile_z * KF_MAP_TILE_SIZE + placement->local_z;
            object->position_y = placement->local_y
                - map_floor_height_grid[placement->tile_z][placement->tile_x] * KF_MAP_HEIGHT_STEP;
            object->action = KF_MAP_OBJECT_ACTION_IDLE;
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
                                                    0x20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                                                    KF_EFFECT_KIND_ORBITING_PROJECTILE,
                                                    &object->position_x,
                                                    effect_output)
                    - effect_pool_records;
                map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_RELEASE_ORBIT_OR_SHORT_SWING);
                break;
            case 115:
            case 124:
            case 125:
            case 137:
                map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_PROJECTILE_EMITTER);
                break;
            case 138:
                object->link.action_parameter = effect_pool_construct(
                                                    *(u8 *)&object->link.spawn_sequence,
                                                    0x20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                                                    KF_EFFECT_KIND_SWINGING_HAZARD_SHORT,
                                                    &object->position_x,
                                                    effect_output,
                                                    &object->rotation)
                    - effect_pool_records;
                map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_RELEASE_ORBIT_OR_SHORT_SWING);
                break;
            case 139:
                object->link.action_parameter = effect_pool_construct(
                                                    *(u8 *)&object->link.spawn_sequence,
                                                    0x20 | KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER,
                                                    KF_EFFECT_KIND_SWINGING_HAZARD_LONG,
                                                    &object->position_x,
                                                    effect_output,
                                                    &object->rotation)
                    - effect_pool_records;
                map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_RELEASE_LONG_SWING);
                break;
            case 135:
                object->link.action_parameter =
                    effect_pool_construct(
                        0, KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER, 0x30, &object->position_x,
                        effect_output, &object->rotation)
                    - effect_pool_records;
                map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_EFFECT_SWITCH);
                break;
            case 56:
            case 63:
            case 64:
            case 68:
            case 69:
                map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_REVEAL_MAP_PIECE);
                object->position_y += 10000;
                break;
            case 111:
            case 123:
                map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_ENABLE_RESTORE_POINT);
                break;
            }
            if (definition->behavior_type == KF_MAP_OBJECT_BEHAVIOR_COPY_REGION) {
                map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_COPY_REGION);
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
            delta_x >>= KF_LENGTH_SQUARE_DOWNSHIFT;
            delta_z >>= KF_LENGTH_SQUARE_DOWNSHIFT;
            distance = SquareRoot0(delta_x * delta_x + delta_z * delta_z) << KF_LENGTH_SQUARE_DOWNSHIFT;
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

    for (index = 0; index < KF_MAP_OBJECT_CAPACITY; index++, object++) {
        if (object->object_id == KF_MAP_OBJECT_FREE) {
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
