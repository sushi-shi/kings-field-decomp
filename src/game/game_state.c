#include <kf/address.h>
#include <kf/game_types.h>
#include <kf/game.h>

extern void tmd_release_last_allocation(s32 slot);

ADDRESS(0x8002af0c, 0x3c)
void game_state_acknowledge_pending(void)
{
    if (pending_game_state == 1) {
        tmd_release_last_allocation(4);
        pending_game_state = 0;
    }
}
