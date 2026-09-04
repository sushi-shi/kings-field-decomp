#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

/*
 * menu_enter_mode (GAME.EXE): tear down and re-establish the system heap around a
 * mode-selected reload. It drains the GPU (DrawSync), runs pool_release_all, resets
 * the system heap, dispatches on the mode argument, resets the heap again, and
 * clears the player motion state. Callers pass a variable number of arguments,
 * so the incoming register arguments are spilled to their home slots.
 */

ADDRESS(0x80036e30, 0x8)
void func_80036e30(void)
{
    /* Unreferenced return stub; purpose and original owner are unresolved. */
}

ADDRESS(0x80036e38, 0xc8)
u32 menu_enter_mode(s32 mode, ...)
{
    u32 result;

    DrawSync(0);
    pool_release_all();
    memory_reset_system_heap();
    switch (mode) {
    case 0:
        result = menu_root();
        break;
    case 1:
        result = item_use_confirm(*((u8 *)&mode + 4));
        break;
    case 2:
        item_menu_root(*((u8 *)&mode + 4));
        result = 0;
        break;
    }
    memory_reset_system_heap();
    player_clear_motion();
    return result;
}
