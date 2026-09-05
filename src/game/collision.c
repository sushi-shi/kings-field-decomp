#include <kf/address.h>
#include <kf/game_collision.h>
#include <kf/game.h>

/*
 * Negative values are floor-relative heights; nonnegative values select
 * map_cell_height_records in effect_map_collision. Attribute 0xff is excluded.
 */
DATA(0x800558b8, 0x1fe)
s16 map_cell_attribute_height_table[255] = {
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
            floor_height = map_floor_height_for_cell_position(
                (u16)cell, point_x, point_z);
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
                collision_target.position = player_state.camera_position;
                collision_target.rotation = player_state.camera_rotation;
                collision_target.radius = 800;
            }
            return 0x800000;
        }
    }
    if ((flags & 0x10) == 0) {
        hit = actor_pool_find_overlap(point_x, point_y, point_z, radius, height);
        if (hit != -1) {
            if (flags & 0x800) {
                KfActor *actor = &actor_state.actors[hit];

                collision_target.position = actor->position;
                collision_target.rotation = *(SVECTOR *)&actor->rotation;
                collision_target.radius =
                    actor_state.definitions[actor->definition_id].collision_radius;
            }
            return hit | 0x100000;
        }
    }
    if ((flags & 0x20) == 0) {
        hit = map_object_pool_find_near_point(point_x, point_z, radius);
        if (hit != -1) {
            if (flags & 0x800) {
                KfMapObject *object = &map_object_state.objects[hit];

                collision_target.position = *(VECTOR *)&object->position_x;
                collision_target.rotation = *(SVECTOR *)&object->rotation;
                collision_target.radius =
                    map_object_state.definitions[object->object_id].collision_radius;
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

            collision_target.position = *(VECTOR *)&event->reference_x;
            collision_target.rotation = *(SVECTOR *)&event->unknown_34;
            collision_target.radius = event->radius;
        }
        return hit | 0x400000;
    }
    return -1;
}
