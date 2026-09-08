#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_collision.h>
#include <kf/game_effect.h>
#include <kf/game.h>

DATA(0x80055ab8, 0x38)
KfCellHeightRecord map_cell_height_records[KF_MAP_CELL_HEIGHT_RECORD_COUNT] = {
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
 * Geometry rejection returns KF_COLLISION_TERRAIN; callers recognize
 * KF_COLLISION_NONE as no collision.
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

    x = position->vx / KF_MAP_TILE_SIZE;
    z = position->vz / KF_MAP_TILE_SIZE;
    subz = position->vz % KF_MAP_TILE_SIZE;
    effect = current_effect;
    if (x < 0 || x >= KF_MAP_COLUMNS || z < 0 || z >= KF_MAP_ROWS) {
        return KF_COLLISION_TERRAIN;
    }
    y = position->vy;
    floor = map_floor_height_grid.cells[z][x] * -KF_MAP_HEIGHT_STEP;
    if (floor < y) {
        return KF_COLLISION_TERRAIN;
    }
    attr = map_cell_attribute_grid.cells[z][x];
    if (attr != KF_MAP_ATTRIBUTE_NONE) {
        height = map_cell_attribute_height_table[attr];
        if (height < 0) {
            height += floor;
            if (y < height) {
                return KF_COLLISION_TERRAIN;
            }
        } else {
            record = &map_cell_height_records[height];
            if (floor + record->y_min <= y && y <= floor + record->y_max) {
                u8 orient = map_cell_orientation_grid.cells[z][x];
                s16 coordinate;

                subx = position->vx % KF_MAP_TILE_SIZE;
                switch (orient) {
                case KF_MAP_ORIENT_UNROTATED:
                    coordinate = subz;
rectangle_span:
                    if (coordinate >= record->x_min && coordinate <= record->x_max) {
                        goto collide;
                    }
                    break;
                case KF_MAP_ORIENT_QUARTER_TURN:
                    coordinate = subx;
                    goto rectangle_span;
                case KF_MAP_ORIENT_HALF_TURN:
                    coordinate = KF_MAP_TILE_SIZE - subz;
                    goto rectangle_span;
                case KF_MAP_ORIENT_THREE_QUARTER_TURN:
                    coordinate = KF_MAP_TILE_SIZE - subx;
                    goto rectangle_span;
                default:
                    goto grid_shape;
                }
            }
        }
    }

grid_shape:
    switch (map_collision_grid.cells[z][x]) {
    case KF_MAP_CELL_BLOCKED:
        if (((map_collision_grid.cells[z + 1][x] != KF_MAP_CELL_FLOOR
                && map_collision_grid.cells[z + 1][x] != KF_MAP_CELL_STEP)
                || position->vz % KF_MAP_TILE_SIZE < KF_MAP_TILE_CENTER) &&
            ((map_collision_grid.cells[z - 1][x] != KF_MAP_CELL_FLOOR
                && map_collision_grid.cells[z - 1][x] != KF_MAP_CELL_STEP)
                || KF_MAP_TILE_CENTER < position->vz % KF_MAP_TILE_SIZE) &&
            ((map_collision_grid.cells[z][x + 1] != KF_MAP_CELL_FLOOR
                && map_collision_grid.cells[z][x + 1] != KF_MAP_CELL_STEP)
                || position->vx % KF_MAP_TILE_SIZE < KF_MAP_TILE_CENTER)) {
            if (map_collision_grid.cells[z][x - 1] != KF_MAP_CELL_FLOOR
                    && map_collision_grid.cells[z][x - 1] != KF_MAP_CELL_STEP) {
                return KF_COLLISION_TERRAIN;
            }
            if (KF_MAP_TILE_CENTER < position->vx % KF_MAP_TILE_SIZE) {
                return KF_COLLISION_TERRAIN;
            }
        }
        break;
    case KF_MAP_CELL_X_GE_Z:
        if (position->vx % KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER
                >= position->vz % KF_MAP_TILE_SIZE) {
            break;
        }
        return KF_COLLISION_TERRAIN;
    case KF_MAP_CELL_SUM_LE_SIZE:
        if ((KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER)
                < position->vx % KF_MAP_TILE_SIZE + position->vz % KF_MAP_TILE_SIZE) {
            return KF_COLLISION_TERRAIN;
        }
        break;
    case KF_MAP_CELL_Z_GE_X:
        if (position->vx % KF_MAP_TILE_SIZE
                > position->vz % KF_MAP_TILE_SIZE + KF_MAP_TILE_CENTER) {
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
    switch (effect->type & KF_EFFECT_COLLISION_TARGETS_MASK) {
    case KF_EFFECT_COLLISION_TARGET_ACTORS:
        return collision_query_world(position->vx, position->vy, position->vz, radius, 0,
            KF_COLLISION_SKIP_TERRAIN | KF_COLLISION_SKIP_PLAYER
                | KF_COLLISION_SKIP_MAP_OBJECTS | KF_COLLISION_SKIP_MAP_EVENTS);
    case KF_EFFECT_COLLISION_TARGET_PLAYER:
        return collision_query_world(position->vx, position->vy, position->vz, radius, 0,
            KF_COLLISION_SKIP_TERRAIN | KF_COLLISION_SKIP_ACTORS
                | KF_COLLISION_SKIP_MAP_OBJECTS | KF_COLLISION_SKIP_MAP_EVENTS);
    case KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER:
        return collision_query_world(position->vx, position->vy, position->vz, radius, 0,
            KF_COLLISION_SKIP_TERRAIN | KF_COLLISION_SKIP_MAP_OBJECTS
                | KF_COLLISION_SKIP_MAP_EVENTS);
    default:
        return 1;
    }
}
