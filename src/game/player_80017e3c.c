#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;

ADDRESS(0x80017e3c, 0xa0)
void player_update_transform_snapshot(struct KfVec4i *position_out, struct KfVec4s *rotation_out)
{
    *position_out = player_state.camera_position;
    *rotation_out = player_state.camera_rotation;
    rotation_out->x += player_state.view_rotation_offset.x;
    rotation_out->y += player_state.view_rotation_offset.y;
    rotation_out->z += player_state.view_rotation_offset.z;
}
