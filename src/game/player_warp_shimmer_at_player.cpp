#include <kf/game_player.h>

void player_warp_shimmer_at_player(KfWarpShimmerMode shimmer_mode)
{
    VECTOR position;

    position.vx = player_state.camera_position.vx;
    position.vz = player_state.camera_position.vz;
    position.vy = player_state.floor_height;
    player_warp_shimmer(shimmer_mode, &position);
}
