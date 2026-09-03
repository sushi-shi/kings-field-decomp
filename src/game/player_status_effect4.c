#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

ADDRESS(0x80018858, 0x28)
void player_status_apply_effect4(void)
{
    player_state.status_effect4_timer = 500;
    player_state.status_effect_flags |= 0x10;
}
