#include <stdarg.h>

#include <kf/game_menu.h>
#include <kf/game.h>

void func_80036e30(void)
{

}

static u32 menu_enter_mode_impl(KfMenuMode menu_mode, int argument)
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
        KfObjectId item_id;

        item_id = kf_enum_decode<KfObjectId>(argument);
        result = kf_enum_encode<u32>(item_pickup_confirm(item_id));
        break;
    }
    case KF_MENU_MODE_SHOP: {
        KfItemStockBank shop_bank;

        shop_bank = kf_enum_decode<KfItemStockBank>(argument);
        item_menu_root(shop_bank);
        result = 0;
        break;
    }
    }
    memory_reset_system_heap();
    player_clear_motion();
    return result;
}

u32 menu_enter_mode(KfMenuMode mode) { return menu_enter_mode_impl(mode, 0); }
u32 menu_enter_mode(KfMenuMode mode, KfObjectId id)
{ return menu_enter_mode_impl(mode, static_cast<int>(id)); }
u32 menu_enter_mode(KfMenuMode mode, KfItemStockBank bank)
{ return menu_enter_mode_impl(mode, static_cast<int>(bank)); }
