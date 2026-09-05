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
 * collision_query_world with the flags selected from the active effect record;
 * 0x10000 means "no collision".
 */
ADDRESS(0x80037850, 0x76c)
u32 effect_map_collision(VECTOR *position, s32 radius)
{
    KfCellHeightRecord *records;
    u8 *cg;
    s32 x;
    s32 z;
    s32 subx;
    s32 subz;
    s32 y;
    s32 floor;
    s32 cell;
    s32 height;
    u8 attr;
    u32 flags;

    x = position->vx / 2000;
    z = position->vz / 2000;
    subz = (s16)(position->vz % 2000);
    if ((x & 0xffff) > 99 || (z & 0xffff) > 99) {
        return 0x10000;
    }
    cell = z * 100 + x;
    y = position->vy;
    floor = (u8)map_floor_height_grid[z][x] * -100;
    if (floor < y) {
        return 0x10000;
    }
    attr = map_cell_attribute_grid[z][x];
    records = map_cell_height_records;
    if (attr != 0xff) {
        height = map_cell_attribute_height_table[attr];
        if (height < 0) {
            if (y < height + floor) {
                return 0x10000;
            }
        } else if (floor + records[height].y_min <= y && y <= floor + records[height].y_max) {
            u8 orient = map_cell_orientation_grid[z][x];

            subx = (s16)(position->vx % 2000);
            switch (orient) {
            case 1:
                subx = subz;
                break;
            case 2:
                break;
            case 3:
                subx = 2000 - subz;
                break;
            case 4:
                subx = 2000 - subx;
                break;
            default:
                goto grid_shape;
            }
            if (records[height].x_min <= subx && subx <= records[height].x_max) {
                goto collide;
            }
        }
    }

grid_shape:
    cg = &map_collision_grid[0][0];
    switch (cg[cell]) {
    case 0:
        if (((cg[cell + 100] != 1 && cg[cell + 100] != 6) || position->vz % 2000 < 1000) &&
            ((cg[cell - 100] != 1 && cg[cell - 100] != 6) || 1000 < position->vz % 2000) &&
            ((cg[cell + 1] != 1 && cg[cell + 1] != 6) || position->vx % 2000 < 1000)) {
            if (cg[cell - 1] != 1 && cg[cell - 1] != 6) {
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
        if (position->vz % 2000 + 1000 < position->vx % 2000) {
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
    {
        u8 kind = current_effect->type & 3;

        if (kind == 2) {
            flags = 0x71;
        } else if (kind < 3) {
            if (kind != 1) {
                return 1;
            }
            flags = 0xe1;
        } else {
            if (kind != 3) {
                return 3;
            }
            flags = 0x61;
        }
    }
    return collision_query_world(position->vx, position->vy, position->vz, radius, 0, flags);
}
