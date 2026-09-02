#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;

ADDRESS(0x80017a24, 0x5c)
void player_update_view_bob(void)
{
    s32 phase;

    if (player_state.vertical_state == 0) {
        phase = (player_state.view_bob_phase + player_state.motion_state.movement_speed * 2)
            & 0xfff;
        player_state.view_bob_phase = phase;
        player_state.view_bob_offset = rsin(phase) >> 6;
    }
}
