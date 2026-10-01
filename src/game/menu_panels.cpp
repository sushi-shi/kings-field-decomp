#include <kf/game/system.h>
#include <kf/game/world.h>
#include <kf/game/player.h>
#include <kf/lib/null.h>

#include <kf/platform/input.hpp>
#include <kf/game/menu.h>
#include <kf/game/game.h>
#include <kf/game/player_actions.h>
static constexpr unsigned MENU_MAGIC_LABEL_CAPACITY = 10;
static constexpr unsigned MENU_MAGIC_ENTRY_CAPACITY = 16;



kf::FrameTask<KfMagicPanelResult> menu_magic_panel(WorldState &world, PlayerContext &player)
{
    KfMenuList ctx;
    s16 labels[MENU_MAGIC_LABEL_CAPACITY][MENU_GLYPHS_PER_ROW];
    KfEffectKind magic_ids[MENU_MAGIC_ENTRY_CAPACITY];
    s32 found;
    s32 magic_id;
    s32 j;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    KfMagicPanelResult selection = KF_MENU_RESULT_PENDING;

    (co_await game_wait_buttons_released());
    menu_list_init(&ctx, KF_MENU_WINDOW_ROOT, kf_enum_encode<s32>(KF_ROOT_CHOICE_USE_MAGIC));

    found = 0;
    for (magic_id = kf_enum_encode<s32>(KF_MAGIC_HEALING); magic_id < kf_enum_encode<s32>(KF_MAGIC_LIGHTNING_BOLT); magic_id++) {
        if (player.learned_magic[magic_id] == KF_MAGIC_LEARNED) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                labels[found][j] = magic_name_rows[magic_id].codes[j];
            magic_ids[found] = kf_enum_decode<KfEffectKind>(magic_id);
            found++;
        }
    }
    ctx.entry_count = found;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = NULL;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_texture(menu_texture_from_magic(magic_ids[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED)
            co_return KF_MENU_RESULT_CANCELLED;
        menu_add_magic_artwork_quad();
    }
    menu_list_render(&ctx);

    for (;;) {
        (co_await menu_present_frame());
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            if ((co_await menu_list_confirm(player, &ctx, KF_MENU_CONFIRM_USE,
                    KF_MENU_PREVIEW_MAGIC_ARTWORK, magic_ids[ctx.selected_index], KF_ITEM_STOCK_PLAYER, KF_TRADE_BUY))
                    == KF_MENU_RESULT_CANCELLED)
                selection = KF_MENU_RESULT_PENDING;
            else
                selection = magic_ids[ctx.selected_index];
        }
        if (selection != KF_MENU_RESULT_PENDING) {
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
                selection = KF_MENU_RESULT_CANCELLED;
            }
        } else if ((co_await menu_list_handle_navigation(ctx, input, prev))) {
            if (menu_load_texture(menu_texture_from_magic(magic_ids[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED)
                co_return KF_MENU_RESULT_CANCELLED;
        } else if (kf::button_pressed(input, prev, kf::Button::Confirm)) {
            (co_await menu_play_input_sound(MENU_SOUND_CONFIRM));
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if (kf::button_pressed(input, prev, kf::Button::Back)) {
            (co_await menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR));
            selection = KF_MENU_RESULT_CANCELLED;
        }

        if (ctx.entry_count != 0)
            menu_add_magic_artwork_quad();
        menu_list_render(&ctx);
    }

    if (selection != KF_MENU_RESULT_CANCELLED) {
        if (player.actions)
            co_await player_request_action(player, kf::net::CommandKind::UseMagic,
                kf_enum_encode<u16>(selection));
        else player_use_support_magic(world, player, kf_enum_decode<KfEffectKind>(kf_enum_encode<u16>(selection)));
    }
    co_return selection;
}

kf::FrameTask<void> menu_equipment_root(WorldState &world, PlayerContext &player)
{
    s32 cursor = 0;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    KfMenuResult result = KF_MENU_RESULT_PENDING;
    KfEquipmentMenuCategory selection = KF_EQUIP_MENU_NONE;

    menu_frame_begin();
    menu_draw_equipment_names(player);
    menu_draw_window(KF_MENU_WINDOW_EQUIPMENT, KF_MENU_EQUIPMENT_ROW_COUNT, 0, KF_MENU_CONFIRM_IDLE);

    for (;;) {
        (co_await menu_present_frame());
        if (selection != KF_EQUIP_MENU_NONE || kf_enum_encode<s32>(result) == kf_enum_encode<s32>(selection)) {
            menu_frame_begin();
            menu_draw_equipment_names(player);
            menu_draw_window(KF_MENU_WINDOW_EQUIPMENT, KF_MENU_EQUIPMENT_ROW_COUNT, cursor, confirm);
            (co_await menu_present_frame());
            (co_await game_wait_buttons_released());
        }
        switch (selection) {
        case KF_EQUIP_MENU_NONE:
            break;
        case KF_EQUIP_MENU_ARM:
        case KF_EQUIP_MENU_LEG:
            if (player.state.equipped_body_armor_id == KF_ITEM_FULL_PLATE) {
                (co_await menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR));
                break;
            }

        case KF_EQUIP_MENU_WEAPON:
        case KF_EQUIP_MENU_SHIELD:
        case KF_EQUIP_MENU_HEAD:
        case KF_EQUIP_MENU_BODY:
        case KF_EQUIP_MENU_ACCESSORY:
            (co_await menu_equip_select(player, selection));
            break;
        case KF_EQUIP_MENU_MAGIC:
            (co_await menu_spell_select(world, player));
            break;
        }
        selection = KF_EQUIP_MENU_NONE;
        if (result != KF_MENU_RESULT_PENDING)
            co_return;
        menu_frame_begin();
        confirm = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = kf::host_read_buttons();
        if (kf::button_pressed(input, prev, kf::Button::Up)) {
            (co_await menu_play_input_sound(MENU_SOUND_CURSOR));
            if (cursor != 0)
                cursor--;
            else
                cursor = KF_MENU_EQUIPMENT_RETURN_ROW;
        } else if (kf::button_pressed(input, prev, kf::Button::Down)) {
            (co_await menu_play_input_sound(MENU_SOUND_CURSOR));
            if (cursor != KF_MENU_EQUIPMENT_RETURN_ROW)
                cursor++;
            else
                cursor = 0;
        } else if (kf::button_pressed(input, prev, kf::Button::Confirm)) {
            (co_await menu_play_input_sound(MENU_SOUND_CONFIRM));
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (cursor < KF_MENU_EQUIPMENT_RETURN_ROW)
                selection = kf_enum_decode<KfEquipmentMenuCategory>(cursor);
            else
                result = KF_MENU_RESULT_CANCELLED;
        } else if (kf::button_pressed(input, prev, kf::Button::Back)) {
            (co_await menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR));
            result = KF_MENU_RESULT_CANCELLED;
        }
        menu_draw_equipment_names(player);
        menu_draw_window(KF_MENU_WINDOW_EQUIPMENT, KF_MENU_EQUIPMENT_ROW_COUNT, cursor, confirm);
    }
}
