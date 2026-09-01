#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfPlayerState player_state;
extern void func_8001fa44(s32 arg0);
extern void player_recalculate_combat_stats(void);

ADDRESS(0x80015f28, 0x98)
void player_increment_physical_power_training(void)
{
    player_state.physical_power_training++;
    if (player_state.physical_power_training >= 100) {
        player_state.base_physical_power++;
        player_state.physical_power_training = 0;
        if (player_state.base_physical_power >= 1000) {
            player_state.base_physical_power = 999;
        } else {
            func_8001fa44(0x1e);
        }
        player_recalculate_combat_stats();
    }
}

ADDRESS(0x80015fc0, 0x98)
void player_increment_magic_training(void)
{
    player_state.magic_training++;
    if (player_state.magic_training >= 100) {
        player_state.base_magic++;
        player_state.magic_training = 0;
        if (player_state.base_magic >= 1000) {
            player_state.base_magic = 999;
        } else {
            func_8001fa44(0x1f);
        }
        player_recalculate_combat_stats();
    }
}
