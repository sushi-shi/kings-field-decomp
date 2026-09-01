#include <kf/address.h>
#include <kf/game_types.h>

extern u16 player_status_effect_flags;
extern s16 player_status_effect4_timer;

ADDRESS(0x80018858, 0x28)
void player_status_apply_effect4(void)
{
    player_status_effect4_timer = 500;
    player_status_effect_flags |= 0x10;
}
