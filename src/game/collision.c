#include <kf/lib/map_data.h>
#include <kf/game/collision.h>
#include <kf/game/game.h>

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

KfCollisionTarget collision_target;

u32 collision_query_world(
    s32 point_x, s32 point_y, s32 point_z, s32 radius, s32 height, u32 flags)
{
    s32 cell = point_x / KF_MAP_TILE_SIZE
        + (s16)(point_z / KF_MAP_TILE_SIZE) * KF_MAP_COLUMNS;
    s32 floor_height;
    s32 hit;
    KfMapAttribute attribute;
    u8 cell_flags;
    u32 rejection_mask;
    u16 query_flags = flags;

    if ((query_flags & KF_COLLISION_SKIP_TERRAIN) == 0) {
        hit = ((u8)(map_collision_grid.linear[(u16)cell]));

        if (!MAP_CELL_HAS_FULL_FLOOR(((s32)(hit)))) {
            return hit | KF_COLLISION_TERRAIN;
        }
        if (point_y != KF_COLLISION_IGNORE_HEIGHT) {
            floor_height = map_floor_height_for_cell_position(
                (u16)cell, point_x, point_z);
            if (floor_height < point_y) {
                return KF_COLLISION_BELOW_FLOOR;
            }
            attribute = map_cell_attribute_grid.linear[(u16)cell];
            if (attribute == KF_MAP_ATTRIBUTE_NONE) {
                return KF_COLLISION_MISSING_ATTRIBUTE;
            }
            if (map_cell_attribute_height_table[((u8)(attribute))] + height < 0
                && point_y < map_cell_attribute_height_table[((u8)(attribute))] + height + floor_height) {
                return KF_COLLISION_CEILING;
            }
        }
    }
    cell_flags = map_collision_flag_grid.linear[(u16)cell];
    rejection_mask = (query_flags >> KF_COLLISION_CELL_FLAG_SHIFT) & KF_COLLISION_CELL_FLAG_MASK;
    hit = cell_flags & rejection_mask;
    if (hit != 0) {
        return hit << KF_COLLISION_CELL_FLAG_SHIFT;
    }
    if ((cell_flags & KF_CELL_OCCUPANT_COUNT_MASK) == 0) {
        return KF_COLLISION_NONE;
    }
    if ((query_flags & KF_COLLISION_SKIP_PLAYER) == 0) {
        hit = player_distance_to_point(
            point_x, point_y, point_z, radius + KF_COLLISION_PLAYER_RADIUS, height);
        if (hit != KF_COLLISION_NONE) {
            if (query_flags & KF_COLLISION_CAPTURE_TARGET) {
                collision_target.position = player_state.camera_position;
                collision_target.rotation = player_state.camera_rotation;
                collision_target.radius = KF_COLLISION_PLAYER_RADIUS;
            }
            return KF_COLLISION_PLAYER;
        }
    }
    if ((query_flags & KF_COLLISION_SKIP_ACTORS) == 0) {
        hit = actor_pool_find_overlap(point_x, point_y, point_z, radius, height);
        if (hit != KF_COLLISION_NONE) {
            if (query_flags & KF_COLLISION_CAPTURE_TARGET) {
                KfActor *actor = &actor_state.actors[hit];
                KfActorDefinition *definition = &actor_state.definitions.entries[actor->definition_id];

                collision_target.position = actor->position;
                collision_target.rotation = actor->rotation.vector;
                collision_target.radius = definition->collision_radius;
            }
            return hit | KF_COLLISION_ACTOR;
        }
    }
    if ((query_flags & KF_COLLISION_SKIP_MAP_OBJECTS) == 0) {
        hit = map_object_pool_find_near_point(point_x, point_z, radius);
        if (hit != KF_COLLISION_NONE) {
            if (query_flags & KF_COLLISION_CAPTURE_TARGET) {
                KfMapObject *object = &map_object_state.objects[hit];
                KfMapObjectDefinition *definition = &map_object_state.definitions.entries[((u8)(object->object_id))];

                collision_target.position = object->position;
                collision_target.rotation = object->rotation.vector;
                collision_target.radius = definition->collision_radius;
            }
            return hit | KF_COLLISION_MAP_OBJECT;
        }
    }
    if (query_flags & KF_COLLISION_SKIP_MAP_EVENTS) {
        return KF_COLLISION_NONE;
    }
    hit = map_event_pool_find_overlap(point_x, point_z, radius);
    if (hit != KF_COLLISION_NONE) {
        if (query_flags & KF_COLLISION_CAPTURE_TARGET) {
            KfMapEvent *event = &map_event_pool[hit];

            collision_target.position = event->reference_position;
            collision_target.rotation = event->rotation;
            collision_target.radius = event->radius;
        }
        return hit | KF_COLLISION_MAP_EVENT;
    }
    return KF_COLLISION_NONE;
}
