#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

ADDRESS(0x80014674, 0x44)
void player_warp_shimmer_at_player(u32 mode)
{
    struct KfVec3i position;

    position.x = player_state.camera_position.vx;
    position.z = player_state.camera_position.vz;
    position.y = player_state.floor_height;
    player_warp_shimmer(mode, &position);
}
