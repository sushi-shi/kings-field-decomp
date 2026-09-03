#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * func_80036e38 (GAME.EXE): tear down and re-establish the system heap around a
 * mode-selected reload. It drains the GPU (DrawSync), runs pool_release_all, resets
 * the system heap, dispatches on the mode argument, resets the heap again, and
 * clears the player motion state. Callers pass a variable number of arguments,
 * so the incoming register arguments are spilled to their home slots.
 */

extern void item_menu_root(u32 arg0);

ADDRESS(0x80036e38, 0xc8)
u32 func_80036e38(s32 mode, ...)
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
