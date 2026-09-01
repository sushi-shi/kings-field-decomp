#include <kf/address.h>
#include <kf/game_types.h>

extern s32 pending_game_state;
extern void func_8001c5ec(s32 value);

ADDRESS(0x8002af0c, 0x3c)
void game_state_acknowledge_pending(void)
{
    if (pending_game_state == 1) {
        func_8001c5ec(4);
        pending_game_state = 0;
    }
}
