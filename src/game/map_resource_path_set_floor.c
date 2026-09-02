#include <kf/address.h>
#include <kf/game_types.h>

extern char map_resource_path[12];

ADDRESS(0x8001b390, 0x14)
void map_resource_path_set_floor(u8 floor)
{
    map_resource_path[1] = floor + '0';
}
