#include <kf/platform/frame_task.hpp>
#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/platform/prelude.h>
#include <kf/game/player.h>

kf::FrameTask<void> player_warp_shimmer_at_player(WorldState &world, PlayerContext &player, KfWarpShimmerMode shimmer_mode)
{
    VECTOR position{player.state.camera_position.vx, player.state.foot_height,
        player.state.camera_position.vz};
    (co_await player_warp_shimmer(world, player, shimmer_mode, &position));
}
