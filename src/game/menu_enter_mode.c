#include <stdarg.h>

#include <kf/game/menu.h>
#include <kf/game/game.h>

void func_80036e30(void)
{

}

u32 menu_enter_mode(KfMenuMode menu_mode, ...)
{
    u32 result;

    DrawSync(0);
    pool_release_all();
    memory_reset_system_heap();
    switch (menu_mode) {
    case KF_MENU_MODE_ROOT:
        result = menu_root();
        break;
    case KF_MENU_MODE_ITEM_PICKUP: {
        va_list arguments;
        KfObjectId item_id;

        va_start(arguments, menu_mode);
        item_id = ((KfObjectId)(va_arg(arguments, int)));
        va_end(arguments);
        result = ((u32)(item_pickup_confirm(item_id)));
        break;
    }
    case KF_MENU_MODE_SHOP: {
        va_list arguments;
        KfItemStockBank shop_bank;

        va_start(arguments, menu_mode);
        shop_bank = ((KfItemStockBank)(va_arg(arguments, int)));
        va_end(arguments);
        item_menu_root(shop_bank);
        result = 0;
        break;
    }
    }
    memory_reset_system_heap();
    player_clear_motion();
    return result;
}
