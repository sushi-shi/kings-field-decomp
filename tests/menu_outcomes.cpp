#include <kf/platform/prelude.h>
#include <kf/game/world.h>
#include "../src/game/menu_enter_mode.cpp"
#include <cassert>
#include <type_traits>

static_assert(!std::is_constructible_v<KfMenuOutcome, KfMenuAction, KfObjectId>);
static_assert(!std::is_copy_constructible_v<MenuSession>);

static kf::InputContext context = kf::InputContext::Gameplay;
static unsigned releases, clears, model_releases;
static PlayerContext *expected_player;
static WorldState *expected_world;
static KfMenuOutcome next_outcome{KfMenuAction::Close};
static KfMenuResult pickup_result;
static KfObjectId picked_item;
static KfItemStockBank opened_shop;

namespace kf {
InputContext host_set_input_context(InputContext next)
{
    const auto previous = context;
    context = next;
    return previous;
}
}
void animation_cache_release_all()
{
    assert(context == kf::InputContext::Menu);
    ++releases;
}
void player_clear_motion(PlayerContext &player)
{
    assert(context == kf::InputContext::Menu);
    assert(&player == expected_player && model_releases == releases);
    ++clears;
}
void menu_release_item_model()
{
    assert(context == kf::InputContext::Menu);
    ++model_releases;
}
kf::FrameTask<KfMenuOutcome> menu_root(WorldState &world, PlayerContext &player)
{
    assert(context == kf::InputContext::Menu);
    assert(&world == expected_world && &player == expected_player);
    assert(releases == clears + 1);
    co_await kf::FrameDelay{1};
    assert(context == kf::InputContext::Menu);
    co_return next_outcome;
}
kf::FrameTask<KfMenuResult> item_pickup_confirm(PlayerContext &player, KfObjectId item)
{
    assert(context == kf::InputContext::Menu);
    assert(&player == expected_player);
    picked_item = item;
    co_await kf::FrameDelay{1};
    co_return pickup_result;
}
kf::FrameTask<void> shop_menu_root(PlayerContext &player, KfItemStockBank bank)
{
    assert(context == kf::InputContext::Menu);
    assert(&player == expected_player);
    opened_shop = bank;
    co_await kf::FrameDelay{1};
}
template<class T> T complete(kf::FrameTask<T> task)
{
    task.advance();
    assert(!task.done() && context == kf::InputContext::Menu && releases == clears + 1);
    task.advance();
    assert(task.done());
    return task.await_resume();
}

int main()
{
    auto world = std::make_unique<WorldState>();
    auto &player = world->party.members[0].player;
    expected_world = world.get(); expected_player = &player;
    for (auto previous : {kf::InputContext::Gameplay, kf::InputContext::Scripted}) {
        context = previous;
        for (const KfMenuOutcome outcome : {KfMenuOutcome{KF_ITEM_SHORT_SWORD}, KfMenuOutcome{KfMenuAction::Close},
                KfMenuOutcome{KfMenuAction::GameLoaded}, KfMenuOutcome{KfMenuAction::ReturnToIntro}}) {
            next_outcome = outcome;
            const auto result = complete(menu_open_root(*world, player));
            assert(result == outcome);
            assert(context == previous && releases == clears);
        }
        for (auto result : {KF_MENU_RESULT_ACCEPTED, KF_MENU_RESULT_CANCELLED,
                            KF_MENU_RESULT_DECLINED, KF_MENU_RESULT_STACK_FULL}) {
            pickup_result = result;
            assert(complete(menu_confirm_pickup(player, KF_ITEM_SHORT_SWORD)) == result);
            assert(picked_item == KF_ITEM_SHORT_SWORD);
            assert(context == previous && releases == clears);
        }
        for (auto bank : {KF_ITEM_STOCK_FIRST_SHOP, KF_ITEM_STOCK_SECOND_SHOP}) {
            complete(menu_open_shop(player, bank));
            assert(opened_shop == bank);
            assert(context == previous && releases == clears);
        }
        auto cancelled = menu_open_root(*world, player);
        cancelled.advance();
        assert(context == kf::InputContext::Menu && releases == clears + 1);
        cancelled = {};
        assert(context == previous && releases == clears && releases == model_releases);
    }
}
