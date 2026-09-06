#include <kf/address.h>
#include <kf/game_collision.h>
#include <kf/game_effect.h>
#include <kf/game.h>

DATA(0x80055ab8, 0x38)
KfCellHeightRecord map_cell_height_records[7] = {
    {500, -2000, 1500, -1250},
    {1000, -2500, 2000, -1000},
    {0, -2500, 2000, -1000},
    {0, -2500, 1000, -1000},
    {1000, -10000, 2000, -15000},
    {0, -10000, 2000, -15000},
    {0, -10000, 1000, -15000},
};

/* Switch jump table for the diagonal-wall cell shapes. */
RODATA(0x80012ce0, 0x18)

/*
 * Probe whether a world position collides with the map geometry at its cell.
 * Converts x/z to a 100x100 cell, rejects out-of-range cells and positions
 * below the cell floor, then tests the cell's attribute-driven height/step
 * shape and its collision-grid shape (flat, four diagonal half-cells, or the
 * neighbour-aware corner cell 0).  A surviving hit is forwarded to
 * collision_query_world with the flags selected from the active effect record.
 * Geometry rejection returns 0x10000; callers recognize -1 as no collision.
 */
ADDRESS(0x80037850, 0x76c)
u32 effect_map_collision(VECTOR *position, s32 radius)
{
    KfCellHeightRecord *record;
    KfEffectRecord *effect;
    s16 x;
    s16 z;
    s32 subx;
    s32 subz;
    s32 y;
    s32 floor;
    s32 height;
    u8 attr;

    x = position->vx / 2000;
    z = position->vz / 2000;
    subz = position->vz % 2000;
    effect = current_effect;
    if (x < 0 || x >= 100 || z < 0 || z >= 100) {
        return 0x10000;
    }
    y = position->vy;
    floor = (u8)map_floor_height_grid[z][x] * -100;
    if (floor < y) {
        return 0x10000;
    }
    attr = map_cell_attribute_grid[z][x];
    if (attr != 0xff) {
        height = map_cell_attribute_height_table[attr];
        if (height < 0) {
            height += floor;
            if (y < height) {
                return 0x10000;
            }
        } else {
            record = &map_cell_height_records[height];
            if (floor + record->y_min <= y && y <= floor + record->y_max) {
                u8 orient = map_cell_orientation_grid[z][x];
                s16 coordinate;

                subx = position->vx % 2000;
                switch (orient) {
                case 1:
                    coordinate = subz;
rectangle_span:
                    if (record->x_min <= coordinate && coordinate <= record->x_max) {
                        goto collide;
                    }
                    break;
                case 2:
                    coordinate = subx;
                    goto rectangle_span;
                case 3:
                    coordinate = 2000 - subz;
                    goto rectangle_span;
                case 4:
                    coordinate = 2000 - subx;
                    goto rectangle_span;
                default:
                    goto grid_shape;
                }
            }
        }
    }

grid_shape:
    switch (map_collision_grid[z][x]) {
    case 0:
        if (((map_collision_grid[z + 1][x] != 1 && map_collision_grid[z + 1][x] != 6)
                || position->vz % 2000 < 1000) &&
            ((map_collision_grid[z - 1][x] != 1 && map_collision_grid[z - 1][x] != 6)
                || 1000 < position->vz % 2000) &&
            ((map_collision_grid[z][x + 1] != 1 && map_collision_grid[z][x + 1] != 6)
                || position->vx % 2000 < 1000)) {
            if (map_collision_grid[z][x - 1] != 1 && map_collision_grid[z][x - 1] != 6) {
                return 0x10000;
            }
            if (1000 < position->vx % 2000) {
                return 0x10000;
            }
        }
        break;
    case 2:
        if (position->vx % 2000 + 1000 < position->vz % 2000) {
            return 0x10000;
        }
        break;
    case 3:
        if (3000 < position->vx % 2000 + position->vz % 2000) {
            return 0x10000;
        }
        break;
    case 4:
        if (position->vx % 2000 > position->vz % 2000 + 1000) {
            return 0x10000;
        }
        break;
    case 5:
        if (position->vx % 2000 + position->vz % 2000 < 1000) {
            return 0x10000;
        }
        break;
    }

collide:
    switch (effect->type & 3) {
    case 1:
        return collision_query_world(position->vx, position->vy, position->vz, radius, 0, 0xe1);
    case 2:
        return collision_query_world(position->vx, position->vy, position->vz, radius, 0, 0x71);
    case 3:
        return collision_query_world(position->vx, position->vy, position->vz, radius, 0, 0x61);
    default:
        return 1;
    }
}
