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
u32 menu_enter_mode(KfMenuMode mode, ...)
{
    u32 result;

    DrawSync(0);
    pool_release_all();
    memory_reset_system_heap();
    switch (mode) {
    case KF_MENU_MODE_ROOT:
        result = menu_root();
        break;
    case KF_MENU_MODE_ITEM_PICKUP: {
#if KF_MODERN_TYPES
        __builtin_va_list arguments;
        KfItemId item_id;

        __builtin_va_start(arguments, mode);
        item_id = KF_ENUM_DECODE(KfItemId, __builtin_va_arg(arguments, int));
        __builtin_va_end(arguments);
#else
        /* The pinned compiler spills mode and the optional argument to homes. */
        KfItemId item_id = KF_ENUM_DECODE(KfItemId, *((u8 *)&mode + sizeof(mode)));
#endif
        result = KF_ENUM_ENCODE(u32, item_pickup_confirm(item_id));
        break;
    }
    case KF_MENU_MODE_SHOP: {
#if KF_MODERN_TYPES
        __builtin_va_list arguments;
        KfShopId shop_id;

        __builtin_va_start(arguments, mode);
        shop_id = KF_ENUM_DECODE(KfShopId, __builtin_va_arg(arguments, int));
        __builtin_va_end(arguments);
#else
        KfShopId shop_id = KF_ENUM_DECODE(KfShopId, *((u8 *)&mode + sizeof(mode)));
#endif
        item_menu_root(shop_id);
        result = 0;
        break;
    }
    }
    memory_reset_system_heap();
    player_clear_motion();
    return result;
}
