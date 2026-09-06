#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_collision.h>
#include <kf/game.h>

/*
 * Negative values are floor-relative heights; nonnegative values select
 * map_cell_height_records in effect_map_collision. Attribute 0xff is excluded.
 */
DATA(0x800558b8, 0x1fe)
s16 map_cell_attribute_height_table[KF_MAP_ATTRIBUTE_COUNT] = {
    -25000, -3000, -3000, -3000, -3000, -3000, -3000, -3000,
    -3000, -3000, -3000, -3000, -3000, -3000, -3000, -3000,
    -3000, -3000, -3000, -5000, -5000, -5000, -5000, -2500,
    -2500, -2500, -2500, -2500, -2500, -2500, -2500, -2500,
    -5000, -5000, -5000, -2500, -2500, -3000, -3000, -3000,
    -3000, -5000, -5000, -5000, -5000, -3000, -3000, -3000,
    -5000, -5000, -3000, -3000, -3000, 0, -5000, -5000,
    -5000, -3000, -3000, -5000, -3000, -5000, -3000, -3000,
    -5000, -3000, -3000, -3000, -3000, -3000, -3000, 4,
    5, 6, -25000, -5000, 1, 2, 3, -3000,
    -3000, -3000, -25000, -3000, -3000, -3000, -3000, -3000,
    -3000, -3000, -5000, -5000, -3000, -13000, -3000, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -25000,
};

/*
 * World-space probe used by movement and interaction. Cells are 2000 units
 * wide and the grids are indexed by the flat cell number. Bits of `flags`:
 * 0x1 skips the terrain tests, 0x80/0x10/0x20/0x40 skip the player, actor,
 * map-object and map-event pools, 0x800 requests the hit's transform in
 * collision_target, and 0xf000 selects flag-grid bits that reject at once.
 */
ADDRESS(0x8001a5ac, 0x504)
u32 collision_query_world(
    s32 point_x, s32 point_y, s32 point_z, s32 radius, s32 height, u32 flags)
{
    s32 cell = point_x / KF_MAP_TILE_SIZE
        + (s16)(point_z / KF_MAP_TILE_SIZE) * KF_MAP_COLUMNS;
    s32 floor_height;
    s32 hit;
    u8 attribute;
    u8 cell_flags;
    u32 rejection_mask;

    if ((flags & KF_COLLISION_SKIP_TERRAIN) == 0) {
        hit = map_collision_grid[0][(u16)cell];

        if (hit != KF_MAP_CELL_FLOOR && hit != KF_MAP_CELL_STEP) {
            return hit | KF_COLLISION_TERRAIN;
        }
        if (point_y != KF_COLLISION_IGNORE_HEIGHT) {
            floor_height = map_floor_height_for_cell_position(
                (u16)cell, point_x, point_z);
            if (floor_height < point_y) {
                return KF_COLLISION_BELOW_FLOOR;
            }
            attribute = map_cell_attribute_grid[0][(u16)cell];
            if (attribute == KF_MAP_ATTRIBUTE_NONE) {
                return KF_COLLISION_MISSING_ATTRIBUTE;
            }
            if (map_cell_attribute_height_table[attribute] + height < 0
                && point_y < map_cell_attribute_height_table[attribute] + height + floor_height) {
                return KF_COLLISION_CEILING;
            }
        }
    }
    cell_flags = map_collision_flag_grid[0][(u16)cell];
    rejection_mask = (flags >> KF_COLLISION_CELL_FLAG_SHIFT) & KF_COLLISION_CELL_FLAG_MASK;
    hit = cell_flags & rejection_mask;
    if (hit != 0) {
        return hit << KF_COLLISION_CELL_FLAG_SHIFT;
    }
    if ((cell_flags & KF_CELL_OCCUPANT_COUNT_MASK) == 0) {
        return KF_COLLISION_NONE;
    }
    if ((flags & KF_COLLISION_SKIP_PLAYER) == 0) {
        hit = player_distance_to_point(
            point_x, point_y, point_z, radius + KF_COLLISION_PLAYER_RADIUS, height);
        if (hit != KF_COLLISION_NONE) {
            if (flags & KF_COLLISION_CAPTURE_TARGET) {
                collision_target.position = player_state.camera_position;
                collision_target.rotation = player_state.camera_rotation;
                collision_target.radius = KF_COLLISION_PLAYER_RADIUS;
            }
            return KF_COLLISION_PLAYER;
        }
    }
    if ((flags & KF_COLLISION_SKIP_ACTORS) == 0) {
        hit = actor_pool_find_overlap(point_x, point_y, point_z, radius, height);
        if (hit != KF_COLLISION_NONE) {
            if (flags & KF_COLLISION_CAPTURE_TARGET) {
                KfActor *actor = &actor_state.actors[hit];
                KfActorDefinition *definition = &actor_state.definitions[actor->definition_id];

                collision_target.position = actor->position;
                collision_target.rotation = *(SVECTOR *)&actor->rotation;
                collision_target.radius = definition->collision_radius;
            }
            return hit | KF_COLLISION_ACTOR;
        }
    }
    if ((flags & KF_COLLISION_SKIP_MAP_OBJECTS) == 0) {
        hit = map_object_pool_find_near_point(point_x, point_z, radius);
        if (hit != KF_COLLISION_NONE) {
            if (flags & KF_COLLISION_CAPTURE_TARGET) {
                KfMapObject *object = &map_object_state.objects[hit];
                KfMapObjectDefinition *definition = &map_object_state.definitions[object->object_id];

                collision_target.position = *(VECTOR *)&object->position_x;
                collision_target.rotation = *(SVECTOR *)&object->rotation;
                collision_target.radius = definition->collision_radius;
            }
            return hit | KF_COLLISION_MAP_OBJECT;
        }
    }
    if (flags & KF_COLLISION_SKIP_MAP_EVENTS) {
        return KF_COLLISION_NONE;
    }
    hit = map_event_pool_find_overlap(point_x, point_z, radius);
    if (hit != KF_COLLISION_NONE) {
        if (flags & KF_COLLISION_CAPTURE_TARGET) {
            KfMapEvent *event = &map_event_pool[hit];

            collision_target.position = *(VECTOR *)&event->reference_x;
            collision_target.rotation = *(SVECTOR *)&event->rotation_x;
            collision_target.radius = event->radius;
        }
        return hit | KF_COLLISION_MAP_EVENT;
    }
    return KF_COLLISION_NONE;
}
