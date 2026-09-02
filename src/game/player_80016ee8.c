#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
extern u8 map_floor_height_grid[100][100];
extern u8 DAT_80095064;
extern void player_clear_motion(void);
extern void collision_adjust_cell_occupancy(s32 object, s32 arg1, char arg2);

ADDRESS(0x80016ee8, 0x158)
void player_sync_position_to_map(void)
{
    s32 cell_x = player_state.camera_position.x / 2000;
    s32 cell_z = player_state.camera_position.z / 2000;
    s32 floor;
    s32 view_offset;
    s32 floor_height;

    player_state.unknown_58 = 0;
    player_state.map_cell.x = cell_x;
    player_state.map_cell.z = cell_z;
    floor = map_floor_height_grid[player_state.map_cell.z][player_state.map_cell.x];
    player_state.unknown_0d = 1;
    floor_height = -(floor * 100);
    view_offset = player_state.view_bob_offset - 1500;
    player_state.floor_height = floor_height;
    player_state.camera_position.y = view_offset + floor_height;
    player_clear_motion();
    collision_adjust_cell_occupancy(player_state.map_cell.x, player_state.map_cell.z, 1);
    DAT_80095064 = 0x56;
    player_state.vertical_state = 0;
    player_state.vertical_velocity = 0;
}
