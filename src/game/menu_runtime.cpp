#include <kf/lib/null.h>
#include <kf/lib/bool.h>

#include <kf/game/input.h>
#include <kf/game/menu.h>
#include <kf/game/player.h>
#include <kf/game/cd.h>
#include <kf/lib/resources.h>
#include <psyq/pad.h>
#include <psyq/libc.h>
#include <kf/game/graphics.h>

KfMenuModelAllocation menu_item_model_allocation = KF_MENU_MODEL_RELEASED;

SVECTOR menu_item_preview_rotation = {0, 0, 0, 0};

static POLY_FT4 *current_poly_ft4;

enum {
    MENU_STATUS_BACKDROP_LEFT_X = 6,
    MENU_STATUS_BACKDROP_RIGHT_X = MENU_STATUS_BACKDROP_LEFT_X + MENU_BACKDROP_COLUMN_STEP
};

static inline void menu_preview_light_source(MATRIX *light_source)
{
    light_source->m[0][0] = -KF_FIXED12_ONE;
    light_source->m[0][1] = -KF_FIXED12_ONE;
    light_source->m[0][2] = -KF_FIXED12_ONE;
    light_source->m[1][0] = -KF_FIXED12_ONE;
    light_source->m[1][1] = -KF_FIXED12_ONE;
    light_source->m[1][2] = -KF_FIXED12_ONE;
    light_source->m[2][0] = 0;
    light_source->m[2][1] = 0;
    light_source->m[2][2] = 0;
}

static inline void menu_enable_window_blending(void)
{
    SetSemiTrans((void *)current_poly_ft4, 1);
}

static inline void menu_status_draw_backdrop_quad(
    s32 x, s32 y, KfBool32 flip_x, KfBool32 flip_y, const u16 *texture_page)
{
    menu_begin_poly_ft4();
    menu_enable_window_blending();
    current_poly_ft4->tpage = *texture_page;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        x,
        y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);

    setUVWH(current_poly_ft4,
        flip_x
            ? menu_assets.window_backdrop.u + (u8)menu_assets.window_backdrop.width
            : menu_assets.window_backdrop.u,
        flip_y
            ? menu_assets.window_backdrop.v + (u8)menu_assets.window_backdrop.height
            : menu_assets.window_backdrop.v,
        flip_x ? -(u8)menu_assets.window_backdrop.width : (u8)menu_assets.window_backdrop.width,
        flip_y ? -(u8)menu_assets.window_backdrop.height : (u8)menu_assets.window_backdrop.height);
    menu_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);
}

void menu_status_panel(void)
{
    s32 frame;
    s32 input;
    const u16 *texture_page;

    frame = 0;
    texture_page = &menu_assets.window_backdrop.tpage;
    while (1) {
        menu_frame_begin();
        menu_draw_status_details();

        menu_status_draw_backdrop_quad(MENU_STATUS_BACKDROP_LEFT_X, MENU_BACKDROP_TOP_Y, KF_FALSE, KF_FALSE, texture_page);

        menu_status_draw_backdrop_quad(MENU_STATUS_BACKDROP_RIGHT_X, MENU_BACKDROP_TOP_Y, KF_TRUE, KF_FALSE, texture_page);

        menu_status_draw_backdrop_quad(MENU_STATUS_BACKDROP_LEFT_X, MENU_BACKDROP_BOTTOM_Y, KF_FALSE, KF_TRUE, texture_page);

        menu_status_draw_backdrop_quad(MENU_STATUS_BACKDROP_RIGHT_X, MENU_BACKDROP_BOTTOM_Y, KF_TRUE, KF_TRUE, texture_page);

        menu_draw_window_backdrop();
        menu_present_frame();
        if (frame < MENU_PANEL_INPUT_RELEASE_FRAME) {
            frame++;
            continue;
        }
        if (frame == MENU_PANEL_INPUT_RELEASE_FRAME) {
            while (PadRead(1) != 0) {
            }
            frame++;
            continue;
        }
        input = PadRead(1);
        if (input != 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            while (PadRead(1) != 0) {
            }
            return;
        }
    }
}

void menu_drop_item_panel(void)
{
    KfMenuList ctx;
    s16 labels[KF_ITEM_COUNT][MENU_GLYPHS_PER_ROW];
    u8 quantities[KF_ITEM_COUNT];
    KfObjectId item_ids[KF_ITEM_COUNT];
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
    menu_list_init(&ctx, KF_MENU_WINDOW_ROOT, kf_enum_encode<s32>(KF_ROOT_CHOICE_DROP_ITEM));

    found = 0;
    item_id = 0;
    player_stock = item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)];
    for (; item_id < KF_ITEM_COUNT; item_id++) {
        if (player_stock[item_id] != 0) {
            quantities[found] = player_stock[item_id];
            if (PLAYER_ITEM_IS_EQUIPPED(item_id))
                quantities[found]--;
            if (quantities[found] != 0) {
                for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                    labels[found][j] = item_name_rows[item_id].codes[j];
                item_ids[found] = kf_enum_decode<KfObjectId>(item_id);
                found++;
            }
        }
    }

    ctx.entry_count = found;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = NULL;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
            return;
        menu_item_model_preview(item_ids[ctx.selected_index]);
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            selection = kf_enum_encode<s32>(menu_list_confirm(&ctx, KF_MENU_CONFIRM_DROP,
                    KF_MENU_PREVIEW_ITEM_MODEL, item_ids[ctx.selected_index], KF_ITEM_STOCK_PLAYER, KF_TRADE_BUY));
            if (selection == kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED))
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
                return;
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_next(&ctx);
            if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return;
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_item_model_preview(item_ids[ctx.selected_index]);
        menu_list_render(&ctx);
    }

    menu_release_item_model();
    if (selection != kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED))
        player_stock[selection]--;
}

KfMenuResult menu_system_panel(void)
{
    KfSaveHeader header;
    KfSavePayload payload;
    s32 cursor = kf_enum_encode<s32>(KF_MENU_SYSTEM_ACTION_LOAD);
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    KfMenuResult result = KF_MENU_RESULT_PENDING;
    KfMenuSystemAction action = KF_MENU_SYSTEM_ACTION_NONE;

    save_payload_buffer = &payload;
    save_header_buffer = &header;

    for (;;) {
        if (action != KF_MENU_SYSTEM_ACTION_NONE || kf_enum_encode<s32>(result) == kf_enum_encode<s32>(action)) {
            menu_frame_begin();
            menu_draw_window(KF_MENU_WINDOW_SYSTEM, KF_MENU_SYSTEM_ROW_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }

        switch (action) {
        case KF_MENU_SYSTEM_ACTION_LOAD:
            result = menu_load_panel();
            if (result == KF_MENU_RESULT_ACCEPTED)
                result = KF_MENU_RESULT_GAME_LOADED;
            break;
        case KF_MENU_SYSTEM_ACTION_QUIT:
            result = menu_two_option_prompt(
                KF_MENU_WINDOW_SYSTEM, KF_MENU_SYSTEM_ROW_COUNT, cursor, NULL);
            if (result == KF_MENU_RESULT_ACCEPTED) {
                menu_load_texture(MENU_TEXTURE_POWER_OFF);
                audio_stop_sequence_fade();
                for (;;) {
                    menu_frame_begin();
                    menu_add_message_image_quad();
                    menu_draw_window(KF_MENU_WINDOW_SYSTEM, KF_MENU_SYSTEM_ROW_COUNT, cursor, confirm);
                    menu_present_frame();
                }
            }
            break;
        default:
            break;
        }

        if (action != KF_MENU_SYSTEM_ACTION_NONE && result == KF_MENU_RESULT_CANCELLED)
            result = KF_MENU_RESULT_PENDING;
        action = KF_MENU_SYSTEM_ACTION_NONE;
        if (result != KF_MENU_RESULT_PENDING)
            break;

        confirm = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = PadRead(1);
        if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 0)
                cursor--;
            else
                cursor = KF_MENU_SYSTEM_RETURN_ROW;
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_MENU_SYSTEM_RETURN_ROW)
                cursor++;
            else
                cursor = kf_enum_encode<s32>(KF_MENU_SYSTEM_ACTION_LOAD);
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (cursor == KF_MENU_SYSTEM_RETURN_ROW) {
                result = KF_MENU_RESULT_CANCELLED;
            } else {
                action = kf_enum_decode<KfMenuSystemAction>(cursor);
            }
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_CANCELLED;
        }

        menu_frame_begin();
        menu_draw_window(KF_MENU_WINDOW_SYSTEM, KF_MENU_SYSTEM_ROW_COUNT, cursor, confirm);
        menu_present_frame();
    }
    return result;
}

KfMenuResult menu_save_panel(void)
{
    KfSaveSlotSummary summaries[KF_SAVE_SLOT_COUNT];
    s32 cursor = 0;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    KfMenuResult result = KF_MENU_RESULT_PENDING;
    s32 status;
    s32 i;

    status = kf_enum_encode<s32>(save_system_read_catalog(summaries));
    if (status != kf_enum_encode<s32>(KF_SAVE_RESULT_OK) && status != kf_enum_encode<s32>(KF_SAVE_RESULT_NO_SPACE)) {
        menu_play_input_sound(MENU_SOUND_CURSOR);
        while (PadRead(1) == 0) {
            menu_frame_begin();
            menu_add_message_image_quad();
            menu_draw_save_slots(summaries, kf_enum_decode<KfSaveSlotOverlay>(cursor));
            menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
            menu_present_frame();
        }
        menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
        while (PadRead(1) != 0)
            ;
        return KF_MENU_RESULT_CANCELLED;
    }

    for (;;) {
        if (confirm == KF_MENU_CONFIRM_REQUESTED || result == KF_MENU_RESULT_CANCELLED) {
            menu_frame_begin();
            menu_draw_save_slots(summaries, kf_enum_decode<KfSaveSlotOverlay>(cursor));
            menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }

        if (confirm == KF_MENU_CONFIRM_REQUESTED && cursor != KF_MENU_SAVE_RETURN_ROW) {
            if (cursor == KF_MENU_SAVE_FORMAT_ROW) {
                status = kf_enum_encode<s32>(save_file_cleanup_temporary());
                if (status == kf_enum_encode<s32>(KF_SAVE_CLEANUP_TEMP_OPENED)) {
                    menu_load_texture(MENU_TEXTURE_CONFIRM_CARD_FORMAT);
                    while (PadRead(1) == 0) {
                        menu_frame_begin();
                        menu_add_message_image_quad();
                        menu_draw_save_slots(summaries, kf_enum_decode<KfSaveSlotOverlay>(cursor));
                        menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
                        menu_present_frame();
                    }
                }
                menu_play_input_sound(MENU_SOUND_CURSOR);
                while (PadRead(1) != 0)
                    ;
            }

            result = menu_two_option_prompt(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, summaries);
            if (result == KF_MENU_RESULT_CANCELLED) {
                result = KF_MENU_RESULT_PENDING;
            } else {
                if (cursor < KF_SAVE_SLOT_COUNT) {
                    menu_load_texture(MENU_TEXTURE_SAVING_DATA);
                    for (i = 0; i < 3; i++) {
                        menu_frame_begin();
                        menu_add_message_image_quad();
                        menu_draw_save_slots(summaries, kf_enum_decode<KfSaveSlotOverlay>(cursor));
                        menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
                        menu_present_frame();
                    }
                    status = kf_enum_encode<s32>(save_system_write_slot(kf_enum_decode<KfSaveSlotArgument>(cursor + kf_enum_encode<s16>(KF_SAVE_SLOT_FIRST))));
                } else if (cursor == KF_MENU_SAVE_FORMAT_ROW) {
                    menu_load_texture(MENU_TEXTURE_FORMATTING_CARD);
                    for (i = 0; i < 3; i++) {
                        menu_frame_begin();
                        menu_add_message_image_quad();
                        menu_draw_save_slots(summaries, kf_enum_decode<KfSaveSlotOverlay>(cursor));
                        menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
                        menu_present_frame();
                    }
                    status = kf_enum_encode<s32>(memory_card_check_or_format(KF_CARD_FORMAT_CONFIRMED));
                    memset((void *)summaries, 0, sizeof(summaries));
                }

                if (status != kf_enum_encode<s32>(KF_SAVE_RESULT_OK)) {
                    while (PadRead(1) == 0) {
                        menu_frame_begin();
                        menu_add_message_image_quad();
                        menu_draw_save_slots(summaries, kf_enum_decode<KfSaveSlotOverlay>(cursor));
                        menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
                        menu_present_frame();
                    }
                    menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
                    while (PadRead(1) != 0)
                        ;
                    result = KF_MENU_RESULT_PENDING;
                } else if (cursor == KF_MENU_SAVE_FORMAT_ROW) {
                    result = KF_MENU_RESULT_PENDING;
                }
            }
        }

        confirm = KF_MENU_CONFIRM_IDLE;
        if (result != KF_MENU_RESULT_PENDING)
            break;

        prev = input;
        input = PadRead(1);
        if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 0)
                cursor--;
            else
                cursor = KF_MENU_SAVE_RETURN_ROW;
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_MENU_SAVE_RETURN_ROW)
                cursor++;
            else
                cursor = 0;
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (cursor == KF_MENU_SAVE_RETURN_ROW)
                result = KF_MENU_RESULT_CANCELLED;
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_CANCELLED;
        }

        menu_frame_begin();
        menu_draw_save_slots(summaries, kf_enum_decode<KfSaveSlotOverlay>(cursor));
        menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
        menu_present_frame();
    }
    return result;
}

KfMenuResult menu_load_panel(void)
{
    KfSaveSlotSummary summaries[KF_SAVE_SLOT_COUNT];
    s32 cursor = 0;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    KfMenuResult result = KF_MENU_RESULT_PENDING;
    s32 i;

    if (save_system_read_catalog(summaries) != KF_SAVE_RESULT_OK) {
        menu_play_input_sound(MENU_SOUND_CURSOR);
        while (PadRead(1) == 0) {
            menu_frame_begin();
            menu_add_message_image_quad();
            menu_draw_save_slots(summaries, kf_enum_decode<KfSaveSlotOverlay>(cursor));
            menu_draw_window(KF_MENU_WINDOW_LOAD, KF_MENU_LOAD_ROW_COUNT, cursor, confirm);
            menu_present_frame();
        }
        menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
        while (PadRead(1) != 0)
            ;
        return KF_MENU_RESULT_CANCELLED;
    }

    for (;;) {
        if (confirm == KF_MENU_CONFIRM_REQUESTED || result == KF_MENU_RESULT_CANCELLED) {
            menu_frame_begin();
            menu_draw_save_slots(summaries, kf_enum_decode<KfSaveSlotOverlay>(cursor));
            menu_draw_window(KF_MENU_WINDOW_LOAD, KF_MENU_LOAD_ROW_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }

        if (confirm == KF_MENU_CONFIRM_REQUESTED && cursor != KF_MENU_LOAD_RETURN_ROW) {
            result = menu_two_option_prompt(KF_MENU_WINDOW_LOAD, KF_MENU_LOAD_ROW_COUNT, cursor, summaries);
            if (result == KF_MENU_RESULT_CANCELLED) {
                result = KF_MENU_RESULT_PENDING;
            } else {
                menu_load_texture(MENU_TEXTURE_LOADING_DATA);
                for (i = 0; i < 3; i++) {
                    menu_frame_begin();
                    menu_add_message_image_quad();
                    menu_draw_save_slots(summaries, kf_enum_decode<KfSaveSlotOverlay>(cursor));
                    menu_draw_window(KF_MENU_WINDOW_LOAD, KF_MENU_LOAD_ROW_COUNT, cursor, confirm);
                    menu_present_frame();
                }
                if (save_system_read_slot(kf_enum_decode<KfSaveSlotArgument>(cursor + kf_enum_encode<s16>(KF_SAVE_SLOT_FIRST))) != KF_SAVE_RESULT_OK) {
                    while (PadRead(1) == 0) {
                        menu_frame_begin();
                        menu_add_message_image_quad();
                        menu_draw_save_slots(summaries, kf_enum_decode<KfSaveSlotOverlay>(cursor));
                        menu_draw_window(KF_MENU_WINDOW_LOAD, KF_MENU_LOAD_ROW_COUNT, cursor, confirm);
                        menu_present_frame();
                    }
                    menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
                    while (PadRead(1) != 0)
                        ;
                    result = KF_MENU_RESULT_PENDING;
                }
            }
        }

        confirm = KF_MENU_CONFIRM_IDLE;
        if (result != KF_MENU_RESULT_PENDING)
            break;

        prev = input;
        input = PadRead(1);
        if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 0)
                cursor--;
            else
                cursor = KF_MENU_LOAD_RETURN_ROW;
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_MENU_LOAD_RETURN_ROW)
                cursor++;
            else
                cursor = 0;
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            if (cursor == KF_MENU_LOAD_RETURN_ROW) {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                confirm = KF_MENU_CONFIRM_REQUESTED;
                result = KF_MENU_RESULT_CANCELLED;
            } else if (summaries[cursor].current_hp == 0) {
                menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            } else {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                confirm = KF_MENU_CONFIRM_REQUESTED;
            }
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_CANCELLED;
        }

        menu_frame_begin();
        menu_draw_save_slots(summaries, kf_enum_decode<KfSaveSlotOverlay>(cursor));
        menu_draw_window(KF_MENU_WINDOW_LOAD, KF_MENU_LOAD_ROW_COUNT, cursor, confirm);
        menu_present_frame();
    }
    return result;
}

enum {
    CONFIG_OPTION_ON_X = 180,
    CONFIG_OPTION_OFF_X = 240,
    CONFIG_OPTION_FIRST_Y = 41,
    CONFIG_OPTION_ROW_STEP = 22
};

void menu_config_panel_draw(
    MenuGlyphString on_label, MenuGlyphString off_label, KfPlayerOption *option_states);

void menu_config_panel(void)
{
    KfPlayerOption option_states[KF_MENU_CONFIG_SETTING_COUNT];
    MenuGlyphString on_label;
    MenuGlyphString off_label;
    s32 row = 0;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    u32 input = 0;
    u32 prev;
    KfMenuResult result = KF_MENU_RESULT_PENDING;
    KfPlayerOption original_music;

    while (PadRead(1) != 0) {
    }

    on_label.position.x = CONFIG_OPTION_ON_X;
    on_label.position.y = CONFIG_OPTION_FIRST_Y;
    on_label.glyphs.codes[0] = 0xf9;
    on_label.glyphs.codes[1] = 0xfa;
    on_label.glyphs.codes[2] = MENU_TEXT_END;
    off_label.position.x = CONFIG_OPTION_OFF_X;
    off_label.position.y = CONFIG_OPTION_FIRST_Y;
    off_label.glyphs.codes[0] = 0xf9;
    off_label.glyphs.codes[1] = 0xfb;
    off_label.glyphs.codes[2] = 0xfb;
    off_label.glyphs.codes[3] = MENU_TEXT_END;
    option_states[KF_MENU_CONFIG_EFFECTS_ROW] = player_state.audio_effects_enabled;
    option_states[KF_MENU_CONFIG_MUSIC_ROW] = player_state.audio_music_enabled;
    option_states[KF_MENU_CONFIG_GAUGES_ROW] = player_state.hud_gauges_enabled;
    option_states[KF_MENU_CONFIG_COMPASS_ROW] = player_state.compass_enabled;
    original_music = option_states[KF_MENU_CONFIG_MUSIC_ROW];

    menu_frame_begin();
    menu_config_panel_draw(on_label, off_label, option_states);
    menu_draw_window(KF_MENU_WINDOW_CONFIG, KF_MENU_CONFIG_ROW_COUNT, row, confirm);
    menu_present_frame();
    do {
        if (confirm == KF_MENU_CONFIRM_REQUESTED || result == KF_MENU_RESULT_CANCELLED) {
            menu_frame_begin();
            menu_config_panel_draw(on_label, off_label, option_states);
            menu_draw_window(KF_MENU_WINDOW_CONFIG, KF_MENU_CONFIG_ROW_COUNT, row, confirm);
            menu_present_frame();
            while (PadRead(1) != 0) {
            }
        }
        if (result != KF_MENU_RESULT_PENDING) {
            break;
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        menu_frame_begin();
        prev = input;
        input = PadRead(1);
        if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (row != 0) {
                row--;
            } else {
                row = KF_MENU_CONFIG_RETURN_ROW;
            }
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (row != KF_MENU_CONFIG_RETURN_ROW) {
                row++;
            } else {
                row = 0;
            }
        } else if ((PAD_PRESSED(input, prev, PADLright)) ||
                   (PAD_PRESSED(input, prev, PADLleft))) {
            if (row != KF_MENU_CONFIG_RETURN_ROW) {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                option_states[row] = kf_enum_decode<KfPlayerOption>(option_states[row] == KF_PLAYER_OPTION_OFF);
            }
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            if (row == KF_MENU_CONFIG_RETURN_ROW) {
                confirm = KF_MENU_CONFIRM_REQUESTED;
                result = KF_MENU_RESULT_CANCELLED;
            } else {
                option_states[row] = kf_enum_decode<KfPlayerOption>(option_states[row] == KF_PLAYER_OPTION_OFF);
            }
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_CANCELLED;
        }
        menu_config_panel_draw(on_label, off_label, option_states);
        menu_draw_window(KF_MENU_WINDOW_CONFIG, KF_MENU_CONFIG_ROW_COUNT, row, confirm);
        menu_present_frame();
    } while (1);

    player_state.audio_effects_enabled = option_states[KF_MENU_CONFIG_EFFECTS_ROW];
    player_state.audio_music_enabled = option_states[KF_MENU_CONFIG_MUSIC_ROW];
    player_state.hud_gauges_enabled = option_states[KF_MENU_CONFIG_GAUGES_ROW];
    player_state.compass_enabled = option_states[KF_MENU_CONFIG_COMPASS_ROW];
    if (player_state.audio_music_enabled != original_music) {
        if (player_state.audio_music_enabled == KF_PLAYER_OPTION_OFF) {
            audio_stop_sequence_fade();
        } else {
            audio_play_current_map_sequence();
        }
    }
}

void menu_config_panel_draw(
    MenuGlyphString on_label, MenuGlyphString off_label, KfPlayerOption *option_states)
{
    s32 i;
    KfPlayerOption *state;
    const MenuSpriteDef *off_box;

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    state = option_states;
    for (i = 0; i < KF_MENU_CONFIG_SETTING_COUNT; i++) {
        if (*state == KF_PLAYER_OPTION_ON) {
            menu_blit_sprite_translucent(&menu_assets.option_highlight, &on_label.position);
            off_box = &menu_assets.option_background;
        } else {
            menu_blit_sprite_translucent(&menu_assets.option_background, &on_label.position);
            off_box = &menu_assets.option_highlight;
        }
        menu_blit_sprite_translucent(off_box, &off_label.position);
        menu_draw_string(
            &menu_assets.glyph_atlas,
            &on_label);
        menu_draw_string(
            &menu_assets.glyph_atlas,
            &off_label);
        state++;
        on_label.position.y += CONFIG_OPTION_ROW_STEP;
        off_label.position.y += CONFIG_OPTION_ROW_STEP;
    }
    MENU_ENQUEUE_BACKGROUND();
}

enum {
    ROOT_STATUS_SUMMARY_ROW_STEP = 23
};

void menu_draw_status_summary(void)
{
    MenuGlyphString text;
    s32 glyph_index;
    s32 row_step = ROOT_STATUS_SUMMARY_ROW_STEP;

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    text.position.x = 0xb5;
    text.position.y = 0x24;
    text.glyphs.codes[0] = 0x82;
    text.glyphs.codes[1] = 0x83;
    text.glyphs.codes[2] = 0x84;
    text.glyphs.codes[3] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0x2b;
    text.glyphs.codes[1] = MENU_TEXT_DAKUTEN | 0x1c;
    text.glyphs.codes[2] = 0x2a;
    text.glyphs.codes[3] = MENU_TEXT_END;
    text.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 7;
    text.glyphs.codes[1] = 0x28;
    text.glyphs.codes[2] = 0xc;
    text.glyphs.codes[3] = MENU_TEXT_END;
    text.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0xcc;
    text.glyphs.codes[1] = 0xcd;
    text.glyphs.codes[2] = MENU_TEXT_END;
    text.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0xf0;
    text.glyphs.codes[1] = 0xf2;
    text.glyphs.codes[2] = MENU_TEXT_END;
    text.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0xf1;
    text.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0x85;
    text.glyphs.codes[1] = 0x86;
    text.glyphs.codes[2] = MENU_TEXT_END;
    text.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = MENU_TEXT_DAKUTEN | 0x9;
    text.glyphs.codes[1] = 0x2d;
    text.glyphs.codes[2] = 0x2a;
    text.glyphs.codes[3] = MENU_TEXT_DAKUTEN | 0x13;
    text.glyphs.codes[4] = MENU_TEXT_END;
    text.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.position.x = 0xfb;
    text.position.y = 0x24;
    menu_format_number(player_state.experience, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);

    text.position.y += row_step;
    menu_format_number(player_state.progress_state.level, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);

    text.position.x = 0xed;
    text.position.y += row_step;
    if (player_state.base_magic < MENU_CLASS_MIDDLE_STAT_MIN) {
        glyph_index = 0;
    } else {
        glyph_index = 2;
        if (player_state.base_magic < MENU_CLASS_HIGH_STAT_MIN) {
            glyph_index = 1;
        }
    }
    if (player_state.base_physical_power > (MENU_CLASS_MIDDLE_STAT_MIN - 1)) {
        if (player_state.base_physical_power < MENU_CLASS_HIGH_STAT_MIN) {
            glyph_index += MENU_CLASS_MAGIC_TIER_COUNT;
        } else {
            glyph_index += 2 * MENU_CLASS_MAGIC_TIER_COUNT;
        }
    }
    glyph_index *= MENU_CLASS_LABEL_GLYPHS;
    text.glyphs.codes[0] = glyph_index + MENU_CLASS_FIRST_GLYPH;
    text.glyphs.codes[1] = glyph_index + MENU_CLASS_FIRST_GLYPH + 1;
    text.glyphs.codes[2] = glyph_index + MENU_CLASS_FIRST_GLYPH + 2;
    text.glyphs.codes[3] = glyph_index + MENU_CLASS_FIRST_GLYPH + 3;
    text.glyphs.codes[4] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.position.x = 0xfb;
    text.position.y += row_step;
    menu_format_number(kf_enum_encode<u8>(player_state.progress_state.current_floor), MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);

    text.position.x = 0xe6;
    text.position.y += row_step;
    menu_format_number(player_state.vitals.current_hp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.glyphs.codes[0] = MENU_NUMBER_SLASH;
    text.glyphs.codes[1] = MENU_TEXT_END;
    text.position.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.position.x += MENU_NUMBER_ADVANCE;
    menu_format_number(player_state.vitals.maximum_hp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);

    text.position.x = 0xe6;
    text.position.y += row_step;
    menu_format_number(player_state.vitals.current_mp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.glyphs.codes[0] = MENU_NUMBER_SLASH;
    text.glyphs.codes[1] = MENU_TEXT_END;
    text.position.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.position.x += MENU_NUMBER_ADVANCE;
    menu_format_number(player_state.vitals.maximum_mp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);

    text.position.x = 0xdf;
    text.glyphs.codes[0] = MENU_TEXT_BLANK;
    text.glyphs.codes[1] = MENU_TEXT_BLANK;
    text.glyphs.codes[2] = MENU_TEXT_BLANK;
    text.glyphs.codes[3] = MENU_TEXT_BLANK;
    text.glyphs.codes[4] = MENU_TEXT_BLANK;
    text.glyphs.codes[5] = MENU_TEXT_END;
    text.position.y += row_step;
    if (player_state.status_effect_flags == KF_PLAYER_STATUS_NONE) {
        text.glyphs.codes[3] = 0xc5;
        text.glyphs.codes[4] = 0xc6;
    }
    glyph_index = 4;
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_SLOWED) != KF_PLAYER_STATUS_NONE) {
        text.glyphs.codes[4] = 0xc9;
        glyph_index = 3;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_POISON) != KF_PLAYER_STATUS_NONE) {
        text.glyphs.codes[glyph_index] = 0x88;
        glyph_index--;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_DARKNESS) != KF_PLAYER_STATUS_NONE) {
        text.glyphs.codes[glyph_index] = 199;
        glyph_index--;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_CURSE) != KF_PLAYER_STATUS_NONE) {
        text.glyphs.codes[glyph_index] = 200;
    }
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.position.x = 0xfb;
    text.position.y += row_step;
    menu_format_number(player_state.gold, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
}

enum {
    STATUS_SUMMARY_ROW_STEP = 16,
    STATUS_COMPONENT_ROW_STEP = 14
};

enum {
    STATUS_PHYSICAL_ATTACK_MULTIPLIER = 3,
    STATUS_PHYSICAL_ATTACK_DOWNSHIFT = 1,
    STATUS_ELEMENTAL_ATTACK_MULTIPLIER = 2,
    STATUS_ATTACK_SCALE_NUMERATOR = 10,
    STATUS_ATTACK_SCALE_DENOMINATOR = 8,
    STATUS_POISON_RESISTANCE_DIVISOR = 5,
    STATUS_DEFENSE_SCALE_NUMERATOR = 10,
    STATUS_DEFENSE_SCALE_DENOMINATOR = 7
};

void menu_draw_status_details(void)
{
    MenuGlyphString text;
    s32 glyph_index;
    s32 rating;
    s32 summary_y_origin;

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    summary_y_origin = 0x23;
    text.position.x = 0x15;
    text.position.y = summary_y_origin;
    text.glyphs.codes[0] = 0x82;
    text.glyphs.codes[1] = 0x83;
    text.glyphs.codes[2] = 0x84;
    text.glyphs.codes[3] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0x2b;
    text.glyphs.codes[1] = MENU_TEXT_DAKUTEN | 0x1c;
    text.glyphs.codes[2] = 0x2a;
    text.glyphs.codes[3] = MENU_TEXT_END;
    text.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 7;
    text.glyphs.codes[1] = 0x28;
    text.glyphs.codes[2] = 0xc;
    text.glyphs.codes[3] = MENU_TEXT_END;
    text.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0xf0;
    text.glyphs.codes[1] = 0xf2;
    text.glyphs.codes[2] = MENU_TEXT_END;
    text.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0xf1;
    text.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0x85;
    text.glyphs.codes[1] = 0x86;
    text.glyphs.codes[2] = MENU_TEXT_END;
    text.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = MENU_TEXT_DAKUTEN | 0x9;
    text.glyphs.codes[1] = 0x2d;
    text.glyphs.codes[2] = 0x2a;
    text.glyphs.codes[3] = MENU_TEXT_DAKUTEN | 0x13;
    text.glyphs.codes[4] = MENU_TEXT_END;
    text.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0x8c;
    text.glyphs.codes[1] = 0x8b;
    text.glyphs.codes[2] = MENU_TEXT_END;
    text.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0x78;
    text.glyphs.codes[2] = MENU_TEXT_END;
    text.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0xce;
    text.glyphs.codes[1] = 0xcf;
    text.glyphs.codes[2] = 0x89;
    text.glyphs.codes[3] = 0x8a;
    text.glyphs.codes[4] = 0x8b;
    text.glyphs.codes[5] = MENU_TEXT_END;
    text.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[2] = 0x7a;
    text.glyphs.codes[3] = 0xd0;
    text.glyphs.codes[5] = MENU_TEXT_END;
    text.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.position.x = 0x5b;
    text.position.y = summary_y_origin;
    menu_format_number(player_state.experience, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);

    text.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.progress_state.level, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);

    text.position.x = 0x4d;
    text.position.y += STATUS_SUMMARY_ROW_STEP;
    {
        s32 summary_row_step = STATUS_SUMMARY_ROW_STEP;

        if (player_state.base_magic < MENU_CLASS_MIDDLE_STAT_MIN) {
            glyph_index = 0;
        } else {
            glyph_index = 2;
            if (player_state.base_magic < MENU_CLASS_HIGH_STAT_MIN) {
                glyph_index = 1;
            }
        }
        if (player_state.base_physical_power > (MENU_CLASS_MIDDLE_STAT_MIN - 1)) {
            if (player_state.base_physical_power < MENU_CLASS_HIGH_STAT_MIN) {
                glyph_index += MENU_CLASS_MAGIC_TIER_COUNT;
            } else {
                glyph_index += 2 * MENU_CLASS_MAGIC_TIER_COUNT;
            }
        }
        glyph_index *= MENU_CLASS_LABEL_GLYPHS;
        text.glyphs.codes[0] = glyph_index + MENU_CLASS_FIRST_GLYPH;
        text.glyphs.codes[1] = glyph_index + MENU_CLASS_FIRST_GLYPH + 1;
        text.glyphs.codes[2] = glyph_index + MENU_CLASS_FIRST_GLYPH + 2;
        text.glyphs.codes[3] = glyph_index + MENU_CLASS_FIRST_GLYPH + 3;
        text.glyphs.codes[4] = MENU_TEXT_END;
        menu_draw_string(&menu_assets.glyph_atlas, &text);

        text.position.x = 0x46;
        text.position.y += summary_row_step;
        menu_format_number(player_state.vitals.current_hp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &text);
        text.glyphs.codes[0] = MENU_NUMBER_SLASH;
        text.glyphs.codes[1] = MENU_TEXT_END;
        text.position.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
        menu_draw_number(&menu_assets.number_atlas, &text);
        text.position.x += MENU_NUMBER_ADVANCE;
        menu_format_number(player_state.vitals.maximum_hp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &text);

        text.position.x = 0x46;
        text.position.y += summary_row_step;
        menu_format_number(player_state.vitals.current_mp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &text);
        text.glyphs.codes[0] = MENU_NUMBER_SLASH;
        text.glyphs.codes[1] = MENU_TEXT_END;
        text.position.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
        menu_draw_number(&menu_assets.number_atlas, &text);
        text.position.x += MENU_NUMBER_ADVANCE;
        menu_format_number(player_state.vitals.maximum_mp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &text);

        text.position.x = 0x3f;
        text.glyphs.codes[0] = MENU_TEXT_BLANK;
        text.glyphs.codes[1] = MENU_TEXT_BLANK;
        text.glyphs.codes[2] = MENU_TEXT_BLANK;
        text.glyphs.codes[3] = MENU_TEXT_BLANK;
        text.glyphs.codes[4] = MENU_TEXT_BLANK;
        text.glyphs.codes[5] = MENU_TEXT_END;
        text.position.y += summary_row_step;
        if (player_state.status_effect_flags == KF_PLAYER_STATUS_NONE) {
            text.glyphs.codes[3] = 0xc5;
            text.glyphs.codes[4] = 0xc6;
        }
        glyph_index = 4;
        if ((player_state.status_effect_flags & KF_PLAYER_STATUS_SLOWED) != KF_PLAYER_STATUS_NONE) {
            text.glyphs.codes[4] = 0xc9;
            glyph_index = 3;
        }
        if ((player_state.status_effect_flags & KF_PLAYER_STATUS_POISON) != KF_PLAYER_STATUS_NONE) {
            text.glyphs.codes[glyph_index] = 0x88;
            glyph_index--;
        }
        if ((player_state.status_effect_flags & KF_PLAYER_STATUS_DARKNESS) != KF_PLAYER_STATUS_NONE) {
            text.glyphs.codes[glyph_index] = 199;
            glyph_index--;
        }
        if ((player_state.status_effect_flags & KF_PLAYER_STATUS_CURSE) != KF_PLAYER_STATUS_NONE) {
            text.glyphs.codes[glyph_index] = 200;
        }
        menu_draw_string(&menu_assets.glyph_atlas, &text);

        text.position.x = 0x5b;
        text.position.y += summary_row_step;
        menu_format_number(player_state.gold, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &text);
        text.position.y += summary_row_step;
        menu_format_number(player_state.physical_power, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &text);
        text.position.y += summary_row_step;
        menu_format_number(player_state.magic, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &text);
        rating = (((u32)player_state.cutting_attack + player_state.striking_attack +
                   player_state.piercing_attack) * STATUS_PHYSICAL_ATTACK_MULTIPLIER
                   >> STATUS_PHYSICAL_ATTACK_DOWNSHIFT) +
                 (player_state.holy_attack + player_state.fire_attack)
                 * STATUS_ELEMENTAL_ATTACK_MULTIPLIER;
        rating = rating * STATUS_ATTACK_SCALE_NUMERATOR
            / STATUS_ATTACK_SCALE_DENOMINATOR;
        text.position.y += summary_row_step;
        menu_format_number(rating, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &text);
        rating = player_state.cutting_defense + player_state.striking_defense +
                 player_state.piercing_defense + player_state.poison_resistance / STATUS_POISON_RESISTANCE_DIVISOR +
                 player_state.magic_defense + player_state.fire_defense;
        rating = rating * STATUS_DEFENSE_SCALE_NUMERATOR / STATUS_DEFENSE_SCALE_DENOMINATOR;
        text.position.y += summary_row_step;
        menu_format_number(rating, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &text);
    }
    text.position.x = 0xb5;
    text.position.y = 0x1e;
    text.glyphs.codes[0] = 0x89;
    text.glyphs.codes[1] = 0x8a;
    text.glyphs.codes[2] = 0x8b;
    text.glyphs.codes[3] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = MENU_TEXT_BLANK;
    text.glyphs.codes[1] = 0xd1;
    text.glyphs.codes[2] = 0x6a;
    text.glyphs.codes[3] = MENU_TEXT_END;
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[1] = 0xd2;
    text.glyphs.codes[2] = 0x51;
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[1] = 0xd3;
    text.glyphs.codes[2] = 0x4c;
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[1] = 0xbf;
    text.glyphs.codes[2] = 0x58;
    text.glyphs.codes[3] = 0x78;
    text.glyphs.codes[4] = 0x79;
    text.glyphs.codes[5] = MENU_TEXT_END;
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[1] = 0xd4;
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = 0x7a;
    text.glyphs.codes[1] = 0xd0;
    text.glyphs.codes[2] = 0x8b;
    text.glyphs.codes[3] = MENU_TEXT_END;
    text.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[0] = MENU_TEXT_BLANK;
    text.glyphs.codes[1] = 0xd1;
    text.glyphs.codes[2] = 0x6a;
    text.glyphs.codes[3] = MENU_TEXT_END;
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[1] = 0xd2;
    text.glyphs.codes[2] = 0x51;
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[1] = 0xd3;
    text.glyphs.codes[2] = 0x4c;
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[1] = 0x88;
    text.glyphs.codes[2] = MENU_TEXT_END;
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[1] = 0x78;
    text.glyphs.codes[2] = 0x58;
    text.glyphs.codes[3] = 0x78;
    text.glyphs.codes[4] = 0x79;
    text.glyphs.codes[5] = MENU_TEXT_END;
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.glyphs.codes[1] = 0xd4;
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.position.x = 0xfb;
    text.position.y = 0x2c;
    menu_format_number(player_state.cutting_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.striking_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.piercing_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.holy_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.fire_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.position.y += 0x1e;
    menu_format_number(player_state.cutting_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.striking_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.piercing_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.poison_resistance, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.magic_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
    text.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.fire_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
}

enum {
    EQUIPMENT_NAME_FIRST_Y = 40,
    EQUIPMENT_NAME_ROW_STEP = 20
};

void menu_draw_equipment_names(void)
{
    MenuGlyphString text;

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    text.position.x = MENU_ITEM_NAME_X;
    text.position.y = EQUIPMENT_NAME_FIRST_Y;
    if (player_state.equipped_weapon_id != KF_OBJECT_NONE) {
        text.glyphs =
            item_name_rows[kf_enum_encode<u8>(player_state.equipped_weapon_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &text);
    }
    text.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.selected_magic_id != KF_MAGIC_NONE) {
        text.glyphs =
            magic_name_rows[kf_enum_encode<u8>(player_state.selected_magic_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &text);
    }
    text.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_shield_id != KF_OBJECT_NONE) {
        text.glyphs =
            item_name_rows[kf_enum_encode<u8>(player_state.equipped_shield_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &text);
    }
    text.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_head_armor_id != KF_OBJECT_NONE) {
        text.glyphs =
            item_name_rows[kf_enum_encode<u8>(player_state.equipped_head_armor_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &text);
    }
    text.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_body_armor_id != KF_OBJECT_NONE) {
        text.glyphs =
            item_name_rows[kf_enum_encode<u8>(player_state.equipped_body_armor_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &text);
    }
    text.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_arm_armor_id != KF_OBJECT_NONE) {
        text.glyphs =
            item_name_rows[kf_enum_encode<u8>(player_state.equipped_arm_armor_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &text);
    }
    text.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_leg_armor_id != KF_OBJECT_NONE) {
        text.glyphs =
            item_name_rows[kf_enum_encode<u8>(player_state.equipped_leg_armor_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &text);
    }
    text.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_accessory_id != KF_OBJECT_NONE) {
        text.glyphs =
            item_name_rows[kf_enum_encode<u8>(player_state.equipped_accessory_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &text);
    }
}

enum {
    MENU_INVENTORY_QUANTITY_LABEL_X = 230,
    MENU_INVENTORY_QUANTITY_VALUE_X = 279
};

void menu_item_model_preview(KfObjectId item_id)
{
    MenuGlyphString text;
    MATRIX rotation;
    MATRIX light_source;
    MATRIX light_result;
    const MenuGlyphRow *rows;
    const MenuGlyphRow *name;
    s32 i;

    if (item_id != KF_OBJECT_NONE) {
        rotation.t[0] = MENU_ITEM_PREVIEW_TRANSLATION_X;
        rotation.t[1] = MENU_ITEM_PREVIEW_TRANSLATION_Y;
        rotation.t[2] = MENU_ITEM_PREVIEW_TRANSLATION_Z;
        menu_item_preview_rotation.vy =
            (menu_item_preview_rotation.vy + MENU_ITEM_PREVIEW_YAW_STEP)
            & KF_ANGLE_WRAP_MASK;
        RotMatrix(&menu_item_preview_rotation, &rotation);

        menu_preview_light_source(&light_source);
        MulMatrix0(&light_source, &rotation, &light_result);
        SetLightMatrix(&light_result);
        SetRotMatrix(&rotation);
        SetTransMatrix(&rotation);
        menu_render_item_model();

        rows = item_name_rows;
        name = &rows[kf_enum_encode<s32>(item_id)];
        current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

        text.position.x = MENU_ITEM_NAME_X;
        text.position.y = MENU_ITEM_PREVIEW_NAME_Y;
        for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
            text.glyphs.codes[i] = name->codes[i];
        }
        menu_draw_string(&menu_assets.glyph_atlas, &text);

        text.position.x = MENU_INVENTORY_QUANTITY_LABEL_X;
        text.glyphs.codes[0] = 0xca;
        text.glyphs.codes[1] = 0xcb;
        text.glyphs.codes[2] = MENU_TEXT_END;
        text.position.y += MENU_ITEM_PREVIEW_LINE_HEIGHT;
        menu_draw_string(&menu_assets.glyph_atlas, &text);

        text.position.x = MENU_INVENTORY_QUANTITY_VALUE_X;
        menu_format_number(item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<s32>(item_id)], MENU_ITEM_PREVIEW_QUANTITY_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &text);
    }
}

enum {
    MENU_ITEM_DETAIL_PRICE_X = 200,
    MENU_ITEM_DETAIL_LABEL_X = 242,
    MENU_ITEM_DETAIL_QUANTITY_X = 284,
    MENU_ITEM_DETAIL_GOLD_X_OFFSET = 28,
    MENU_ITEM_DETAIL_PRICE_DIGITS = 6,
    MENU_ITEM_DETAIL_GOLD_DIGITS = 6,
    MENU_SAVE_SLOT0_QUAD = 2,
    MENU_SAVE_SLOT1_QUAD = 3,
    MENU_SAVE_SLOT2_QUAD = 4,
    MENU_SAVE_SUMMARY_ROW_HEIGHT = 65,
    MENU_SAVE_SUMMARY_LINE_HEIGHT = 14,
    MENU_SAVE_SUMMARY_LABEL_X = 181,
    MENU_SAVE_SUMMARY_VALUE_X = 230,
    MENU_SAVE_STATUS_DIGITS = 4
};

void menu_draw_item_detail(KfObjectId item_id, KfItemStockBank shop_bank, KfTradeMode price_mode)
{
    MenuGlyphString text;
    MATRIX rotation;
    MATRIX light_source;
    MATRIX light_result;
    s32 price;
    const MenuGlyphRow *rows;
    const s16 *glyph;
    s32 i;

    if (item_id == KF_OBJECT_NONE) {
        return;
    }

    rotation.t[0] = MENU_ITEM_PREVIEW_TRANSLATION_X;
    rotation.t[1] = MENU_ITEM_PREVIEW_TRANSLATION_Y;
    rotation.t[2] = MENU_ITEM_PREVIEW_TRANSLATION_Z;
    menu_item_preview_rotation.vy =
        (menu_item_preview_rotation.vy + MENU_ITEM_PREVIEW_YAW_STEP)
        & KF_ANGLE_WRAP_MASK;
    RotMatrix(&menu_item_preview_rotation, &rotation);

    menu_preview_light_source(&light_source);
    MulMatrix0(&light_source, &rotation, &light_result);
    SetLightMatrix(&light_result);
    SetRotMatrix(&rotation);
    SetTransMatrix(&rotation);
    menu_render_item_model();

    rows = item_name_rows;
    glyph = rows[kf_enum_encode<s32>(item_id)].codes;
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    text.position.x = MENU_ITEM_NAME_X;
    text.position.y = MENU_ITEM_PREVIEW_NAME_Y;

    do {
        i = 0;
    } while (0);
    for (; i < MENU_GLYPHS_PER_ROW; i++) {
        text.glyphs.codes[i] = *glyph++;
    }
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.position.x = MENU_ITEM_DETAIL_PRICE_X;
    text.position.y += MENU_ITEM_PREVIEW_LINE_HEIGHT;
    if (price_mode == KF_TRADE_BUY) {
        price = item_buy_prices[kf_enum_encode<s32>(item_id)]
            [kf_enum_encode<s32>(shop_bank) - kf_enum_encode<s32>(KF_ITEM_STOCK_FIRST_SHOP)];
    } else {
        price = item_sell_prices[kf_enum_encode<s32>(item_id)]
            [kf_enum_encode<s32>(shop_bank) - kf_enum_encode<s32>(KF_ITEM_STOCK_FIRST_SHOP)];
    }
    menu_format_number(price, MENU_ITEM_DETAIL_PRICE_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);

    text.position.x = MENU_ITEM_DETAIL_LABEL_X;
    text.glyphs.codes[0] = MENU_TEXT_DAKUTEN | 0x9;
    text.glyphs.codes[1] = 0x2d;
    text.glyphs.codes[2] = 0x2a;
    text.glyphs.codes[3] = MENU_TEXT_DAKUTEN | 0x13;
    text.glyphs.codes[4] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.position.x = MENU_ITEM_DETAIL_LABEL_X;
    text.glyphs.codes[0] = 0xca;
    text.glyphs.codes[1] = 0xcb;
    text.glyphs.codes[2] = MENU_TEXT_END;
    text.position.y += MENU_ITEM_PREVIEW_LINE_HEIGHT;
    menu_draw_string(&menu_assets.glyph_atlas, &text);

    text.position.x = MENU_ITEM_DETAIL_QUANTITY_X;
    menu_format_number(item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)][kf_enum_encode<s32>(item_id)], MENU_ITEM_PREVIEW_QUANTITY_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);

    menu_blit_sprite_translucent(
        &menu_assets.row_background,
        &menu_window_layouts[kf_enum_encode<s32>(KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD].position);
    menu_draw_string(&menu_assets.glyph_atlas, &menu_window_layouts[kf_enum_encode<s32>(KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD]);

    text.position.x = menu_window_layouts[kf_enum_encode<s32>(KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD].position.x + MENU_ITEM_DETAIL_GOLD_X_OFFSET;
    text.position.y = menu_window_layouts[kf_enum_encode<s32>(KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD].position.y;
    menu_format_number(player_state.gold, MENU_ITEM_DETAIL_GOLD_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &text);
}

void menu_add_magic_artwork_quad(void)
{
    AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_OVERLAY_OT_DEPTH),
            (void *)(&menu_assets.magic_artwork_quads[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)]));
}

void menu_add_message_image_quad(void)
{
    AddPrim((void *)game_graphics_runtime.display_state.ordering_table,
            (void *)(&menu_assets.message_image_quads[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)]));
}

void menu_draw_save_slots(const KfSaveSlotSummary *summaries, KfSaveSlotOverlay slot_overlay)
{
    MenuGlyphString text;
    s32 i;

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    if (slot_overlay == KF_SAVE_OVERLAY_SKIP_FIRST) {
        AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH),
                (void *)(&menu_assets.dialog_quads[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT1_QUAD]));
        AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH),
                (void *)(&menu_assets.dialog_quads[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT2_QUAD]));
    }
    if (slot_overlay == KF_SAVE_OVERLAY_SKIP_SECOND) {
        AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH),
                (void *)(&menu_assets.dialog_quads[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT0_QUAD]));
        AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH),
                (void *)(&menu_assets.dialog_quads[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT2_QUAD]));
    }
    if (slot_overlay == KF_SAVE_OVERLAY_SKIP_THIRD) {
        AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH),
                (void *)(&menu_assets.dialog_quads[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT0_QUAD]));
        AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH),
                (void *)(&menu_assets.dialog_quads[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT1_QUAD]));
    }
    if (slot_overlay >= KF_SAVE_OVERLAY_ALL) {
        AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH),
                (void *)(&menu_assets.dialog_quads[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT0_QUAD]));
        AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH),
                (void *)(&menu_assets.dialog_quads[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT1_QUAD]));
        AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH),
                (void *)(&menu_assets.dialog_quads[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT2_QUAD]));
    }

    if (summaries == NULL) {
        return;
    }

    for (i = 0; i < KF_SAVE_SLOT_COUNT; i++) {
        text.position.y = i * MENU_SAVE_SUMMARY_ROW_HEIGHT + 30;
        if ((s32)summaries[i].current_hp > 0) {
            text.position.x = MENU_SAVE_SUMMARY_LABEL_X;
            text.glyphs.codes[0] = 0x82;
            text.glyphs.codes[1] = 0x83;
            text.glyphs.codes[2] = 0x84;
            text.glyphs.codes[3] = MENU_TEXT_END;
            menu_draw_string(&menu_assets.glyph_atlas, &text);

            text.position.x = 251;
            menu_format_number(summaries[i].experience, 6, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
            menu_draw_number(&menu_assets.number_atlas, &text);

            text.position.x = MENU_SAVE_SUMMARY_LABEL_X;
            text.glyphs.codes[0] = 0xcc;
            text.glyphs.codes[1] = 0xcd;
            text.glyphs.codes[2] = MENU_TEXT_END;
            text.position.y += MENU_SAVE_SUMMARY_LINE_HEIGHT;
            menu_draw_string(&menu_assets.glyph_atlas, &text);

            text.position.x = 286;
            menu_format_number(kf_enum_encode<u32>(summaries[i].current_floor), 1, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
            menu_draw_number(&menu_assets.number_atlas, &text);

            text.position.x = MENU_SAVE_SUMMARY_LABEL_X;
            text.glyphs.codes[0] = 0xf0;
            text.glyphs.codes[1] = 242;
            text.glyphs.codes[2] = MENU_TEXT_END;
            text.position.y += MENU_SAVE_SUMMARY_LINE_HEIGHT;
            menu_draw_string(&menu_assets.glyph_atlas, &text);

            text.position.x = MENU_SAVE_SUMMARY_VALUE_X;
            menu_format_number((summaries[i].current_hp), MENU_SAVE_STATUS_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
            menu_draw_number(&menu_assets.number_atlas, &text);
            text.glyphs.codes[0] = MENU_NUMBER_SLASH;
            text.glyphs.codes[1] = MENU_TEXT_END;
            text.position.x += MENU_SAVE_STATUS_DIGITS * MENU_NUMBER_ADVANCE;
            menu_draw_number(&menu_assets.number_atlas, &text);
            text.position.x += MENU_NUMBER_ADVANCE;
            menu_format_number((summaries[i].maximum_hp), MENU_SAVE_STATUS_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
            menu_draw_number(&menu_assets.number_atlas, &text);

            text.position.x = MENU_SAVE_SUMMARY_LABEL_X;
            text.glyphs.codes[0] = 0xf1;
            text.glyphs.codes[1] = 242;
            text.glyphs.codes[2] = MENU_TEXT_END;
            text.position.y += MENU_SAVE_SUMMARY_LINE_HEIGHT;
            menu_draw_string(&menu_assets.glyph_atlas, &text);

            text.position.x = MENU_SAVE_SUMMARY_VALUE_X;
            menu_format_number((summaries[i].current_mp), MENU_SAVE_STATUS_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
            menu_draw_number(&menu_assets.number_atlas, &text);
            text.glyphs.codes[0] = MENU_NUMBER_SLASH;
            text.glyphs.codes[1] = MENU_TEXT_END;
            text.position.x += MENU_SAVE_STATUS_DIGITS * MENU_NUMBER_ADVANCE;
            menu_draw_number(&menu_assets.number_atlas, &text);
            text.position.x += MENU_NUMBER_ADVANCE;
            menu_format_number((summaries[i].maximum_mp), MENU_SAVE_STATUS_DIGITS, KF_FORMAT_PAD_SPACES, text.glyphs.codes);
            menu_draw_number(&menu_assets.number_atlas, &text);
        }
    }
}

enum {
    MENU_LIST_CONFIRM_ACCEPT_Y = 185,
    MENU_LIST_CONFIRM_DECLINE_Y = MENU_LIST_CONFIRM_ACCEPT_Y + MENU_CONFIRM_ROW_STEP
};

static KfMenuResult menu_list_confirm_impl(
    const KfMenuList *list, KfMenuConfirmKind confirm_kind, KfMenuPreviewMode preview_mode,
    s32 preview_id, KfItemStockBank shop_bank, KfTradeMode price_mode)
{
    MenuGlyphString accept_label;
    MenuGlyphString decline_label;
    KfMenuConfirmChoice selected;
    KfEnumStorage<KfMenuConfirmState, u32> confirmation;
    u32 input;
    u32 prev;
    KfMenuResult result;

    selected = KF_MENU_CHOICE_ACCEPT;
    confirmation = KF_MENU_CONFIRM_IDLE;
    input = 0;
    result = KF_MENU_RESULT_PENDING;
    while (PadRead(1) != 0) {
    }

    accept_label.position.x = MENU_CONFIRM_TEXT_X;
    accept_label.position.y = MENU_LIST_CONFIRM_ACCEPT_Y;
    decline_label.position.x = MENU_CONFIRM_TEXT_X;
    decline_label.position.y = MENU_LIST_CONFIRM_DECLINE_Y;
    if (confirm_kind == KF_MENU_CONFIRM_USE) {
        accept_label.glyphs.codes[0] = 0x72;
        accept_label.glyphs.codes[1] = 0x42;
        accept_label.glyphs.codes[2] = MENU_TEXT_END;
    } else if (confirm_kind == KF_MENU_CONFIRM_DROP) {
        accept_label.glyphs.codes[0] = 0x75;
        accept_label.glyphs.codes[1] = 0x52;
        accept_label.glyphs.codes[2] = 0x6a;
        accept_label.glyphs.codes[3] = MENU_TEXT_END;
    } else if (confirm_kind == KF_MENU_CONFIRM_YES_NO) {
        accept_label.glyphs.codes[0] = 0x59;
        accept_label.glyphs.codes[1] = 0x41;
        accept_label.glyphs.codes[2] = MENU_TEXT_END;
    } else if (confirm_kind == KF_MENU_CONFIRM_BUY) {
        accept_label.glyphs.codes[0] = 0x74;
        accept_label.glyphs.codes[1] = 0x42;
        accept_label.glyphs.codes[2] = MENU_TEXT_END;
    } else if (confirm_kind == KF_MENU_CONFIRM_SELL) {
        accept_label.glyphs.codes[0] = 0x73;
        accept_label.glyphs.codes[1] = 0x6a;
        accept_label.glyphs.codes[2] = MENU_TEXT_END;
    } else {
        accept_label.glyphs.codes[0] = 0x70;
        accept_label.glyphs.codes[1] = 0x71;
        accept_label.glyphs.codes[2] = MENU_TEXT_END;
    }
    if (confirm_kind == KF_MENU_CONFIRM_YES_NO) {
        decline_label.glyphs.codes[0] = 0x41;
        decline_label.glyphs.codes[1] = 0x41;
        decline_label.glyphs.codes[2] = 0x43;
    } else {
        decline_label.glyphs.codes[0] = 99;
        decline_label.glyphs.codes[1] = 0x61;
        decline_label.glyphs.codes[2] = 0x6a;
    }
    decline_label.glyphs.codes[3] = MENU_TEXT_END;

    menu_frame_begin();
    if (preview_mode == KF_MENU_PREVIEW_ITEM_MODEL) {
        menu_item_model_preview(kf_enum_decode<KfObjectId>(preview_id));
    } else if (preview_mode == KF_MENU_PREVIEW_ITEM_DETAIL) {
        menu_draw_item_detail(kf_enum_decode<KfObjectId>(preview_id), shop_bank, price_mode);
    } else if (preview_mode == KF_MENU_PREVIEW_MAGIC_ARTWORK
            && preview_id != kf_enum_encode<s32>(KF_MAGIC_NONE)) {
        menu_add_magic_artwork_quad();
    }
    menu_list_render(list);
    menu_draw_two_option(&accept_label, &decline_label, selected, confirmation);
    menu_present_frame();
    do {
        if (result != KF_MENU_RESULT_PENDING) {
            menu_frame_begin();
            if (preview_mode == KF_MENU_PREVIEW_ITEM_MODEL) {
                menu_item_model_preview(kf_enum_decode<KfObjectId>(preview_id));
            } else if (preview_mode == KF_MENU_PREVIEW_ITEM_DETAIL) {
                menu_draw_item_detail(kf_enum_decode<KfObjectId>(preview_id), shop_bank, price_mode);
            } else if (preview_mode == KF_MENU_PREVIEW_MAGIC_ARTWORK
                    && preview_id != kf_enum_encode<s32>(KF_MAGIC_NONE)) {
                menu_add_magic_artwork_quad();
            }
            menu_list_render(list);
            menu_draw_two_option(&accept_label, &decline_label, selected, confirmation);
            menu_present_frame();
            while (PadRead(1) != 0) {
            }
            return result;
        }

        confirmation = KF_MENU_CONFIRM_IDLE;
        menu_frame_begin();
        prev = input;
        input = PadRead(1);
        if ((PAD_PRESSED(input, prev, PADLup)) ||
            (PAD_PRESSED(input, prev, PADLdown))) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (selected == KF_MENU_CHOICE_ACCEPT) {
                selected = KF_MENU_CHOICE_DECLINE;
            } else {
                selected = KF_MENU_CHOICE_ACCEPT;
            }
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirmation = KF_MENU_CONFIRM_REQUESTED;
            result = menu_confirm_result_from_choice(selected);
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_CANCELLED;
        }
        if (preview_mode == KF_MENU_PREVIEW_ITEM_MODEL) {
            menu_item_model_preview(kf_enum_decode<KfObjectId>(preview_id));
        } else if (preview_mode == KF_MENU_PREVIEW_ITEM_DETAIL) {
            menu_draw_item_detail(kf_enum_decode<KfObjectId>(preview_id), shop_bank, price_mode);
        } else if (preview_mode == KF_MENU_PREVIEW_MAGIC_ARTWORK
                && preview_id != kf_enum_encode<s32>(KF_MAGIC_NONE)) {
            menu_add_magic_artwork_quad();
        }
        menu_list_render(list);
        menu_draw_two_option(&accept_label, &decline_label, selected, confirmation);
        menu_present_frame();
    } while (1);
}

enum {
    MENU_PROMPT_ACCEPT_Y_OFFSET = 44,
    MENU_PROMPT_DECLINE_Y_OFFSET = MENU_PROMPT_ACCEPT_Y_OFFSET + MENU_CONFIRM_ROW_STEP
};

KfMenuResult menu_two_option_prompt(
    KfMenuWindowKind window_kind, s32 row_count, s32 highlight_row,
    const KfSaveSlotSummary *summaries)
{
    MenuGlyphString accept_label;
    MenuGlyphString decline_label;
    KfMenuConfirmChoice selected = KF_MENU_CHOICE_ACCEPT;
    KfMenuConfirmState confirmation = KF_MENU_CONFIRM_IDLE;
    KfSaveSlotOverlay slot_overlay = KF_SAVE_OVERLAY_NONE;
    s32 input = 0;
    s32 prev;
    KfMenuResult result = KF_MENU_RESULT_PENDING;

    while (PadRead(1) != 0)
        ;

    if (window_kind == KF_MENU_WINDOW_SAVE || window_kind == KF_MENU_WINDOW_LOAD)
        slot_overlay = kf_enum_decode<KfSaveSlotOverlay>(highlight_row);

    accept_label.position.x = MENU_CONFIRM_TEXT_X;
    accept_label.position.y = row_count * MENU_CONFIRM_ROW_STEP + MENU_PROMPT_ACCEPT_Y_OFFSET;
    accept_label.glyphs.codes[0] = 0x59;
    accept_label.glyphs.codes[1] = 0x41;
    accept_label.glyphs.codes[2] = MENU_TEXT_END;
    decline_label.position.x = MENU_CONFIRM_TEXT_X;
    decline_label.position.y = row_count * MENU_CONFIRM_ROW_STEP + MENU_PROMPT_DECLINE_Y_OFFSET;
    decline_label.glyphs.codes[0] = 0x41;
    decline_label.glyphs.codes[1] = 0x41;
    decline_label.glyphs.codes[2] = 0x43;
    decline_label.glyphs.codes[3] = MENU_TEXT_END;

    for (;;) {
        if (result != KF_MENU_RESULT_PENDING) {
            menu_frame_begin();
            menu_draw_save_slots(summaries, slot_overlay);
            menu_draw_window(window_kind, row_count, highlight_row, KF_MENU_CONFIRM_REQUESTED);
            menu_draw_two_option(&accept_label, &decline_label, selected, confirmation);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
            return result;
        }

        confirmation = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = PadRead(1);
        if ((PAD_PRESSED(input, prev, PADLup)) ||
            (PAD_PRESSED(input, prev, PADLdown))) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (selected != KF_MENU_CHOICE_ACCEPT)
                selected = KF_MENU_CHOICE_ACCEPT;
            else
                selected = KF_MENU_CHOICE_DECLINE;
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirmation = KF_MENU_CONFIRM_REQUESTED;
            result = menu_confirm_result_from_choice(selected);
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_CANCELLED;
        }

        menu_frame_begin();
        menu_draw_save_slots(summaries, slot_overlay);
        menu_draw_window(window_kind, row_count, highlight_row, KF_MENU_CONFIRM_REQUESTED);
        menu_draw_two_option(&accept_label, &decline_label, selected, confirmation);
        menu_present_frame();
    }
}

void menu_draw_window(KfMenuWindowKind window_kind, s32 row_count, s32 highlight_row, KfMenuConfirmState confirmation)
{
    const MenuWindowLayout *layout;
    s32 row;

    layout = &menu_window_layouts[kf_enum_encode<s32>(window_kind)];
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    if (layout->title.position.x != 0) {
        menu_blit_sprite_translucent(
            &menu_assets.row_background, &layout->title.position);
        menu_draw_string(&menu_assets.glyph_atlas, &layout->title);
    }
    if (row_count > 0) {
        row = 0;
        do {
            if (row == highlight_row && confirmation == KF_MENU_CONFIRM_REQUESTED) {
                menu_blit_sprite_translucent(&menu_assets.row_confirmed_background,
                    &layout->rows[row].position);
            } else {
                menu_blit_sprite_translucent(&menu_assets.row_background,
                    &layout->rows[row].position);
            }
            if (row == highlight_row) {
                menu_blit_sprite(&menu_assets.selection_cursor, &layout->rows[row].position);
            }
            menu_draw_string(&menu_assets.glyph_atlas, &layout->rows[row]);
            row++;
        } while (row < row_count);
    }
    if (window_kind != KF_MENU_WINDOW_CONFIG) {
        menu_draw_window_backdrop();
    }
}

enum {
    MENU_LIST_TEXT_INSET = 3,
    MENU_LIST_ROW_HEIGHT = 12,
    MENU_LIST_QUANTITY_X_OFFSET = 110,
    MENU_LIST_QUANTITY_Y_OFFSET = 1
};

void menu_list_render(const KfMenuList *list)
{
    MenuGlyphString text;
    const MenuTileSprite *tile;
    s16 *glyph;
    u8 *quantity;
    s32 row;
    s32 i;
    s32 yoff;
    u32 tens;
    u32 ones;

    glyph = list->glyph_rows;
    quantity = list->quantities;
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    if (list->title.position.x != 0) {
        menu_blit_sprite_translucent(
            &menu_assets.row_background, &list->title.position);
        menu_draw_string(
            &menu_assets.glyph_atlas, &list->title);
    }

    quantity = quantity + list->scroll_offset;
    glyph = glyph + list->scroll_offset * list->glyphs_per_entry;
    row = 0;
    if (row < list->visible_rows && row < list->entry_count) {
        do {
            text.position.x = list->list_x + MENU_LIST_TEXT_INSET;
            text.position.y = list->list_y + MENU_LIST_TEXT_INSET;
            text.position.y += row * MENU_LIST_ROW_HEIGHT;
            for (i = 0; i < list->glyphs_per_entry; i++) {
                text.glyphs.codes[i] = *glyph++;
            }
            menu_draw_string(
                &menu_assets.glyph_atlas, &text);
            if (list->quantities != NULL) {
                tens = *quantity / 10u;
                ones = *quantity % 10u;
                text.position.x += MENU_LIST_QUANTITY_X_OFFSET;
                text.position.y += MENU_LIST_QUANTITY_Y_OFFSET;
                text.glyphs.codes[0] = tens;
                if (tens == 0) {
                    text.glyphs.codes[0] = MENU_NUMBER_BLANK;
                }
                text.glyphs.codes[1] = ones;
                text.glyphs.codes[2] = MENU_TEXT_END;
                menu_draw_number(
                    &menu_assets.number_atlas, &text);
                quantity++;
            }
        } while (++row < list->visible_rows && row < list->entry_count);
    }

    tile = &menu_assets.list_tiles[MENU_LIST_TILE_BACKDROP];
    menu_begin_poly_ft4();
    current_poly_ft4->tpage = (tile)->tpage;
    current_poly_ft4->clut = (tile)->clut;
    setXYWH (current_poly_ft4, (list->list_x), (list->list_y), (tile)->width, (tile)->height);
    setUVWH (current_poly_ft4, (tile)->u, (tile)->v, (tile)->width, (tile)->height);
    menu_enable_window_blending();
    menu_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);

    row = 0;
    if (row < list->visible_rows) {
        yoff = 0;
        do {
            tile = &menu_assets.list_tiles[MENU_LIST_TILE_ROW];
            if (row == list->cursor_row) {
                tile = &menu_assets.list_tiles[MENU_LIST_TILE_SELECTED];
            }
            row++;
            menu_begin_poly_ft4();
            current_poly_ft4->tpage = (tile)->tpage;
            current_poly_ft4->clut = (tile)->clut;
            setXYWH (current_poly_ft4, (list->list_x), (list->list_y + yoff + MENU_LIST_TEXT_INSET), (tile)->width, (tile)->height);
            setUVWH (current_poly_ft4, (tile)->u, (tile)->v, (tile)->width, (tile)->height);
            menu_enable_window_blending();
            menu_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);
            yoff += MENU_LIST_ROW_HEIGHT;
        } while (row < list->visible_rows);
    }

    tile = &menu_assets.list_tiles[MENU_LIST_TILE_END];
    menu_begin_poly_ft4();
    current_poly_ft4->tpage = (tile)->tpage;
    current_poly_ft4->clut = (tile)->clut;
    setXYWH (current_poly_ft4, (list->list_x), (list->list_y + list->visible_rows * MENU_LIST_ROW_HEIGHT + MENU_LIST_TEXT_INSET), (tile)->width, (tile)->height);
    setUVWH (current_poly_ft4, (tile)->u, (tile)->v, (tile)->width, (tile)->height);
    menu_enable_window_blending();
    menu_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);

    menu_draw_window_backdrop();
}

enum {
    MENU_TRANSLUCENT_SPRITE_X_OFFSET = 4,
    MENU_TRANSLUCENT_SPRITE_Y_OFFSET = 3,
    MENU_OPAQUE_SPRITE_X_OFFSET = 18,
    MENU_OPAQUE_SPRITE_Y_OFFSET = 2,
    MENU_PRIMITIVE_BRIGHTNESS = 96,
    MENU_LIST_DEFAULT_VISIBLE_ROWS = 11,
    MENU_LIST_DEFAULT_GLYPHS_PER_ENTRY = 8,
    MENU_BACKDROP_LEFT_X = 166,
    MENU_BACKDROP_RIGHT_X = MENU_BACKDROP_LEFT_X + MENU_BACKDROP_COLUMN_STEP,
    MENU_PICKUP_BACKDROP_LEFT_X = 118,
    MENU_PICKUP_BACKDROP_RIGHT_X = MENU_PICKUP_BACKDROP_LEFT_X + MENU_BACKDROP_COLUMN_STEP
};

enum {
    MENU_FONT_COLUMNS = 16,
    MENU_FONT_CELL_WIDTH = 14,
    MENU_FONT_CELL_HEIGHT = 12,
    MENU_NUMBER_CELL_HEIGHT = 11,

    MENU_DAKUTEN_U = 14 * MENU_FONT_CELL_WIDTH,
    MENU_HANDAKUTEN_U = 15 * MENU_FONT_CELL_WIDTH,
    MENU_KANA_MARK_V = 2 * MENU_FONT_CELL_HEIGHT
};

void menu_draw_two_option(
    const MenuGlyphString *accept_label, const MenuGlyphString *decline_label,
    KfMenuConfirmChoice selected_choice, KfMenuConfirmState confirmation)
{
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    if (selected_choice == KF_MENU_CHOICE_ACCEPT) {
        menu_blit_sprite(&menu_assets.selection_cursor, &accept_label->position);
    } else {
        menu_blit_sprite(&menu_assets.selection_cursor, &decline_label->position);
    }
    if (confirmation == KF_MENU_CONFIRM_REQUESTED) {
        if (selected_choice == KF_MENU_CHOICE_ACCEPT) {
            menu_blit_sprite_translucent(
                &menu_assets.option_highlight, &accept_label->position);
            menu_blit_sprite_translucent(
                &menu_assets.option_background, &decline_label->position);
        } else {
            menu_blit_sprite_translucent(
                &menu_assets.option_background, &accept_label->position);
            menu_blit_sprite_translucent(
                &menu_assets.option_highlight, &decline_label->position);
        }
    } else {
        menu_blit_sprite_translucent(
            &menu_assets.option_background, &accept_label->position);
        menu_blit_sprite_translucent(
            &menu_assets.option_background, &decline_label->position);
    }
    menu_draw_string(&menu_assets.glyph_atlas, accept_label);
    menu_draw_string(&menu_assets.glyph_atlas, decline_label);
}

void menu_draw_pickup_preview(KfObjectId item_id)
{
    MenuGlyphString string;
    MATRIX rotation;
    MATRIX light_source;
    MATRIX light_result;
    const MenuGlyphRow *rows;
    const MenuGlyphRow *name;
    s32 i;

    rotation.t[0] = 0xdc;
    rotation.t[1] = 0x8c;
    rotation.t[2] = 0x5dc;
    menu_item_preview_rotation.vy =
        (menu_item_preview_rotation.vy + MENU_PICKUP_PREVIEW_YAW_STEP)
        & KF_ANGLE_WRAP_MASK;
    RotMatrix(&menu_item_preview_rotation, &rotation);

    menu_preview_light_source(&light_source);
    MulMatrix0(&light_source, &rotation, &light_result);
    SetLightMatrix(&light_result);
    SetRotMatrix(&rotation);
    SetTransMatrix(&rotation);
    menu_render_item_model();

    rows = item_name_rows;
    name = &rows[kf_enum_encode<s32>(item_id)];
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    string.position.x = 0x80;
    string.position.y = 0x24;
    for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
        string.glyphs.codes[i] = name->codes[i];
    }
    menu_draw_string(&menu_assets.glyph_atlas, &string);

    menu_begin_poly_ft4();
    menu_enable_window_blending();
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH (current_poly_ft4, MENU_PICKUP_BACKDROP_LEFT_X, MENU_BACKDROP_TOP_Y, menu_assets.window_backdrop.width, menu_assets.window_backdrop.height);
    setUVWH (current_poly_ft4, KF_FALSE ? menu_assets.window_backdrop.u + menu_assets.window_backdrop.width : menu_assets.window_backdrop.u, KF_FALSE ? menu_assets.window_backdrop.v + menu_assets.window_backdrop.height : menu_assets.window_backdrop.v, KF_FALSE ? - menu_assets.window_backdrop.width : menu_assets.window_backdrop.width, KF_FALSE ? - menu_assets.window_backdrop.height : menu_assets.window_backdrop.height);
    menu_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    menu_begin_poly_ft4();
    menu_enable_window_blending();
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH (current_poly_ft4, MENU_PICKUP_BACKDROP_RIGHT_X, MENU_BACKDROP_TOP_Y, menu_assets.window_backdrop.width, menu_assets.window_backdrop.height);
    setUVWH (current_poly_ft4, KF_TRUE ? menu_assets.window_backdrop.u + menu_assets.window_backdrop.width : menu_assets.window_backdrop.u, KF_FALSE ? menu_assets.window_backdrop.v + menu_assets.window_backdrop.height : menu_assets.window_backdrop.v, KF_TRUE ? - menu_assets.window_backdrop.width : menu_assets.window_backdrop.width, KF_FALSE ? - menu_assets.window_backdrop.height : menu_assets.window_backdrop.height);
    menu_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    menu_begin_poly_ft4();
    menu_enable_window_blending();
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH (current_poly_ft4, MENU_PICKUP_BACKDROP_LEFT_X, MENU_BACKDROP_BOTTOM_Y, menu_assets.window_backdrop.width, menu_assets.window_backdrop.height);
    setUVWH (current_poly_ft4, KF_FALSE ? menu_assets.window_backdrop.u + menu_assets.window_backdrop.width : menu_assets.window_backdrop.u, KF_TRUE ? menu_assets.window_backdrop.v + menu_assets.window_backdrop.height : menu_assets.window_backdrop.v, KF_FALSE ? - menu_assets.window_backdrop.width : menu_assets.window_backdrop.width, KF_TRUE ? - menu_assets.window_backdrop.height : menu_assets.window_backdrop.height);
    menu_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    menu_begin_poly_ft4();
    menu_enable_window_blending();
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH (current_poly_ft4, MENU_PICKUP_BACKDROP_RIGHT_X, MENU_BACKDROP_BOTTOM_Y, menu_assets.window_backdrop.width, menu_assets.window_backdrop.height);
    setUVWH (current_poly_ft4, KF_TRUE ? menu_assets.window_backdrop.u + menu_assets.window_backdrop.width : menu_assets.window_backdrop.u, KF_TRUE ? menu_assets.window_backdrop.v + menu_assets.window_backdrop.height : menu_assets.window_backdrop.v, KF_TRUE ? - menu_assets.window_backdrop.width : menu_assets.window_backdrop.width, KF_TRUE ? - menu_assets.window_backdrop.height : menu_assets.window_backdrop.height);
    menu_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    MENU_ENQUEUE_BACKGROUND();
}

void menu_blit_sprite_translucent(
    const MenuSpriteDef *sprite, const MenuPoint *position)
{
    menu_begin_poly_ft4();
    current_poly_ft4->tpage = sprite->tpage;
    current_poly_ft4->clut = sprite->clut;
    setXYWH(current_poly_ft4, position->x - MENU_TRANSLUCENT_SPRITE_X_OFFSET, position->y - MENU_TRANSLUCENT_SPRITE_Y_OFFSET,
        sprite->width, sprite->height);
    setUVWH(current_poly_ft4, sprite->u, sprite->v, sprite->width, sprite->height);
    menu_enable_window_blending();
    menu_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);
}

void menu_blit_sprite(
    const MenuSpriteDef *sprite, const MenuPoint *position)
{
    menu_begin_poly_ft4();
    current_poly_ft4->tpage = sprite->tpage;
    current_poly_ft4->clut = sprite->clut;
    setXYWH(current_poly_ft4, position->x - MENU_OPAQUE_SPRITE_X_OFFSET, position->y - MENU_OPAQUE_SPRITE_Y_OFFSET,
        sprite->width, sprite->height);
    setUVWH(current_poly_ft4, sprite->u, sprite->v, sprite->width, sprite->height);
    menu_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);
}

static inline void menu_begin_text_glyph(
    const MenuSpriteDef *font, const MenuGlyphString *string, s32 x_offset)
{
    menu_begin_poly_ft4();
    current_poly_ft4->tpage = font->tpage;
    current_poly_ft4->clut = font->clut;
    setXYWH(current_poly_ft4, string->position.x + x_offset,
        string->position.y, font->width, font->height);
}

void menu_draw_string(
    const MenuSpriteDef *font, const MenuGlyphString *string)
{
    s32 i;
    s32 x_offset;

    for (i = 0; string->glyphs.codes[i] != MENU_TEXT_END; i++) {
        s32 glyph;

        x_offset = i * MENU_FONT_CELL_WIDTH;
        menu_begin_text_glyph(font, string, x_offset);
        glyph = string->glyphs.codes[i] & MENU_TEXT_GLYPH_MASK;
        setUVWH(current_poly_ft4,
            (glyph % MENU_FONT_COLUMNS) * MENU_FONT_CELL_WIDTH,
            (glyph / MENU_FONT_COLUMNS) * MENU_FONT_CELL_HEIGHT,
            font->width,
            font->height);
        menu_commit_poly_ft4(MENU_CONTENT_OT_DEPTH);

        if (string->glyphs.codes[i] & MENU_TEXT_DAKUTEN) {
            menu_begin_text_glyph(font, string, x_offset);
            setUVWH(current_poly_ft4, MENU_DAKUTEN_U, MENU_KANA_MARK_V, font->width, font->height);
            menu_commit_poly_ft4(MENU_CONTENT_OT_DEPTH);
        }

        if (string->glyphs.codes[i] & MENU_TEXT_HANDAKUTEN) {
            menu_begin_text_glyph(font, string, x_offset);
            setUVWH(current_poly_ft4,
                MENU_HANDAKUTEN_U,
                MENU_KANA_MARK_V,
                font->width,
                font->height);
            menu_commit_poly_ft4(MENU_CONTENT_OT_DEPTH);
        }
    }
}

void menu_draw_number(
    const MenuSpriteDef *font, const MenuGlyphString *string)
{
    const MenuSpriteDef *atlas = font;
    const MenuGlyphString *label = string;
    s32 i;
    s32 x_offset;

    for (i = 0; label->glyphs.codes[i] != MENU_TEXT_END; i++) {
        x_offset = i * MENU_NUMBER_ADVANCE;
        menu_begin_poly_ft4();
        current_poly_ft4->tpage = atlas->tpage;
        current_poly_ft4->clut = atlas->clut;
        setXYWH(current_poly_ft4,
            label->position.x + x_offset,
            label->position.y,
            atlas->width,
            atlas->height);
        setUVWH(current_poly_ft4,
            atlas->u,
            label->glyphs.codes[i] * MENU_NUMBER_CELL_HEIGHT,
            atlas->width,
            atlas->height);
        menu_commit_poly_ft4(MENU_CONTENT_OT_DEPTH);
    }
}

void menu_draw_window_backdrop(void)
{
    menu_begin_poly_ft4();
    menu_enable_window_blending();
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH (current_poly_ft4, MENU_BACKDROP_LEFT_X, MENU_BACKDROP_TOP_Y, menu_assets.window_backdrop.width, menu_assets.window_backdrop.height);
    setUVWH (current_poly_ft4, KF_FALSE ? menu_assets.window_backdrop.u + menu_assets.window_backdrop.width : menu_assets.window_backdrop.u, KF_FALSE ? menu_assets.window_backdrop.v + menu_assets.window_backdrop.height : menu_assets.window_backdrop.v, KF_FALSE ? - menu_assets.window_backdrop.width : menu_assets.window_backdrop.width, KF_FALSE ? - menu_assets.window_backdrop.height : menu_assets.window_backdrop.height);
    menu_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    menu_begin_poly_ft4();
    menu_enable_window_blending();
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH (current_poly_ft4, MENU_BACKDROP_RIGHT_X, MENU_BACKDROP_TOP_Y, menu_assets.window_backdrop.width, menu_assets.window_backdrop.height);
    setUVWH (current_poly_ft4, KF_TRUE ? menu_assets.window_backdrop.u + menu_assets.window_backdrop.width : menu_assets.window_backdrop.u, KF_FALSE ? menu_assets.window_backdrop.v + menu_assets.window_backdrop.height : menu_assets.window_backdrop.v, KF_TRUE ? - menu_assets.window_backdrop.width : menu_assets.window_backdrop.width, KF_FALSE ? - menu_assets.window_backdrop.height : menu_assets.window_backdrop.height);
    menu_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    menu_begin_poly_ft4();
    menu_enable_window_blending();
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH (current_poly_ft4, MENU_BACKDROP_LEFT_X, MENU_BACKDROP_BOTTOM_Y, menu_assets.window_backdrop.width, menu_assets.window_backdrop.height);
    setUVWH (current_poly_ft4, KF_FALSE ? menu_assets.window_backdrop.u + menu_assets.window_backdrop.width : menu_assets.window_backdrop.u, KF_TRUE ? menu_assets.window_backdrop.v + menu_assets.window_backdrop.height : menu_assets.window_backdrop.v, KF_FALSE ? - menu_assets.window_backdrop.width : menu_assets.window_backdrop.width, KF_TRUE ? - menu_assets.window_backdrop.height : menu_assets.window_backdrop.height);
    menu_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    menu_begin_poly_ft4();
    menu_enable_window_blending();
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH (current_poly_ft4, MENU_BACKDROP_RIGHT_X, MENU_BACKDROP_BOTTOM_Y, menu_assets.window_backdrop.width, menu_assets.window_backdrop.height);
    setUVWH (current_poly_ft4, KF_TRUE ? menu_assets.window_backdrop.u + menu_assets.window_backdrop.width : menu_assets.window_backdrop.u, KF_TRUE ? menu_assets.window_backdrop.v + menu_assets.window_backdrop.height : menu_assets.window_backdrop.v, KF_TRUE ? - menu_assets.window_backdrop.width : menu_assets.window_backdrop.width, KF_TRUE ? - menu_assets.window_backdrop.height : menu_assets.window_backdrop.height);
    menu_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    MENU_ENQUEUE_BACKGROUND();
}

void menu_frame_begin(void)
{
    game_graphics_runtime.display_state.buffer_index = display_next_buffer(game_graphics_runtime.display_state.buffer_index);
    game_graphics_runtime.display_state.primitive_buffer =
        &game_graphics_runtime.display_state.primitive_buffers[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)];
    game_graphics_runtime.display_state.ordering_table =
        game_graphics_runtime.display_state.ordering_tables[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)].entries;
    ClearOTagR(game_graphics_runtime.display_state.ordering_table, KF_ORDERING_TABLE_LENGTH);
    game_graphics_runtime.display_state.primitive_buffer->cursor =
        game_graphics_runtime.display_state.primitive_buffer->start;
}

void menu_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&game_graphics_runtime.display_draw_environments[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)]);
    PutDispEnv(&game_graphics_runtime.display_disp_environments[kf_enum_encode<u8>(game_graphics_runtime.display_state.buffer_index)]);
    DrawOTag(game_graphics_runtime.display_state.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));
}

void menu_begin_poly_ft4(void)
{
    SetPolyFT4(current_poly_ft4);
    setRGB0(current_poly_ft4,
        MENU_PRIMITIVE_BRIGHTNESS,
        MENU_PRIMITIVE_BRIGHTNESS,
        MENU_PRIMITIVE_BRIGHTNESS);
}

void menu_commit_poly_ft4(s32 depth)
{
    AddPrim(
        (void *)(&game_graphics_runtime.display_state.ordering_table[depth]),
        (void *)current_poly_ft4);
    current_poly_ft4++;
    game_graphics_runtime.display_state.primitive_buffer->cursor = (u8 *)current_poly_ft4;
}

void menu_list_init(KfMenuList *list, KfMenuWindowKind window_kind, s32 title_row)
{
    s32 i;

    list->title.position.x = 12;
    list->title.position.y = 19;
    for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
        list->title.glyphs.codes[i] = menu_window_layouts[kf_enum_encode<s32>(window_kind)].rows[title_row].glyphs.codes[i];
    }
    list->list_x = 0x16;
    list->list_y = 0x26;
    list->entry_count = 0;
    list->visible_rows = MENU_LIST_DEFAULT_VISIBLE_ROWS;
    list->scroll_offset = 0;
    list->selected_index = 0;
    list->cursor_row = 0;
    list->glyphs_per_entry = MENU_LIST_DEFAULT_GLYPHS_PER_ENTRY;
}

void menu_format_number(s32 value, s32 digit_count, KfFormatPaddingMode padding_mode, s16 *out)
{
    s32 i = 0;
    s32 blank;

    blank = (padding_mode == KF_FORMAT_PAD_SPACES) ? MENU_NUMBER_BLANK : MENU_NUMBER_ZERO;
    for (; i < digit_count; i++) {
        out[i] = blank;
    }
    out[digit_count] = MENU_TEXT_END;
    for (i = digit_count - 1; i >= 0; i--) {
        out[i] = value % 10;
        value /= 10;
        if (value == 0) {
            i = -1;
        }
    }
}

KfResourceLoadResult menu_load_item_model(KfObjectId item_id)
{
    u8 *asset;

    menu_release_item_model();
    if (item_id != KF_OBJECT_NONE) {
        if (cd_file_load_table_entry(&asset, kf_enum_encode<s32>(item_id)) != KF_RESOURCE_LOADED) {
            return KF_RESOURCE_LOAD_FAILED;
        }
        tmd_register(KF_TMD_SLOT_MENU_ITEM, (KfTmdHeader *)asset);
        menu_item_model_allocation = KF_MENU_MODEL_ALLOCATED;
    }
    menu_item_preview_rotation.vy = 0;
    return KF_RESOURCE_LOADED;
}

void menu_release_item_model(void)
{
    if (menu_item_model_allocation == KF_MENU_MODEL_ALLOCATED) {
        tmd_release_last_allocation(KF_TMD_SLOT_MENU_ITEM);
        menu_item_model_allocation = KF_MENU_MODEL_RELEASED;
    }
}

KfResourceLoadResult menu_load_texture(KfMenuTextureId texture_id)
{
    char name[16] = "TIM\\M000.";
    u8 *destination;
    s32 number;

    if (texture_id != KF_MENU_TEXTURE_NONE) {
        number = kf_enum_encode<s32>(texture_id) + 1;
        (& name[5])[0] = (number) / 100 + '0';
        (& name[5])[1] = ((number) % 100) / 10 + '0';
        (& name[5])[2] = ((number) % 100) % 10 + '0';
        destination = game_graphics_runtime.display_state.primitive_buffer->cursor;
        if (cd_file_load_into((void *)destination, name) != KF_RESOURCE_LOADED) {
            return KF_RESOURCE_LOAD_FAILED;
        }
        tim_upload_images(destination);
    }
    return KF_RESOURCE_LOADED;
}

KfMenuResult menu_list_confirm(
    const KfMenuList *list, KfMenuConfirmKind confirmation, KfMenuPreviewMode preview,
    KfObjectId id, KfItemStockBank bank, KfTradeMode trade)
{
    return menu_list_confirm_impl(list, confirmation, preview, static_cast<s32>(id), bank, trade);
}

KfMenuResult menu_list_confirm(
    const KfMenuList *list, KfMenuConfirmKind confirmation, KfMenuPreviewMode preview,
    KfEffectKind id, KfItemStockBank bank, KfTradeMode trade)
{
    return menu_list_confirm_impl(list, confirmation, preview, static_cast<s32>(id), bank, trade);
}
