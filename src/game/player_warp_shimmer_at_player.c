#include <kf/address.h>
#include <kf/game_player.h>

ADDRESS(0x80014674, 0x44)
void player_warp_shimmer_at_player(u32 mode)
{
    VECTOR position;

    position.vx = player_state.camera_position.vx;
    position.vz = player_state.camera_position.vz;
    position.vy = player_state.floor_height;
    player_warp_shimmer(mode, &position);
}
