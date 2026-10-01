#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/game/player.h>
#include <stdarg.h>

#include <kf/game/menu.h>
#include <kf/game/game.h>

void func_80036e30(void)
{

}

static kf::FrameTask<u32> menu_enter_mode_impl(WorldState &world, PlayerContext &player, KfMenuMode menu_mode, int argument)
{
    struct MenuModelScope { ~MenuModelScope() { menu_release_item_model(); } } model_scope;
    u32 result;
    kf::InputContextScope input_context(kf::InputContext::Menu);

    animation_cache_release_all();
    switch (menu_mode) {
    case KF_MENU_MODE_ROOT:
        result = (co_await menu_root(world, player));
        break;
    case KF_MENU_MODE_ITEM_PICKUP: {
        KfObjectId item_id;

        item_id = kf_enum_decode<KfObjectId>(argument);
        result = kf_enum_encode<u32>((co_await item_pickup_confirm(player, item_id)));
        break;
    }
    case KF_MENU_MODE_SHOP: {
        KfItemStockBank shop_bank;

        shop_bank = kf_enum_decode<KfItemStockBank>(argument);
        (co_await shop_menu_root(player, shop_bank));
        result = 0;
        break;
    }
    }
    player_clear_motion(player);

    co_return result;
}

kf::FrameTask<u32> menu_enter_mode(WorldState &world, PlayerContext &player, KfMenuMode mode) { co_return (co_await menu_enter_mode_impl(world, player, mode, 0)); }
kf::FrameTask<u32> menu_enter_mode(WorldState &world, PlayerContext &player, KfMenuMode mode, KfObjectId id)
{ co_return (co_await menu_enter_mode_impl(world, player, mode, static_cast<int>(id))); }
kf::FrameTask<u32> menu_enter_mode(WorldState &world, PlayerContext &player, KfMenuMode mode, KfItemStockBank bank)
{ co_return (co_await menu_enter_mode_impl(world, player, mode, static_cast<int>(bank))); }
