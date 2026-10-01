#include <kf/platform/frame_task.hpp>
#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/game/player.h>
#include <kf/lib/null.h>

#include <kf/platform/input.h>
#include <kf/game/menu.h>
#include <kf/platform/prelude.h>
#include <kf/game/game.h>

#include <array>
#include <optional>

static constexpr unsigned MENU_INVENTORY_LABEL_CAPACITY = 50;
static constexpr unsigned MENU_INVENTORY_ENTRY_CAPACITY = 56;


kf::FrameTask<std::optional<KfObjectId>> menu_use_item_panel(PlayerContext &player);

kf::FrameTask<void> menu_save_confirm(WorldState &world, PlayerContext &player)
{
    kf::InputContextScope input_context(kf::InputContext::Menu);
    s32 i;

    i = 0;
    do {
        i++;
        menu_frame_begin();
        menu_draw_save_slots({}, KF_SAVE_OVERLAY_ALL);
        menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, 0, KF_MENU_CONFIRM_IDLE);
        (co_await menu_present_frame());
    } while (i < 3);
    (co_await menu_play_input_sound(MENU_SOUND_CURSOR));
    (co_await game_wait_buttons_released());
    (co_await menu_save_panel(world, player));

}

kf::FrameTask<KfMenuOutcome> menu_root(WorldState &world, PlayerContext &player)
{
    s32 cursor = 0;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    std::optional<KfMenuOutcome> result;
    KfMenuRootChoice selection = KF_ROOT_CHOICE_NONE;
    s32 i;

    i = 0;
    do {
        i++;
        menu_frame_begin();
        menu_draw_status_summary(player);
        menu_draw_window(KF_MENU_WINDOW_ROOT, KF_MENU_ROOT_ROW_COUNT, cursor, confirm);
        (co_await menu_present_frame());
    } while (i < 3);
    (co_await menu_play_input_sound(MENU_SOUND_CURSOR));
    (co_await game_wait_buttons_released());

    for (;;) {
        if (selection != KF_ROOT_CHOICE_NONE || (result && *result == KfMenuOutcome{KfMenuAction::Close})) {
            menu_frame_begin();
            menu_draw_status_summary(player);
            menu_draw_window(KF_MENU_WINDOW_ROOT, KF_MENU_ROOT_ROW_COUNT, cursor, confirm);
            (co_await menu_present_frame());
            (co_await game_wait_buttons_released());
        }
        switch (selection) {
        case KF_ROOT_CHOICE_NONE:
            break;
        case KF_ROOT_CHOICE_USE_ITEM:
            if (const auto item = (co_await menu_use_item_panel(player)))
                result = *item;
            break;
        case KF_ROOT_CHOICE_USE_MAGIC:
            if ((co_await menu_magic_panel(world, player)))
                result = KfMenuOutcome{KfMenuAction::Close};
            break;
        case KF_ROOT_CHOICE_EQUIPMENT:
            (co_await menu_equipment_root(world, player));
            break;
        case KF_ROOT_CHOICE_STATUS:
            (co_await menu_status_panel(player));
            break;
        case KF_ROOT_CHOICE_DROP_ITEM:
            (co_await menu_drop_item_panel(player));
            break;
        case KF_ROOT_CHOICE_SYSTEM: {
            const auto action = (co_await menu_system_panel(world, player));
            if (action != KfMenuAction::Close)
                result = KfMenuOutcome{action};
            break;
        }
        case KF_ROOT_CHOICE_CONFIG:
            (co_await menu_config_panel(player));
            break;
        }
        if (result) {
            selection = KF_ROOT_CHOICE_NONE;
            (co_await game_wait_buttons_released());
            co_return *result;
        }
        selection = KF_ROOT_CHOICE_NONE;
        confirm = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = kf::host_read_buttons();
        if (kf::button_pressed(input, prev, kf::Button::Up)) {
            (co_await menu_play_input_sound(MENU_SOUND_CURSOR));
            if (cursor != 0)
                cursor--;
            else
                cursor = KF_MENU_ROOT_RETURN_ROW;
        } else if (kf::button_pressed(input, prev, kf::Button::Down)) {
            (co_await menu_play_input_sound(MENU_SOUND_CURSOR));
            if (cursor != KF_MENU_ROOT_RETURN_ROW)
                cursor++;
            else
                cursor = 0;
        } else if (kf::button_pressed(input, prev, kf::Button::Confirm)) {
            (co_await menu_play_input_sound(MENU_SOUND_CONFIRM));
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (cursor < KF_MENU_ROOT_RETURN_ROW)
                selection = kf_enum_decode<KfMenuRootChoice>(cursor);
            else
                result = KfMenuOutcome{KfMenuAction::Close};
        } else if (kf::button_pressed(input, prev, kf::Button::Back)) {
            (co_await menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR));
            result = KfMenuOutcome{KfMenuAction::Close};
        }
        menu_frame_begin();
        menu_draw_status_summary(player);
        menu_draw_window(KF_MENU_WINDOW_ROOT, KF_MENU_ROOT_ROW_COUNT, cursor, confirm);
        (co_await menu_present_frame());
    }
}

kf::FrameTask<std::optional<KfObjectId>> menu_use_item_panel(PlayerContext &player)
{
    KfMenuList ctx;
    std::array<std::array<s16, MENU_GLYPHS_PER_ROW>, MENU_INVENTORY_LABEL_CAPACITY> labels;
    std::array<u8, MENU_INVENTORY_ENTRY_CAPACITY> quantities;
    std::array<KfObjectId, MENU_INVENTORY_ENTRY_CAPACITY> item_ids;
    s32 found;
    s32 item_id;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    std::optional<KfObjectId> selection;
    KfMenuResult result = KF_MENU_RESULT_PENDING;

    (co_await game_wait_buttons_released());
    menu_list_init(&ctx, KF_MENU_WINDOW_ROOT, kf_enum_encode<s32>(KF_ROOT_CHOICE_USE_ITEM));

    auto &player_stock = player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)];
    found = 0;
    if (player_stock[kf_enum_encode<u8>(KF_ITEM_WATCHMAN_MAP)] != 0) {
        labels[found] = item_name_rows[kf_enum_encode<u8>(KF_ITEM_WATCHMAN_MAP)].codes;
        quantities[found] = player_stock[kf_enum_encode<u8>(KF_ITEM_WATCHMAN_MAP)];
        item_ids[found] = KF_ITEM_WATCHMAN_MAP;
        found++;
    }
    if (player_stock[kf_enum_encode<u8>(KF_ITEM_SORCERER_MAP)] != 0) {
        labels[found] = item_name_rows[kf_enum_encode<u8>(KF_ITEM_SORCERER_MAP)].codes;
        quantities[found] = player_stock[kf_enum_encode<u8>(KF_ITEM_SORCERER_MAP)];
        item_ids[found] = KF_ITEM_SORCERER_MAP;
        found++;
    }
    for (item_id = kf_enum_encode<s32>(KF_ITEM_VERDITE); item_id < kf_enum_encode<s32>(KF_ITEM_LIGHT_RING); item_id++) {
        if (item_id != kf_enum_encode<s32>(KF_ITEM_WATCHMAN_MAP) && item_id != kf_enum_encode<s32>(KF_ITEM_SORCERER_MAP) && player_stock[item_id] != 0) {
            labels[found] = item_name_rows[item_id].codes;
            quantities[found] = player_stock[item_id];
            item_ids[found] = kf_enum_decode<KfObjectId>(item_id);
            found++;
        }
    }
    for (item_id = kf_enum_encode<s32>(KF_ITEM_GOLD_CROSS); item_id < KF_ITEM_COUNT; item_id++) {
        if (item_id != kf_enum_encode<s32>(KF_ITEM_WATCHMAN_MAP) && item_id != kf_enum_encode<s32>(KF_ITEM_SORCERER_MAP) && player_stock[item_id] != 0) {
            labels[found] = item_name_rows[item_id].codes;
            quantities[found] = player_stock[item_id];
            item_ids[found] = kf_enum_decode<KfObjectId>(item_id);
            found++;
        }
    }
    ctx.entry_count = found;
    ctx.glyph_rows = labels;
    ctx.quantities = quantities;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
            co_return std::nullopt;
        menu_item_model_preview(player, item_ids[ctx.selected_index]);
    }
    menu_list_render(&ctx);
    (co_await menu_present_frame());

    for (;;) {
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            if ((co_await menu_list_confirm(player, &ctx, KF_MENU_CONFIRM_USE,
                    KF_MENU_PREVIEW_ITEM_MODEL, item_ids[ctx.selected_index], KF_ITEM_STOCK_PLAYER, KF_TRADE_BUY))
                    == KF_MENU_RESULT_CANCELLED)
                result = KF_MENU_RESULT_PENDING;
            else {
                selection = item_ids[ctx.selected_index];
                result = KF_MENU_RESULT_ACCEPTED;
            }
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (result != KF_MENU_RESULT_PENDING) {
            (co_await game_wait_buttons_released());
            break;
        }

        prev = input;
        input = kf::host_read_buttons();
        if (ctx.entry_count == 0) {
            if (input != 0) {
                (co_await menu_play_input_sound(MENU_SOUND_CURSOR));
                result = KF_MENU_RESULT_CANCELLED;
            }
        } else if ((co_await menu_list_handle_navigation(ctx, input, prev))) {
            if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
                co_return std::nullopt;
        } else if (kf::button_pressed(input, prev, kf::Button::Confirm)) {
            (co_await menu_play_input_sound(MENU_SOUND_CONFIRM));
            if (item_ids[ctx.selected_index] == KF_ITEM_WATCHMAN_MAP || item_ids[ctx.selected_index] == KF_ITEM_SORCERER_MAP) {
                menu_release_item_model();
                (co_await menu_map_viewer(player, item_ids[ctx.selected_index]));
                (co_await menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR));
                if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
                    co_return std::nullopt;
            } else {
                confirm = KF_MENU_CONFIRM_REQUESTED;
            }
        } else if (kf::button_pressed(input, prev, kf::Button::Back)) {
            (co_await menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR));
            result = KF_MENU_RESULT_CANCELLED;
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_item_model_preview(player, item_ids[ctx.selected_index]);
        menu_list_render(&ctx);
        (co_await menu_present_frame());
    }

    menu_release_item_model();
    co_return selection;
}
