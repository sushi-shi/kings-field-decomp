#include <kf/address.h>
#include <kf/game_types.h>
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
