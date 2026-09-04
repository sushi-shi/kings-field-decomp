#include <kf/address.h>
#include <kf/game_collision.h>
#include <kf/game.h>

ADDRESS(0x8001a29c, 0x1b0)
s32 map_floor_height_for_cell_position(
    u16 cell_index, s32 point_x, s32 point_z)
{
    s32 floor_height = -(map_floor_height_grid[0][cell_index] * 100);

    if (map_collision_grid[0][cell_index] == 6) {
        switch (map_cell_orientation_grid[0][cell_index]) {
        case 1:
            if (point_x % 2000 > 1000) {
                floor_height += 300;
            }
            break;
        case 2:
            if (point_z % 2000 < 1000) {
                floor_height += 300;
            }
            break;
        case 3:
            if (point_x % 2000 < 1000) {
                floor_height += 300;
            }
            break;
        case 4:
            if (point_z % 2000 > 1000) {
                floor_height += 300;
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
        (point_z / 2000) * 100 + point_x / 2000,
        point_x,
        point_z);
}

ADDRESS(0x8001a4e8, 0xc4)
void collision_adjust_cell_occupancy(u16 cell_x, u16 cell_z, s32 delta)
{
    u16 first_x;
    s16 rows;
    u8 *next_row;

    cell_x -= 2;
    first_x = cell_x;
    cell_z -= 2;
    next_row = &map_collision_flag_grid[(s16)cell_z][(s16)cell_x];
    rows = 5;

    do {
        u8 *cell = next_row;

        next_row = cell + 100;

        if (cell_z < 100) {
            u16 x = first_x;
            s16 columns = 5;

            do {
                if (x < 100) {
                    *cell = (*cell & 0xe0) | ((*cell + delta) & 0x1f);
                }
                x++;
                cell++;
            } while (--columns != 0);
        }
        cell_z++;
    } while (--rows != 0);
}
