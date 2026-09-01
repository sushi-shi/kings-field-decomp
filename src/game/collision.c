#include <kf/address.h>
#include <kf/semantic_types.h>

extern s32 func_8001a29c(u32 cell, s32 point_x, s32 point_z);
extern s32 player_distance_to_point(
    s32 point_x, s32 point_y, s32 point_z, s32 max_distance, s32 point_height);
extern s32 actor_pool_find_overlap(s32 x, s32 y, s32 z, s32 extra_radius, s32 point_height);
extern s32 map_object_pool_find_near_point(s32 point_x, s32 point_z, s32 radius_padding);
extern s32 map_event_pool_find_overlap(s32 point_x, s32 point_z, s32 radius_padding);

extern u8 map_collision_grid[100][100];
extern u8 map_cell_attribute_grid[100][100];
extern s16 map_cell_attribute_height_table[284];
extern u8 map_collision_flag_grid[100][100];
extern KfCollisionTarget collision_target;
extern struct KfVec4i camera_position;
extern struct KfVec4s camera_rotation;
extern KfActor actor_pool[128];
extern KfActorDefinition actor_definitions[12];
extern KfMapObject map_object_pool[190];
extern KfMapObjectDefinition map_object_definitions[160];
extern KfMapEvent map_event_pool[8];

/*
 * World-space probe used by movement and interaction. Cells are 2000 units
 * wide and the grids are indexed by the flat cell number. Bits of `flags`:
 * 0x1 skips the terrain tests, 0x80/0x10/0x20/0x40 skip the player, actor,
 * map-object and map-event pools, 0x800 requests the hit's transform in
 * collision_target, and 0xf000 selects flag-grid bits that reject at once.
 *
 * Retail reads the actor and map-object definition radii relative to the
 * pool base register, which the compiler emits only when the definitions
 * and the pool are one aggregate; the inventory keeps them separate.
 */
ADDRESS(0x8001a5ac, 0x504)
u32 collision_query_world(
    s32 point_x, s32 point_y, s32 point_z, s32 radius, s32 height, u32 flags)
{
    s32 cell = point_x / 2000 + (s16)(point_z / 2000) * 100;
    s32 floor_height;
    s32 hit;
    u8 attribute;
    u8 cell_flags;
    u32 rejected;

    if ((flags & 1) == 0) {
        u32 kind = map_collision_grid[0][(u16)cell];

        if (kind != 1 && kind != 6) {
            return kind | 0x10000;
        }
        if (point_y != 0xffff) {
            floor_height = func_8001a29c((u16)cell, point_x, point_z);
            if (floor_height < point_y) {
                return 0x1fff0;
            }
            attribute = map_cell_attribute_grid[0][(u16)cell];
            if (attribute == 0xff) {
                return 0x1fff2;
            }
            if (map_cell_attribute_height_table[attribute] + height < 0
                && point_y < map_cell_attribute_height_table[attribute] + height + floor_height) {
                return 0x1fff1;
            }
        }
    }
    cell_flags = map_collision_flag_grid[0][(u16)cell];
    rejected = cell_flags & ((flags >> 8) & 0xf0);
    if (rejected != 0) {
        return rejected << 8;
    }
    if ((cell_flags & 0x1f) == 0) {
        return -1;
    }
    if ((flags & 0x80) == 0) {
        hit = player_distance_to_point(point_x, point_y, point_z, radius + 800, height);
        if (hit != -1) {
            if (flags & 0x800) {
                collision_target.position = camera_position;
                collision_target.rotation = camera_rotation;
                collision_target.radius = 800;
            }
            return 0x800000;
        }
    }
    if ((flags & 0x10) == 0) {
        hit = actor_pool_find_overlap(point_x, point_y, point_z, radius, height);
        if (hit != -1) {
            if (flags & 0x800) {
                KfActor *actor = &actor_pool[hit];

                collision_target.position = *(struct KfVec4i *)&actor->position;
                collision_target.rotation = *(struct KfVec4s *)&actor->rotation;
                collision_target.radius =
                    actor_definitions[actor->definition_id].collision_radius;
            }
            return hit | 0x100000;
        }
    }
    if ((flags & 0x20) == 0) {
        hit = map_object_pool_find_near_point(point_x, point_z, radius);
        if (hit != -1) {
            if (flags & 0x800) {
                KfMapObject *object = &map_object_pool[hit];

                collision_target.position = *(struct KfVec4i *)&object->position_x;
                collision_target.rotation = *(struct KfVec4s *)&object->rotation;
                collision_target.radius =
                    map_object_definitions[object->object_id].collision_radius;
            }
            return hit | 0x200000;
        }
    }
    if (flags & 0x40) {
        return -1;
    }
    hit = map_event_pool_find_overlap(point_x, point_z, radius);
    if (hit != -1) {
        if (flags & 0x800) {
            KfMapEvent *event = &map_event_pool[hit];

            collision_target.position = *(struct KfVec4i *)&event->reference_x;
            collision_target.rotation = *(struct KfVec4s *)&event->unknown_34;
            collision_target.radius = event->radius;
        }
        return hit | 0x400000;
    }
    return -1;
}
