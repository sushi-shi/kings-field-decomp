#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
extern void player_warp_shimmer(u32 mode, struct KfVec3i *position);

ADDRESS(0x80014674, 0x44)
void func_80014674(u32 mode)
{
    struct KfVec3i position;

    position.x = player_state.camera_position.vx;
    position.z = player_state.camera_position.vz;
    position.y = player_state.floor_height;
    player_warp_shimmer(mode, &position);
}
