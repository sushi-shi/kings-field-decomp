#include <kf/platform/prelude.h>
#include <kf/platform/frame_task.hpp>
#include <kf/game/game.h>
#include <kf/game/menu.h>

namespace {
class MenuSession {
public:
    explicit MenuSession(PlayerContext &player) : player(player), input(kf::InputContext::Menu) {
        animation_cache_release_all();
    }
    ~MenuSession() { menu_release_item_model(); player_clear_motion(player); }
private:
    PlayerContext &player;
    kf::InputContextScope input;
};
}

kf::FrameTask<KfMenuOutcome> menu_open_root(WorldState &world, PlayerContext &player) {
    const MenuSession session(player);
    co_return co_await menu_root(world, player);
}

kf::FrameTask<KfMenuResult> menu_confirm_pickup(PlayerContext &player, KfObjectId item) {
    const MenuSession session(player);
    co_return co_await item_pickup_confirm(player, item);
}

kf::FrameTask<void> menu_open_shop(PlayerContext &player, KfItemStockBank bank) {
    const MenuSession session(player);
    co_await shop_menu_root(player, bank);
}
