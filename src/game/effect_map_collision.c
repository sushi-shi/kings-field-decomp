#include <kf/address.h>
#include <kf/map_data.h>
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
 * KF_COLLISION_TERRAIN stops the effect at a terrain boundary.
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

    x = position->vx / KF_MAP_TILE_SIZE;
    z = position->vz / KF_MAP_TILE_SIZE;
    subz = (s16)(position->vz % KF_MAP_TILE_SIZE);
    if ((x & 0xffff) > KF_MAP_COLUMNS - 1
        || (z & 0xffff) > KF_MAP_ROWS - 1) {
        return KF_COLLISION_TERRAIN;
    }
    cell = z * KF_MAP_COLUMNS + x;
    y = position->vy;
    floor = (u8)map_floor_height_grid[z][x] * -KF_MAP_HEIGHT_STEP;
    if (floor < y) {
        return KF_COLLISION_TERRAIN;
    }
    attr = map_cell_attribute_grid[z][x];
    records = map_cell_height_records;
    if (attr != KF_MAP_ATTRIBUTE_NONE) {
        height = map_cell_attribute_height_table[attr];
        if (height < 0) {
            if (y < height + floor) {
                return KF_COLLISION_TERRAIN;
            }
        } else if (floor + records[height].y_min <= y && y <= floor + records[height].y_max) {
            u8 orient = map_cell_orientation_grid[z][x];

            subx = (s16)(position->vx % KF_MAP_TILE_SIZE);
            switch (orient) {
            case KF_MAP_ORIENT_UNROTATED:
                subx = subz;
                break;
            case KF_MAP_ORIENT_QUARTER_TURN:
                break;
            case KF_MAP_ORIENT_HALF_TURN:
                subx = KF_MAP_TILE_SIZE - subz;
                break;
            case KF_MAP_ORIENT_THREE_QUARTER_TURN:
                subx = KF_MAP_TILE_SIZE - subx;
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
    case KF_MAP_CELL_BLOCKED:
        if (((cg[cell + KF_MAP_COLUMNS] != KF_MAP_CELL_FLOOR
              && cg[cell + KF_MAP_COLUMNS] != KF_MAP_CELL_STEP)
             || position->vz % KF_MAP_TILE_SIZE < KF_MAP_TILE_CENTER)
            && ((cg[cell - KF_MAP_COLUMNS] != KF_MAP_CELL_FLOOR
                 && cg[cell - KF_MAP_COLUMNS] != KF_MAP_CELL_STEP)
                || KF_MAP_TILE_CENTER < position->vz % KF_MAP_TILE_SIZE)
            && ((cg[cell + 1] != KF_MAP_CELL_FLOOR && cg[cell + 1] != KF_MAP_CELL_STEP)
                || position->vx % KF_MAP_TILE_SIZE < KF_MAP_TILE_CENTER)) {
            if (cg[cell - 1] != KF_MAP_CELL_FLOOR && cg[cell - 1] != KF_MAP_CELL_STEP) {
                return KF_COLLISION_TERRAIN;
            }
            if (KF_MAP_TILE_CENTER < position->vx % KF_MAP_TILE_SIZE) {
                return KF_COLLISION_TERRAIN;
            }
        }
        break;
    case KF_MAP_CELL_X_GE_Z:
        if (position->vx % KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER
            < position->vz % KF_MAP_TILE_SIZE) {
            return KF_COLLISION_TERRAIN;
        }
        break;
    case KF_MAP_CELL_SUM_LE_SIZE:
        if (KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER
            < position->vx % KF_MAP_TILE_SIZE + position->vz % KF_MAP_TILE_SIZE) {
            return KF_COLLISION_TERRAIN;
        }
        break;
    case KF_MAP_CELL_Z_GE_X:
        if (position->vz % KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER
            < position->vx % KF_MAP_TILE_SIZE) {
            return KF_COLLISION_TERRAIN;
        }
        break;
    case KF_MAP_CELL_SUM_GE_SIZE:
        if (position->vx % KF_MAP_TILE_SIZE + position->vz % KF_MAP_TILE_SIZE
            < KF_MAP_TILE_CENTER) {
            return KF_COLLISION_TERRAIN;
        }
        break;
    }

collide:
    {
        u8 kind = current_effect->type & 3;

        if (kind == 2) {
            flags = KF_COLLISION_SKIP_TERRAIN | KF_COLLISION_SKIP_ACTORS
                | KF_COLLISION_SKIP_MAP_OBJECTS | KF_COLLISION_SKIP_MAP_EVENTS;
        } else if (kind < 3) {
            if (kind != 1) {
                return 1;
            }
            flags = KF_COLLISION_SKIP_TERRAIN | KF_COLLISION_SKIP_PLAYER
                | KF_COLLISION_SKIP_MAP_OBJECTS | KF_COLLISION_SKIP_MAP_EVENTS;
        } else {
            if (kind != 3) {
                return 3;
            }
            flags = KF_COLLISION_SKIP_TERRAIN | KF_COLLISION_SKIP_MAP_OBJECTS
                | KF_COLLISION_SKIP_MAP_EVENTS;
        }
    }
    return collision_query_world(position->vx, position->vy, position->vz, radius, 0, flags);
}
