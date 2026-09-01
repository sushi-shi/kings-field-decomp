#include <kf/address.h>
#include <kf/game_types.h>

extern u8 map_collision_flag_grid[100][100];

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
