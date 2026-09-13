#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/game_collision.h>
#include <kf/game.h>

ADDRESS(0x8001a29c, 0x1b0)
s32 map_floor_height_for_cell_position(
    u16 cell_index, s32 point_x, s32 point_z)
{
    s32 floor_height = -(map_floor_height_grid.linear[cell_index] * KF_MAP_HEIGHT_STEP);

    if (map_collision_grid.linear[cell_index] == KF_MAP_CELL_STEP) {
        switch (map_cell_orientation_grid.linear[cell_index]) {
        case KF_MAP_ORIENT_UNROTATED:
            if (point_x % KF_MAP_TILE_SIZE > KF_MAP_TILE_CENTER) {
                floor_height += KF_MAP_HALF_CELL_STEP_HEIGHT;
            }
            break;
        case KF_MAP_ORIENT_QUARTER_TURN:
            if (point_z % KF_MAP_TILE_SIZE < KF_MAP_TILE_CENTER) {
                floor_height += KF_MAP_HALF_CELL_STEP_HEIGHT;
            }
            break;
        case KF_MAP_ORIENT_HALF_TURN:
            if (point_x % KF_MAP_TILE_SIZE < KF_MAP_TILE_CENTER) {
                floor_height += KF_MAP_HALF_CELL_STEP_HEIGHT;
            }
            break;
        case KF_MAP_ORIENT_THREE_QUARTER_TURN:
            if (point_z % KF_MAP_TILE_SIZE > KF_MAP_TILE_CENTER) {
                floor_height += KF_MAP_HALF_CELL_STEP_HEIGHT;
            }
            break;
        }
    }
    return floor_height;
}

ADDRESS(0x8001a44c, 0x9c)
s32 map_floor_height_at_position(const VECTOR *position)
{
    s32 point_z = position->vz;
    s32 point_x = position->vx;

    return map_floor_height_for_cell_position(
        (point_z / KF_MAP_TILE_SIZE) * KF_MAP_COLUMNS + point_x / KF_MAP_TILE_SIZE,
        point_x,
        point_z);
}

ADDRESS(0x8001a4e8, 0xc4)
void collision_adjust_cell_occupancy(u16 cell_x, u16 cell_z, s32 delta)
{
    u16 first_x;
    s16 rows;
    u8 *next_row;

    cell_x -= KF_OCCUPANCY_CELL_RADIUS;
    first_x = cell_x;
    cell_z -= KF_OCCUPANCY_CELL_RADIUS;
    next_row = &map_collision_flag_grid.cells[(s16)cell_z][(s16)cell_x];
    rows = KF_OCCUPANCY_CELL_SPAN;

    do {
        u8 *cell = next_row;

        next_row = cell + KF_MAP_COLUMNS;

        if (cell_z < KF_MAP_ROWS) {
            u16 x = first_x;
            s16 columns = KF_OCCUPANCY_CELL_SPAN;

            do {
                if (x < KF_MAP_COLUMNS) {
                    *cell = (*cell & KF_CELL_PRESERVED_FLAGS_MASK) | ((*cell + delta) & KF_CELL_OCCUPANT_COUNT_MASK);
                }
                x++;
                cell++;
            } while (--columns != 0);
        }
        cell_z++;
    } while (--rows != 0);
}
