#include <kf/lib/null.h>

#include <kf/game/input.h>
#include <kf/game/menu.h>
#include <kf/game/player.h>
#include <psyq/pad.h>

s32 menu_use_item_panel(void);

void menu_save_confirm(void)
{
    KfSaveHeader header;
    KfSavePayload payload;
    s32 i;

    save_payload_buffer = &payload;
    save_header_buffer = &header;

    i = 0;
    do {
        i++;
        menu_frame_begin();
        menu_draw_save_slots(NULL, KF_SAVE_OVERLAY_ALL);
        menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, 0, KF_MENU_CONFIRM_IDLE);
        menu_present_frame();
    } while (i < 3);
    menu_play_input_sound(MENU_SOUND_CURSOR);
    while (PadRead(1) != 0)
        ;
    menu_save_panel();
}

s32 menu_root(void)
{
    s32 cursor = 0;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 result = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);
    KfMenuRootChoice selection = KF_ROOT_CHOICE_NONE;
    s32 i;

    i = 0;
    do {
        i++;
        menu_frame_begin();
        menu_draw_status_summary();
        menu_draw_window(KF_MENU_WINDOW_ROOT, KF_MENU_ROOT_ROW_COUNT, cursor, confirm);
        menu_present_frame();
    } while (i < 3);
    menu_play_input_sound(MENU_SOUND_CURSOR);
    while (PadRead(1) != 0)
        ;

    for (;;) {
        if (selection != KF_ROOT_CHOICE_NONE || result == kf_enum_encode<s32>(selection)) {
            menu_frame_begin();
            menu_draw_status_summary();
            menu_draw_window(KF_MENU_WINDOW_ROOT, KF_MENU_ROOT_ROW_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }
        switch (selection) {
        case KF_ROOT_CHOICE_USE_ITEM:
            result = menu_use_item_panel();
            if (result == kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED))
                result = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);
            break;
        case KF_ROOT_CHOICE_USE_MAGIC:
            result = kf_enum_encode<s32>(menu_magic_panel());
            if (result == kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED))
                result = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);
            else
                result = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
            break;
        case KF_ROOT_CHOICE_EQUIPMENT:
            menu_equipment_root();
            break;
        case KF_ROOT_CHOICE_STATUS:
            menu_status_panel();
            break;
        case KF_ROOT_CHOICE_DROP_ITEM:
            menu_drop_item_panel();
            break;
        case KF_ROOT_CHOICE_SYSTEM:
            result = kf_enum_encode<s32>(menu_system_panel());
            if (result == kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED))
                result = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);
            break;
        case KF_ROOT_CHOICE_CONFIG:
            menu_config_panel();
            break;
        }
        if (result != kf_enum_encode<s32>(KF_MENU_RESULT_PENDING)) {
            selection = KF_ROOT_CHOICE_NONE;
            while (PadRead(1) != 0)
                ;
            return result;
        }
        selection = KF_ROOT_CHOICE_NONE;
        confirm = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = PadRead(1);
        if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 0)
                cursor--;
            else
                cursor = KF_MENU_ROOT_RETURN_ROW;
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_MENU_ROOT_RETURN_ROW)
                cursor++;
            else
                cursor = 0;
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (cursor < KF_MENU_ROOT_RETURN_ROW)
                selection = kf_enum_decode<KfMenuRootChoice>(cursor);
            else
                result = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
        }
        menu_frame_begin();
        menu_draw_status_summary();
        menu_draw_window(KF_MENU_WINDOW_ROOT, KF_MENU_ROOT_ROW_COUNT, cursor, confirm);
        menu_present_frame();
    }
}

enum {
    MEDICINAL_HERB_HP_RECOVERY = 25,
    ANTIDOTE_HERB_HP_RECOVERY = 10,
    RECOVERY_MEDICINE_HP_RECOVERY = 80,
    DRAGON_KING_GRASS_LEAF_HP_RECOVERY = 150,
    DRAGON_KING_GRASS_FRUIT_HP_RECOVERY = 300
};

s32 menu_use_item_panel(void)
{
    KfMenuList ctx;
    s16 labels[50][MENU_GLYPHS_PER_ROW];
    u8 quantities[56];
    KfObjectId item_ids[56];
    u8 *player_stock;
    s32 found;
    s32 item_id;
    s32 j;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);

    while (PadRead(1) != 0)
        ;
    menu_list_init(&ctx, KF_MENU_WINDOW_ROOT, kf_enum_encode<s32>(KF_ROOT_CHOICE_USE_ITEM));

    player_stock = item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)];
    found = 0;
    if (player_stock[kf_enum_encode<u8>(KF_ITEM_WATCHMAN_MAP)] != 0) {
        for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
            labels[found][j] = item_name_rows[kf_enum_encode<u8>(KF_ITEM_WATCHMAN_MAP)].codes[j];
        quantities[found] = player_stock[kf_enum_encode<u8>(KF_ITEM_WATCHMAN_MAP)];
        item_ids[found] = KF_ITEM_WATCHMAN_MAP;
        found++;
    }
    if (player_stock[kf_enum_encode<u8>(KF_ITEM_SORCERER_MAP)] != 0) {
        for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
            labels[found][j] = item_name_rows[kf_enum_encode<u8>(KF_ITEM_SORCERER_MAP)].codes[j];
        quantities[found] = player_stock[kf_enum_encode<u8>(KF_ITEM_SORCERER_MAP)];
        item_ids[found] = KF_ITEM_SORCERER_MAP;
        found++;
    }
    for (item_id = kf_enum_encode<s32>(KF_ITEM_VERDITE); item_id < kf_enum_encode<s32>(KF_ITEM_LIGHT_RING); item_id++) {
        if (item_id != kf_enum_encode<s32>(KF_ITEM_WATCHMAN_MAP) && item_id != kf_enum_encode<s32>(KF_ITEM_SORCERER_MAP) && player_stock[item_id] != 0) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                labels[found][j] = item_name_rows[item_id].codes[j];
            quantities[found] = player_stock[item_id];
            item_ids[found] = kf_enum_decode<KfObjectId>(item_id);
            found++;
        }
    }
    for (item_id = kf_enum_encode<s32>(KF_ITEM_GOLD_CROSS); item_id < KF_ITEM_COUNT; item_id++) {
        if (item_id != kf_enum_encode<s32>(KF_ITEM_WATCHMAN_MAP) && item_id != kf_enum_encode<s32>(KF_ITEM_SORCERER_MAP) && player_stock[item_id] != 0) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                labels[found][j] = item_name_rows[item_id].codes[j];
            quantities[found] = player_stock[item_id];
            item_ids[found] = kf_enum_decode<KfObjectId>(item_id);
            found++;
        }
    }
    ctx.entry_count = found;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = quantities;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
            return kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
        menu_item_model_preview(item_ids[ctx.selected_index]);
    }
    menu_list_render(&ctx);
    menu_present_frame();

    for (;;) {
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            if (menu_list_confirm(&ctx, KF_MENU_CONFIRM_USE,
                    KF_MENU_PREVIEW_ITEM_MODEL, item_ids[ctx.selected_index], KF_ITEM_STOCK_PLAYER, KF_TRADE_BUY)
                    == KF_MENU_RESULT_CANCELLED)
                selection = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);
            else
                selection = kf_enum_encode<u8>(item_ids[ctx.selected_index]);
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (selection != kf_enum_encode<s32>(KF_MENU_RESULT_PENDING)) {
            while (PadRead(1) != 0)
                ;
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
            }
        } else if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_previous(&ctx);
            if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_next(&ctx);
            if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            if (item_ids[ctx.selected_index] == KF_ITEM_WATCHMAN_MAP || item_ids[ctx.selected_index] == KF_ITEM_SORCERER_MAP) {
                menu_release_item_model();
                menu_map_viewer(item_ids[ctx.selected_index]);
                menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
                if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
                    return kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
            } else {
                confirm = KF_MENU_CONFIRM_REQUESTED;
            }
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_item_model_preview(item_ids[ctx.selected_index]);
        menu_list_render(&ctx);
        menu_present_frame();
    }

    menu_release_item_model();
    if ((u32)(selection - kf_enum_encode<s32>(KF_ITEM_VERDITE)) < kf_enum_encode<s32>(KF_ITEM_LIGHT_RING) - kf_enum_encode<s32>(KF_ITEM_VERDITE)) {
        player_stock[selection]--;
        if (selection == kf_enum_encode<s32>(KF_ITEM_MEDICINAL_HERB)) {
            player_state.vitals.current_hp += MEDICINAL_HERB_HP_RECOVERY;
        } else if (selection == kf_enum_encode<s32>(KF_ITEM_ANTIDOTE_HERB)) {
            player_state.vitals.current_hp += ANTIDOTE_HERB_HP_RECOVERY;
            player_state.status_effect_flags &= KF_PLAYER_STATUS_CURSE
                | KF_PLAYER_STATUS_DARKNESS | KF_PLAYER_STATUS_SLOWED;
        } else if (selection == kf_enum_encode<s32>(KF_ITEM_RECOVERY_MEDICINE)) {
            player_state.vitals.current_hp += RECOVERY_MEDICINE_HP_RECOVERY;
            player_state.status_effect_flags &= KF_PLAYER_STATUS_CURSE
                | KF_PLAYER_STATUS_DARKNESS;
        } else if (selection == kf_enum_encode<s32>(KF_ITEM_DRAGON_KING_GRASS_LEAF)) {
            player_state.vitals.current_hp += DRAGON_KING_GRASS_LEAF_HP_RECOVERY;
            player_state.status_effect_flags = KF_PLAYER_STATUS_NONE;
        } else if (selection == kf_enum_encode<s32>(KF_ITEM_DRAGON_KING_GRASS_FRUIT)) {
            player_state.vitals.current_hp += DRAGON_KING_GRASS_FRUIT_HP_RECOVERY;
            player_state.status_effect_flags = KF_PLAYER_STATUS_NONE;
            player_state.vitals.current_mp = player_state.vitals.maximum_mp;
        }
        if (player_state.vitals.current_hp > player_state.vitals.maximum_hp)
            player_state.vitals.current_hp = player_state.vitals.maximum_hp;
        if (player_state.vitals.current_mp > player_state.vitals.maximum_mp)
            player_state.vitals.current_mp = player_state.vitals.maximum_mp;
    }
    return selection;
}
