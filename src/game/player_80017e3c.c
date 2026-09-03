#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

ADDRESS(0x80017e3c, 0xa0)
void player_update_transform_snapshot(VECTOR *position_out, SVECTOR *rotation_out)
{
    *position_out = player_state.camera_position;
    *rotation_out = player_state.camera_rotation;
    rotation_out->vx += player_state.view_rotation_offset.vx;
    rotation_out->vy += player_state.view_rotation_offset.vy;
    rotation_out->vz += player_state.view_rotation_offset.vz;
}
