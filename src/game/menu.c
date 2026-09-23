#include <kf/lib/null.h>

#include <kf/game/input.h>
#include <kf/game/menu.h>
#include <kf/game/game.h>

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
        menu_draw_dialog_frame(NULL, KF_SAVE_OVERLAY_ALL);
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
    s32 result = ((s32)(KF_MENU_RESULT_PENDING));
    KfMenuRootChoice selection = KF_ROOT_CHOICE_NONE;
    s32 i;

    i = 0;
    do {
        i++;
        menu_frame_begin();
        menu_draw_stats_header();
        menu_draw_window(KF_MENU_WINDOW_ROOT, KF_MENU_ROOT_ROW_COUNT, cursor, confirm);
        menu_present_frame();
    } while (i < 3);
    menu_play_input_sound(MENU_SOUND_CURSOR);
    while (PadRead(1) != 0)
        ;

    for (;;) {
        if (selection != KF_ROOT_CHOICE_NONE || result == ((s32)(selection))) {
            menu_frame_begin();
            menu_draw_stats_header();
            menu_draw_window(KF_MENU_WINDOW_ROOT, KF_MENU_ROOT_ROW_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }
        switch (selection) {
        case KF_ROOT_CHOICE_USE_ITEM:
            result = menu_use_item_panel();
            if (result == ((s32)(KF_MENU_RESULT_CANCELLED)))
                result = ((s32)(KF_MENU_RESULT_PENDING));
            break;
        case KF_ROOT_CHOICE_USE_MAGIC:
            result = ((s32)(menu_magic_panel()));
            if (result == ((s32)(KF_MENU_RESULT_CANCELLED)))
                result = ((s32)(KF_MENU_RESULT_PENDING));
            else
                result = ((s32)(KF_MENU_RESULT_CANCELLED));
            break;
        case KF_ROOT_CHOICE_EQUIPMENT:
            menu_option_root();
            break;
        case KF_ROOT_CHOICE_STATUS:
            menu_status_panel();
            break;
        case KF_ROOT_CHOICE_DROP_ITEM:
            menu_drop_item();
            break;
        case KF_ROOT_CHOICE_SYSTEM:
            result = ((s32)(menu_save_load_hub()));
            if (result == ((s32)(KF_MENU_RESULT_CANCELLED)))
                result = ((s32)(KF_MENU_RESULT_PENDING));
            break;
        case KF_ROOT_CHOICE_CONFIG:
            menu_config_panel();
            break;
        }
        if (result != ((s32)(KF_MENU_RESULT_PENDING))) {
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
                selection = ((KfMenuRootChoice)(cursor));
            else
                result = ((s32)(KF_MENU_RESULT_CANCELLED));
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = ((s32)(KF_MENU_RESULT_CANCELLED));
        }
        menu_frame_begin();
        menu_draw_stats_header();
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
    u8 counts[56];
    KfObjectId codes[56];
    u8 *inv;
    s32 found;
    s32 code;
    s32 j;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = ((s32)(KF_MENU_RESULT_PENDING));

    while (PadRead(1) != 0)
        ;
    menu_list_init(&ctx, KF_MENU_WINDOW_ROOT, ((s32)(KF_ROOT_CHOICE_USE_ITEM)));

    inv = item_stock[((u8)(KF_ITEM_STOCK_PLAYER))];
    found = 0;
    if (inv[((u8)(KF_ITEM_WATCHMAN_MAP))] != 0) {
        for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
            labels[found][j] = item_name_rows[((u8)(KF_ITEM_WATCHMAN_MAP))].codes[j];
        counts[found] = inv[((u8)(KF_ITEM_WATCHMAN_MAP))];
        codes[found] = KF_ITEM_WATCHMAN_MAP;
        found++;
    }
    if (inv[((u8)(KF_ITEM_SORCERER_MAP))] != 0) {
        for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
            labels[found][j] = item_name_rows[((u8)(KF_ITEM_SORCERER_MAP))].codes[j];
        counts[found] = inv[((u8)(KF_ITEM_SORCERER_MAP))];
        codes[found] = KF_ITEM_SORCERER_MAP;
        found++;
    }
    for (code = ((s32)(KF_ITEM_VERDITE)); code < ((s32)(KF_ITEM_LIGHT_RING)); code++) {
        if (code != ((s32)(KF_ITEM_WATCHMAN_MAP)) && code != ((s32)(KF_ITEM_SORCERER_MAP)) && inv[code] != 0) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                labels[found][j] = item_name_rows[code].codes[j];
            counts[found] = inv[code];
            codes[found] = ((KfObjectId)(code));
            found++;
        }
    }
    for (code = ((s32)(KF_ITEM_GOLD_CROSS)); code < KF_ITEM_COUNT; code++) {
        if (code != ((s32)(KF_ITEM_WATCHMAN_MAP)) && code != ((s32)(KF_ITEM_SORCERER_MAP)) && inv[code] != 0) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                labels[found][j] = item_name_rows[code].codes[j];
            counts[found] = inv[code];
            codes[found] = ((KfObjectId)(code));
            found++;
        }
    }
    ctx.entry_count = found;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = counts;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_model(codes[ctx.selected_index]) != KF_RESOURCE_LOADED)
            return ((s32)(KF_MENU_RESULT_CANCELLED));
        menu_item_model_preview(codes[ctx.selected_index]);
    }
    menu_list_render(&ctx);
    menu_present_frame();

    for (;;) {
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            if (menu_list_interact(&ctx, KF_MENU_CONFIRM_USE,
                    KF_MENU_PREVIEW_ITEM_MODEL, codes[ctx.selected_index], KF_ITEM_STOCK_PLAYER, KF_TRADE_BUY)
                    == KF_MENU_RESULT_CANCELLED)
                selection = ((s32)(KF_MENU_RESULT_PENDING));
            else
                selection = ((u8)(codes[ctx.selected_index]));
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (selection != ((s32)(KF_MENU_RESULT_PENDING))) {
            while (PadRead(1) != 0)
                ;
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = ((s32)(KF_MENU_RESULT_CANCELLED));
            }
        } else if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_previous(&ctx);
            if (menu_load_item_model(codes[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return ((s32)(KF_MENU_RESULT_CANCELLED));
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_next(&ctx);
            if (menu_load_item_model(codes[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return ((s32)(KF_MENU_RESULT_CANCELLED));
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            if (codes[ctx.selected_index] == KF_ITEM_WATCHMAN_MAP || codes[ctx.selected_index] == KF_ITEM_SORCERER_MAP) {
                menu_release_item_model();
                menu_map_viewer(codes[ctx.selected_index]);
                menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
                if (menu_load_item_model(codes[ctx.selected_index]) != KF_RESOURCE_LOADED)
                    return ((s32)(KF_MENU_RESULT_CANCELLED));
            } else {
                confirm = KF_MENU_CONFIRM_REQUESTED;
            }
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = ((s32)(KF_MENU_RESULT_CANCELLED));
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_item_model_preview(codes[ctx.selected_index]);
        menu_list_render(&ctx);
        menu_present_frame();
    }

    menu_release_item_model();
    if ((u32)(selection - ((s32)(KF_ITEM_VERDITE))) < ((s32)(KF_ITEM_LIGHT_RING)) - ((s32)(KF_ITEM_VERDITE))) {
        inv[selection]--;
        if (selection == ((s32)(KF_ITEM_MEDICINAL_HERB))) {
            player_state.vitals.current_hp += MEDICINAL_HERB_HP_RECOVERY;
        } else if (selection == ((s32)(KF_ITEM_ANTIDOTE_HERB))) {
            player_state.vitals.current_hp += ANTIDOTE_HERB_HP_RECOVERY;
            player_state.status_effect_flags &= KF_PLAYER_STATUS_CURSE
                | KF_PLAYER_STATUS_DARKNESS | KF_PLAYER_STATUS_SLOWED;
        } else if (selection == ((s32)(KF_ITEM_RECOVERY_MEDICINE))) {
            player_state.vitals.current_hp += RECOVERY_MEDICINE_HP_RECOVERY;
            player_state.status_effect_flags &= KF_PLAYER_STATUS_CURSE
                | KF_PLAYER_STATUS_DARKNESS;
        } else if (selection == ((s32)(KF_ITEM_DRAGON_KING_GRASS_LEAF))) {
            player_state.vitals.current_hp += DRAGON_KING_GRASS_LEAF_HP_RECOVERY;
            player_state.status_effect_flags = KF_PLAYER_STATUS_NONE;
        } else if (selection == ((s32)(KF_ITEM_DRAGON_KING_GRASS_FRUIT))) {
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
