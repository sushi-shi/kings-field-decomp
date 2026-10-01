#include <kf/platform/frame_task.hpp>
#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/game/player.h>
#include <kf/game/menu_glyphs.h>
#include <kf/lib/null.h>

#include <kf/platform/input.h>
#include <kf/game/menu.h>
#include <kf/game/game.h>
#include <kf/game/player_actions.h>
#include <kf/platform/prelude.h>
#include <kf/game/menu_text.h>
#include <array>
static constexpr unsigned MENU_SELECTION_LIST_CAPACITY = 20;


kf::FrameTask<void> menu_equip_select(PlayerContext &player, KfEquipmentMenuCategory equipment_category)
{
    KfMenuList ctx;
    std::array<KfObjectId, MENU_SELECTION_LIST_CAPACITY> item_ids;
    s32 item_id;
    s32 found;
    s32 start;
    s32 end;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);

    (co_await game_wait_buttons_released());

    auto &player_stock = player.item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)];
    switch (equipment_category) {
    case KF_EQUIP_MENU_NONE:
    case KF_EQUIP_MENU_MAGIC:
        // No item range exists for these; magic has its own selector.
        co_return;
    case KF_EQUIP_MENU_WEAPON:
        start = kf_enum_encode<u8>(KF_ITEM_SHORT_SWORD);
        end = kf_enum_encode<u8>(KF_ITEM_IRON_MASK);
        break;
    case KF_EQUIP_MENU_SHIELD:
        start = kf_enum_encode<u8>(KF_ITEM_SMALL_SHIELD);
        end = kf_enum_encode<u8>(KF_ITEM_GAUNTLET);
        break;
    case KF_EQUIP_MENU_HEAD:
        start = kf_enum_encode<u8>(KF_ITEM_IRON_MASK);
        end = kf_enum_encode<u8>(KF_ITEM_BREASTPLATE);
        break;
    case KF_EQUIP_MENU_BODY:
        start = kf_enum_encode<u8>(KF_ITEM_BREASTPLATE);
        end = kf_enum_encode<u8>(KF_ITEM_SMALL_SHIELD);
        break;
    case KF_EQUIP_MENU_ARM:
        start = kf_enum_encode<u8>(KF_ITEM_GAUNTLET);
        end = kf_enum_encode<u8>(KF_ITEM_IRON_BOOTS);
        break;
    case KF_EQUIP_MENU_LEG:
        start = kf_enum_encode<u8>(KF_ITEM_IRON_BOOTS);
        end = kf_enum_encode<u8>(KF_ITEM_GOLD_COIN);
        break;
    case KF_EQUIP_MENU_ACCESSORY:
        start = kf_enum_encode<u8>(KF_ITEM_LIGHT_RING);
        end = kf_enum_encode<u8>(KF_ITEM_GOLD_CROSS);
        break;
    }

    found = 0;
    for (item_id = start; item_id < end; item_id++) {
        if (player_stock[item_id] != 0) {
            item_ids[found] = kf_enum_decode<KfObjectId>(item_id);
            found++;
        }
    }
    item_ids[found] = KF_OBJECT_NONE;
    found++;

    menu_list_init(&ctx, KF_MENU_WINDOW_EQUIPMENT, kf_enum_encode<s32>(equipment_category));
    ctx.entry_count = found;
    ctx.entries = std::span<const KfObjectId>(item_ids).first(found);
    ctx.quantities = {};

    if (ctx.entry_count != 0) {
        if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
            co_return;
    }

    for (;;) {
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            if ((co_await menu_list_confirm(player, &ctx, KF_MENU_CONFIRM_EQUIP,
                    KF_MENU_PREVIEW_ITEM_MODEL, item_ids[ctx.selected_index], KF_ITEM_STOCK_PLAYER, KF_TRADE_BUY))
                    == KF_MENU_RESULT_CANCELLED)
                selection = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);
            else
                selection = kf_enum_encode<u8>(item_ids[ctx.selected_index]);
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (selection != kf_enum_encode<s32>(KF_MENU_RESULT_PENDING)) {
            (co_await game_wait_buttons_released());
            break;
        }

        prev = input;
        input = kf::host_read_buttons();
        if (ctx.entry_count == 0) {
            if (input != 0) {
                (co_await menu_play_input_sound(MENU_SOUND_CURSOR));
                selection = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
            }
        } else if ((co_await menu_list_handle_navigation(ctx, input, prev))) {
            if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
                co_return;
        } else if (kf::button_pressed(input, prev, kf::Button::Confirm)) {
            (co_await menu_play_input_sound(MENU_SOUND_CONFIRM));
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if (kf::button_pressed(input, prev, kf::Button::Back)) {
            (co_await menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR));
            selection = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_item_model_preview(player, item_ids[ctx.selected_index]);
        menu_list_render(&ctx);
        (co_await menu_present_frame());
    }

    menu_release_item_model();
    if (selection != kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED)) {
        if (player.actions)
            co_await player_request_action(player, kf::net::CommandKind::Equip,
                static_cast<u16>(selection), kf_enum_encode<u16>(equipment_category));
        else player_equip_item(player, equipment_category, kf_enum_decode<KfObjectId>(selection));
    }
}

kf::FrameTask<void> menu_spell_select(WorldState &world, PlayerContext &player)
{
    KfMenuList ctx;
    std::array<KfEffectKind, MENU_SELECTION_LIST_CAPACITY> magic_ids;
    s32 magic_id;
    s32 found;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);

    (co_await game_wait_buttons_released());

    found = 0;
    for (magic_id = kf_enum_encode<s32>(KF_MAGIC_LIGHTNING_BOLT); magic_id < KF_MAGIC_PLAYER_COUNT; magic_id++) {
        if (player.learned_magic[magic_id] == KF_MAGIC_LEARNED) {
            magic_ids[found] = kf_enum_decode<KfEffectKind>(magic_id);
            found++;
        }
    }
    magic_ids[found] = KF_MAGIC_NONE;
    found++;

    menu_list_init(&ctx, KF_MENU_WINDOW_EQUIPMENT, kf_enum_encode<s32>(KF_EQUIP_MENU_MAGIC));
    ctx.entry_count = found;
    ctx.entries = std::span<const KfEffectKind>(magic_ids).first(found);
    ctx.quantities = {};

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_texture(menu_texture_from_magic(magic_ids[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED)
            co_return;
        if (magic_ids[ctx.selected_index] != KF_MAGIC_NONE)
            menu_add_magic_artwork_quad();
    }
    menu_list_render(&ctx);

    for (;;) {
        (co_await menu_present_frame());
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            selection = kf_enum_encode<s32>((co_await menu_list_confirm(player, &ctx, KF_MENU_CONFIRM_EQUIP, magic_ids[ctx.selected_index])));
            if (selection == kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED))
                selection = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);
            else
                selection = ctx.selected_index;
        }
        if (selection != kf_enum_encode<s32>(KF_MENU_RESULT_PENDING)) {
            (co_await game_wait_buttons_released());
            break;
        }

        menu_frame_begin();
        confirm = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = kf::host_read_buttons();
        if (ctx.entry_count == 0) {
            if (input != 0) {
                (co_await menu_play_input_sound(MENU_SOUND_CURSOR));
                selection = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
            }
        } else if ((co_await menu_list_handle_navigation(ctx, input, prev))) {
            if (menu_load_texture(menu_texture_from_magic(magic_ids[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED)
                co_return;
        } else if (kf::button_pressed(input, prev, kf::Button::Confirm)) {
            (co_await menu_play_input_sound(MENU_SOUND_CONFIRM));
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if (kf::button_pressed(input, prev, kf::Button::Back)) {
            (co_await menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR));
            selection = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
        }

        if (ctx.entry_count != 0) {
            if (magic_ids[ctx.selected_index] != KF_MAGIC_NONE)
                menu_add_magic_artwork_quad();
        }
        menu_list_render(&ctx);
    }

    if (selection != kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED)) {
        if (player.actions)
            co_await player_request_action(player, kf::net::CommandKind::SelectMagic,
                kf_enum_encode<u16>(magic_ids[selection]));
        else player_select_magic(world, player, magic_ids[selection]);
    }
}
