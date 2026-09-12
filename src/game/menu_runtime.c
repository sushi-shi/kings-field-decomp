#include <kf/null.h>
#include <kf/bool.h>
#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>
#include <psyq/libc.h>
#include <kf/game_graphics.h>

/* Menu presentation and packet state share this contiguous WIP owner. */
RODATA(0x80012350, 0xa)

DATA(0x80057b6c, 0x4)
KfMenuModelAllocation menu_item_model_allocation_pending = KF_MENU_MODEL_RELEASED;

DATA(0x80057b70, 0x8)
SVECTOR menu_item_preview_rotation = {0, 0, 0, 0};

/* Draw entry points set this from the active primitive buffer before use. */
DATA(0x80057e88, 0x4)
static POLY_FT4 *current_poly_ft4;

enum {
    MENU_STATUS_BACKDROP_LEFT_X = 6,
    MENU_STATUS_BACKDROP_RIGHT_X = MENU_STATUS_BACKDROP_LEFT_X + MENU_BACKDROP_COLUMN_STEP
};

/* Draw one reflected tile of the status-panel backdrop. */
static inline void menu_status_draw_backdrop_quad(
    s32 x, s32 y, KfBool32 flip_x, KfBool32 flip_y, const u16 *texture_page)
{
    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = *texture_page;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        x,
        y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    /* UV dimensions wrap at eight bits; screen dimensions remain halfwords. */
    setUVWH(current_poly_ft4,
        flip_x
            ? menu_assets.window_backdrop.u + (u8)menu_assets.window_backdrop.width
            : menu_assets.window_backdrop.u,
        flip_y
            ? menu_assets.window_backdrop.v + (u8)menu_assets.window_backdrop.height
            : menu_assets.window_backdrop.v,
        flip_x ? -(u8)menu_assets.window_backdrop.width : (u8)menu_assets.window_backdrop.width,
        flip_y ? -(u8)menu_assets.window_backdrop.height : (u8)menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);
}

/*
 * Status panel top-level loop reached from menu_root: each frame it draws the
 * status detail page, tiles the four mirrored window-sprite quads that frame
 * it (a 2x2 grid whose texel corners flip per quadrant), and the shared
 * backdrop.  The first three frames just settle (debouncing the pad on the
 * third); from then on any button press plays the cancel cue and returns.
 */
ADDRESS(0x8002430c, 0x69c)
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

/*
 * Drop-item panel dispatched by the hub menu (slot 4).  Builds a scrollable
 * list of every held item, subtracting one copy of any item currently worn in
 * one of the seven equipment slots so the equipped instance cannot be
 * discarded, runs the windowed cursor, and on confirm removes one of the
 * chosen item from the owned-item block.
 */
ADDRESS(0x800249a8, 0x4bc)
void menu_drop_item(void)
{
    KfMenuList ctx;
    s16 labels[KF_ITEM_COUNT][MENU_GLYPHS_PER_ROW];
    u8 counts[KF_ITEM_COUNT];
    KfObjectId codes[KF_ITEM_COUNT];
    u8 *inv;
    s32 found;
    s32 code;
    s32 j;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_PENDING);

    while (PadRead(1) != 0)
        ;
    menu_list_init(&ctx, KF_MENU_WINDOW_ROOT, KF_ENUM_ENCODE(s32, KF_ROOT_CHOICE_DROP_ITEM));

    found = 0;
    code = 0;
    inv = item_stock[KF_ENUM_ENCODE(u8, KF_ITEM_STOCK_PLAYER)];
    for (; code < KF_ITEM_COUNT; code++) {
        if (inv[code] != 0) {
            counts[found] = inv[code];
            if (code == KF_ENUM_ENCODE(u8, player_state.equipped_weapon_id) ||
                code == KF_ENUM_ENCODE(u8, player_state.equipped_head_armor_id) ||
                code == KF_ENUM_ENCODE(u8, player_state.equipped_body_armor_id) ||
                code == KF_ENUM_ENCODE(u8, player_state.equipped_shield_id) ||
                code == KF_ENUM_ENCODE(u8, player_state.equipped_arm_armor_id) ||
                code == KF_ENUM_ENCODE(u8, player_state.equipped_leg_armor_id) ||
                code == KF_ENUM_ENCODE(u8, player_state.equipped_accessory_id))
                counts[found]--;
            if (counts[found] != 0) {
                for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                    labels[found][j] = item_name_rows[code].codes[j];
                codes[found] = KF_ENUM_DECODE(KfObjectId, code);
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
        if (menu_load_item_model(codes[ctx.selected_index]) != KF_RESOURCE_LOADED)
            return;
        menu_item_model_preview(codes[ctx.selected_index]);
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            selection = KF_ENUM_ENCODE(s32, menu_list_interact(&ctx, KF_MENU_CONFIRM_DROP,
                    KF_MENU_PREVIEW_ITEM_MODEL, codes[ctx.selected_index], KF_ITEM_STOCK_PLAYER, KF_TRADE_BUY));
            if (selection == KF_ENUM_ENCODE(s32, KF_MENU_RESULT_CANCELLED))
                selection = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_PENDING);
            else
                selection = KF_ENUM_ENCODE(u8, codes[ctx.selected_index]);
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (selection != KF_ENUM_ENCODE(s32, KF_MENU_RESULT_PENDING)) {
            while (PadRead(1) != 0)
                ;
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_CANCELLED);
            }
        } else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (ctx.selected_index != 0) {
                ctx.selected_index--;
                if (ctx.cursor_row == 0)
                    ctx.scroll_offset--;
                else
                    ctx.cursor_row--;
            } else {
                ctx.selected_index = ctx.entry_count - 1;
                if (ctx.entry_count < ctx.visible_rows) {
                    ctx.scroll_offset = 0;
                    ctx.cursor_row = ctx.entry_count - 1;
                } else {
                    ctx.scroll_offset = ctx.entry_count - ctx.visible_rows;
                    ctx.cursor_row = ctx.visible_rows - 1;
                }
            }
            if (menu_load_item_model(codes[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return;
        } else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (ctx.selected_index < ctx.entry_count - 1) {
                ctx.selected_index++;
                if (ctx.cursor_row == ctx.visible_rows - 1)
                    ctx.scroll_offset++;
                else
                    ctx.cursor_row++;
            } else {
                ctx.selected_index = 0;
                ctx.scroll_offset = 0;
                ctx.cursor_row = 0;
            }
            if (menu_load_item_model(codes[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_CANCELLED);
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_item_model_preview(codes[ctx.selected_index]);
        menu_list_render(&ctx);
    }

    menu_release_item_model();
    if (selection != KF_ENUM_ENCODE(s32, KF_MENU_RESULT_CANCELLED))
        inv[selection]--;
}

/*
 * Three-row hub dispatched by menu_root. The load action maps result 0 to -3;
 * accepting the second action loads texture 0x3e6, stops music, and redraws
 * forever. Cancellation of a sub-action keeps this hub open.
 */
ADDRESS(0x80024e64, 0x260)
KfMenuResult menu_save_load_hub(void)
{
    KfSaveHeader header;
    KfSavePayload payload;
    s32 cursor = KF_ENUM_ENCODE(s32, KF_MENU_SYSTEM_ACTION_LOAD);
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    KfMenuResult result = KF_MENU_RESULT_PENDING;
    KfMenuSystemAction action = KF_MENU_SYSTEM_ACTION_NONE;

    save_payload_buffer = &payload;
    save_header_buffer = &header;

    for (;;) {
        if (action != KF_MENU_SYSTEM_ACTION_NONE || KF_ENUM_ENCODE(s32, result) == KF_ENUM_ENCODE(s32, action)) {
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
                menu_load_item_texture(MENU_TEXTURE_POWER_OFF);
                audio_stop_sequence_fade();
                for (;;) {
                    menu_frame_begin();
                    menu_add_frame_quad();
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
        if ((input & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 0)
                cursor--;
            else
                cursor = KF_MENU_SYSTEM_RETURN_ROW;
        } else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_MENU_SYSTEM_RETURN_ROW)
                cursor++;
            else
                cursor = KF_ENUM_ENCODE(s32, KF_MENU_SYSTEM_ACTION_LOAD);
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (cursor == KF_MENU_SYSTEM_RETURN_ROW) {
                result = KF_MENU_RESULT_CANCELLED;
            } else {
                action = KF_ENUM_DECODE(KfMenuSystemAction, cursor);
            }
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_CANCELLED;
        }

        menu_frame_begin();
        menu_draw_window(KF_MENU_WINDOW_SYSTEM, KF_MENU_SYSTEM_ROW_COUNT, cursor, confirm);
        menu_present_frame();
    }
    return result;
}

/*
 * Save panel dispatched by the save-confirmation screen.  Reads the catalogue
 * and runs a five-row cursor: three data slots, a card-format row (3), and an
 * exit row (4).  Confirm on a data slot opens the dialog, plays the save
 * effect, holds a three-frame animation, and writes the slot.  The format row
 * first purges any temporary file, then reformats the card and clears the
 * catalogue.  Returns the slot dialog result, or -1 on cancel.
 */
ADDRESS(0x800250c4, 0x468)
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

    status = KF_ENUM_ENCODE(s32, save_system_read_catalog(summaries));
    if (status != KF_ENUM_ENCODE(s32, KF_SAVE_RESULT_OK) && status != KF_ENUM_ENCODE(s32, KF_SAVE_RESULT_NO_SPACE)) {
        menu_play_input_sound(MENU_SOUND_CURSOR);
        while (PadRead(1) == 0) {
            menu_frame_begin();
            menu_add_frame_quad();
            menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
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
            menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
            menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }

        if (confirm == KF_MENU_CONFIRM_REQUESTED && cursor != KF_MENU_SAVE_RETURN_ROW) {
            if (cursor == KF_MENU_SAVE_FORMAT_ROW) {
                status = KF_ENUM_ENCODE(s32, save_file_cleanup_temporary());
                if (status == KF_ENUM_ENCODE(s32, KF_SAVE_CLEANUP_TEMP_OPENED)) {
                    menu_load_item_texture(MENU_TEXTURE_CONFIRM_CARD_FORMAT);
                    while (PadRead(1) == 0) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
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
                    menu_load_item_texture(MENU_TEXTURE_SAVING_DATA);
                    for (i = 0; i < 3; i++) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
                        menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
                        menu_present_frame();
                    }
                    status = KF_ENUM_ENCODE(s32, save_system_write_slot(KF_ENUM_DECODE(KfSaveSlotArgument, cursor + KF_ENUM_ENCODE(s16, KF_SAVE_SLOT_FIRST))));
                } else if (cursor == KF_MENU_SAVE_FORMAT_ROW) {
                    menu_load_item_texture(MENU_TEXTURE_FORMATTING_CARD);
                    for (i = 0; i < 3; i++) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
                        menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
                        menu_present_frame();
                    }
                    status = KF_ENUM_ENCODE(s32, memory_card_check_or_format(KF_CARD_FORMAT_CONFIRMED));
                    memset(summaries, 0, sizeof(summaries));
                }

                if (status != KF_ENUM_ENCODE(s32, KF_SAVE_RESULT_OK)) {
                    while (PadRead(1) == 0) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
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
        if ((input & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 0)
                cursor--;
            else
                cursor = KF_MENU_SAVE_RETURN_ROW;
        } else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_MENU_SAVE_RETURN_ROW)
                cursor++;
            else
                cursor = 0;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (cursor == KF_MENU_SAVE_RETURN_ROW)
                result = KF_MENU_RESULT_CANCELLED;
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_CANCELLED;
        }

        menu_frame_begin();
        menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
        menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
        menu_present_frame();
    }
    return result;
}

/*
 * Load panel: reads the memory-card catalogue into three slot summaries and
 * runs a four-row cursor (three data slots plus an exit row).  Confirm on a
 * populated slot opens the confirmation dialog, plays the load effect, holds a
 * three-frame animation, and reads the slot; a failed read shows the error
 * frame. Returns the dialog result after a successful read, or -1 on cancel
 * or catalogue failure; a read failure keeps the panel open.
 */
ADDRESS(0x8002552c, 0x370)
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
            menu_add_frame_quad();
            menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
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
            menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
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
                menu_load_item_texture(MENU_TEXTURE_LOADING_DATA);
                for (i = 0; i < 3; i++) {
                    menu_frame_begin();
                    menu_add_frame_quad();
                    menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
                    menu_draw_window(KF_MENU_WINDOW_LOAD, KF_MENU_LOAD_ROW_COUNT, cursor, confirm);
                    menu_present_frame();
                }
                if (save_system_read_slot(KF_ENUM_DECODE(KfSaveSlotArgument, cursor + KF_ENUM_ENCODE(s16, KF_SAVE_SLOT_FIRST))) != KF_SAVE_RESULT_OK) {
                    while (PadRead(1) == 0) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
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
        if ((input & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 0)
                cursor--;
            else
                cursor = KF_MENU_LOAD_RETURN_ROW;
        } else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_MENU_LOAD_RETURN_ROW)
                cursor++;
            else
                cursor = 0;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
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
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_CANCELLED;
        }

        menu_frame_begin();
        menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
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

/* The two labels are private by-value copies; only their Y positions advance. */
void menu_config_panel_draw(
    MenuGlyphString option_a, MenuGlyphString option_b, KfPlayerOption *values);

/*
 * Four editable config rows and an exit row. Both exit inputs publish the
 * working values after the final redraw/release wait; a changed music flag
 * starts or stops the current map sequence.
 */
ADDRESS(0x8002589c, 0x504)
void menu_config_panel(void)
{
    KfPlayerOption states[KF_MENU_CONFIG_SETTING_COUNT];
    MenuGlyphString option_a;
    MenuGlyphString option_b;
    s32 row = 0;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    u32 pad = 0;
    u32 prev;
    KfMenuResult phase = KF_MENU_RESULT_PENDING;
    KfPlayerOption music_orig;

    while (PadRead(1) != 0) {
    }

    option_a.position.x = CONFIG_OPTION_ON_X;
    option_a.position.y = CONFIG_OPTION_FIRST_Y;
    option_a.glyphs.codes[0] = 0xf9;
    option_a.glyphs.codes[1] = 0xfa;
    option_a.glyphs.codes[2] = MENU_TEXT_END;
    option_b.position.x = CONFIG_OPTION_OFF_X;
    option_b.position.y = CONFIG_OPTION_FIRST_Y;
    option_b.glyphs.codes[0] = 0xf9;
    option_b.glyphs.codes[1] = 0xfb;
    option_b.glyphs.codes[2] = 0xfb;
    option_b.glyphs.codes[3] = MENU_TEXT_END;
    states[KF_MENU_CONFIG_EFFECTS_ROW] = player_state.audio_effects_enabled;
    states[KF_MENU_CONFIG_MUSIC_ROW] = player_state.audio_music_enabled;
    states[KF_MENU_CONFIG_GAUGES_ROW] = player_state.hud_gauges_enabled;
    states[KF_MENU_CONFIG_COMPASS_ROW] = player_state.compass_enabled;
    music_orig = states[KF_MENU_CONFIG_MUSIC_ROW];

    menu_frame_begin();
    menu_config_panel_draw(option_a, option_b, states);
    menu_draw_window(KF_MENU_WINDOW_CONFIG, KF_MENU_CONFIG_ROW_COUNT, row, confirm);
    menu_present_frame();
    do {
        if (confirm == KF_MENU_CONFIRM_REQUESTED || phase == KF_MENU_RESULT_CANCELLED) {
            menu_frame_begin();
            menu_config_panel_draw(option_a, option_b, states);
            menu_draw_window(KF_MENU_WINDOW_CONFIG, KF_MENU_CONFIG_ROW_COUNT, row, confirm);
            menu_present_frame();
            while (PadRead(1) != 0) {
            }
        }
        if (phase != KF_MENU_RESULT_PENDING) {
            break;
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        menu_frame_begin();
        prev = pad;
        pad = PadRead(1);
        if ((pad & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (row != 0) {
                row--;
            } else {
                row = KF_MENU_CONFIG_RETURN_ROW;
            }
        } else if ((pad & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (row != KF_MENU_CONFIG_RETURN_ROW) {
                row++;
            } else {
                row = 0;
            }
        } else if (((pad & PADLright) != 0 && (prev & PADLright) == 0) ||
                   ((pad & PADLleft) != 0 && (prev & PADLleft) == 0)) {
            if (row != KF_MENU_CONFIG_RETURN_ROW) {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                states[row] = KF_ENUM_DECODE(
                    KfPlayerOption, states[row] == KF_PLAYER_OPTION_OFF);
            }
        } else if ((pad & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            if (row == KF_MENU_CONFIG_RETURN_ROW) {
                confirm = KF_MENU_CONFIRM_REQUESTED;
                phase = KF_MENU_RESULT_CANCELLED;
            } else {
                states[row] = KF_ENUM_DECODE(KfPlayerOption, states[row] == KF_PLAYER_OPTION_OFF);
            }
        } else if ((pad & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            phase = KF_MENU_RESULT_CANCELLED;
        }
        menu_config_panel_draw(option_a, option_b, states);
        menu_draw_window(KF_MENU_WINDOW_CONFIG, KF_MENU_CONFIG_ROW_COUNT, row, confirm);
        menu_present_frame();
    } while (1);

    player_state.audio_effects_enabled = states[KF_MENU_CONFIG_EFFECTS_ROW];
    player_state.audio_music_enabled = states[KF_MENU_CONFIG_MUSIC_ROW];
    player_state.hud_gauges_enabled = states[KF_MENU_CONFIG_GAUGES_ROW];
    player_state.compass_enabled = states[KF_MENU_CONFIG_COMPASS_ROW];
    if (player_state.audio_music_enabled != music_orig) {
        if (player_state.audio_music_enabled == KF_PLAYER_OPTION_OFF) {
            audio_stop_sequence_fade();
        } else {
            audio_play_current_map_sequence();
        }
    }
}

/*
 * Draw four config rows.  Each row places two option boxes at the row's left
 * and right anchors -- highlighting the one whose per-row state equals 1 -- and
 * a label under each, then steps both anchors down 22 pixels.  Finally links
 * the four double-buffered map-viewer frame quads for the current buffer at
 * ordering-table slot 3000.
 */
ADDRESS(0x80025da0, 0x198)
void menu_config_panel_draw(
    MenuGlyphString option_a, MenuGlyphString option_b, KfPlayerOption *values)
{
    s32 i;
    KfPlayerOption *states;
    const MenuSpriteDef *box_b;

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    states = values;
    for (i = 0; i < KF_MENU_CONFIG_SETTING_COUNT; i++) {
        if (*states == KF_PLAYER_OPTION_ON) {
            menu_blit_sprite_translucent(&menu_assets.option_highlight, &option_a.position);
            box_b = &menu_assets.option_background;
        } else {
            menu_blit_sprite_translucent(&menu_assets.option_background, &option_a.position);
            box_b = &menu_assets.option_highlight;
        }
        menu_blit_sprite_translucent(box_b, &option_b.position);
        menu_draw_string(
            &menu_assets.glyph_atlas,
            &option_a);
        menu_draw_string(
            &menu_assets.glyph_atlas,
            &option_b);
        states++;
        option_a.position.y += CONFIG_OPTION_ROW_STEP;
        option_b.position.y += CONFIG_OPTION_ROW_STEP;
    }
    AddPrim(&game_graphics_runtime.display_state.ordering_table[MENU_BACKGROUND_OT_DEPTH],
            &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][3]);
    AddPrim(&game_graphics_runtime.display_state.ordering_table[MENU_BACKGROUND_OT_DEPTH],
            &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][2]);
    AddPrim(&game_graphics_runtime.display_state.ordering_table[MENU_BACKGROUND_OT_DEPTH],
            &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][1]);
    AddPrim(&game_graphics_runtime.display_state.ordering_table[MENU_BACKGROUND_OT_DEPTH],
            &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][0]);
}

enum {
    STATS_HEADER_ROW_STEP = 23
};

/*
 * Draw the main menu's player statistics: experience, level, displayed class,
 * floor, HP/MP pairs, status-effect icons and gold. The class title occupies
 * four atlas cells selected by the base physical-power and magic tiers.
 */
ADDRESS(0x80025f38, 0x5a0)
void menu_draw_stats_header(void)
{
    MenuGlyphString gs;
    s32 glyph_index;
    s32 row_step = STATS_HEADER_ROW_STEP;

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    gs.position.x = 0xb5;
    gs.position.y = 0x24;
    gs.glyphs.codes[0] = 0x82;
    gs.glyphs.codes[1] = 0x83;
    gs.glyphs.codes[2] = 0x84;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x2b;
    gs.glyphs.codes[1] = MENU_TEXT_DAKUTEN | 0x1c;
    gs.glyphs.codes[2] = 0x2a;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 7;
    gs.glyphs.codes[1] = 0x28;
    gs.glyphs.codes[2] = 0xc;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0xcc;
    gs.glyphs.codes[1] = 0xcd;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0xf0;
    gs.glyphs.codes[1] = 0xf2;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0xf1;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x85;
    gs.glyphs.codes[1] = 0x86;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = MENU_TEXT_DAKUTEN | 0x9;
    gs.glyphs.codes[1] = 0x2d;
    gs.glyphs.codes[2] = 0x2a;
    gs.glyphs.codes[3] = MENU_TEXT_DAKUTEN | 0x13;
    gs.glyphs.codes[4] = MENU_TEXT_END;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = 0xfb;
    gs.position.y = 0x24;
    menu_format_number(player_state.experience, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.y += row_step;
    menu_format_number(player_state.progress_state.level, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0xed;
    gs.position.y += row_step;
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
    gs.glyphs.codes[0] = glyph_index + MENU_CLASS_FIRST_GLYPH;
    gs.glyphs.codes[1] = glyph_index + MENU_CLASS_FIRST_GLYPH + 1;
    gs.glyphs.codes[2] = glyph_index + MENU_CLASS_FIRST_GLYPH + 2;
    gs.glyphs.codes[3] = glyph_index + MENU_CLASS_FIRST_GLYPH + 3;
    gs.glyphs.codes[4] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = 0xfb;
    gs.position.y += row_step;
    menu_format_number(KF_ENUM_ENCODE(u8, player_state.progress_state.current_floor), MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0xe6;
    gs.position.y += row_step;
    menu_format_number(player_state.vitals.current_hp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.glyphs.codes[0] = MENU_NUMBER_SLASH;
    gs.glyphs.codes[1] = MENU_TEXT_END;
    gs.position.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.x += MENU_NUMBER_ADVANCE;
    menu_format_number(player_state.vitals.maximum_hp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0xe6;
    gs.position.y += row_step;
    menu_format_number(player_state.vitals.current_mp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.glyphs.codes[0] = MENU_NUMBER_SLASH;
    gs.glyphs.codes[1] = MENU_TEXT_END;
    gs.position.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.x += MENU_NUMBER_ADVANCE;
    menu_format_number(player_state.vitals.maximum_mp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0xdf;
    gs.glyphs.codes[0] = MENU_TEXT_BLANK;
    gs.glyphs.codes[1] = MENU_TEXT_BLANK;
    gs.glyphs.codes[2] = MENU_TEXT_BLANK;
    gs.glyphs.codes[3] = MENU_TEXT_BLANK;
    gs.glyphs.codes[4] = MENU_TEXT_BLANK;
    gs.glyphs.codes[5] = MENU_TEXT_END;
    gs.position.y += row_step;
    if (player_state.status_effect_flags == KF_PLAYER_STATUS_NONE) {
        gs.glyphs.codes[3] = 0xc5;
        gs.glyphs.codes[4] = 0xc6;
    }
    glyph_index = 4;
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_SLOWED) != KF_PLAYER_STATUS_NONE) {
        gs.glyphs.codes[4] = 0xc9;
        glyph_index = 3;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_POISON) != KF_PLAYER_STATUS_NONE) {
        gs.glyphs.codes[glyph_index] = 0x88;
        glyph_index--;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_DARKNESS) != KF_PLAYER_STATUS_NONE) {
        gs.glyphs.codes[glyph_index] = 199;
        glyph_index--;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_CURSE) != KF_PLAYER_STATUS_NONE) {
        gs.glyphs.codes[glyph_index] = 200;
    }
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = 0xfb;
    gs.position.y += row_step;
    menu_format_number(player_state.gold, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
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

/*
 * Draw the full status page: the left column of vitals (experience, level,
 * displayed class, HP/MP pairs, status icons, gold, the derived
 * attack and defence ratings) and the right column of per-element attack and
 * defence component values, all through the shared glyph workspace.
 */
ADDRESS(0x800264d8, 0xcb4)
void menu_draw_status_details(void)
{
    MenuGlyphString gs;
    s32 glyph_index;
    s32 rating;
    s32 summary_y_origin;

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    summary_y_origin = 0x23;
    gs.position.x = 0x15;
    gs.position.y = summary_y_origin;
    gs.glyphs.codes[0] = 0x82;
    gs.glyphs.codes[1] = 0x83;
    gs.glyphs.codes[2] = 0x84;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x2b;
    gs.glyphs.codes[1] = MENU_TEXT_DAKUTEN | 0x1c;
    gs.glyphs.codes[2] = 0x2a;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 7;
    gs.glyphs.codes[1] = 0x28;
    gs.glyphs.codes[2] = 0xc;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0xf0;
    gs.glyphs.codes[1] = 0xf2;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0xf1;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x85;
    gs.glyphs.codes[1] = 0x86;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = MENU_TEXT_DAKUTEN | 0x9;
    gs.glyphs.codes[1] = 0x2d;
    gs.glyphs.codes[2] = 0x2a;
    gs.glyphs.codes[3] = MENU_TEXT_DAKUTEN | 0x13;
    gs.glyphs.codes[4] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x8c;
    gs.glyphs.codes[1] = 0x8b;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x78;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0xce;
    gs.glyphs.codes[1] = 0xcf;
    gs.glyphs.codes[2] = 0x89;
    gs.glyphs.codes[3] = 0x8a;
    gs.glyphs.codes[4] = 0x8b;
    gs.glyphs.codes[5] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[2] = 0x7a;
    gs.glyphs.codes[3] = 0xd0;
    gs.glyphs.codes[5] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = 0x5b;
    gs.position.y = summary_y_origin;
    menu_format_number(player_state.experience, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.progress_state.level, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0x4d;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
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
        gs.glyphs.codes[0] = glyph_index + MENU_CLASS_FIRST_GLYPH;
        gs.glyphs.codes[1] = glyph_index + MENU_CLASS_FIRST_GLYPH + 1;
        gs.glyphs.codes[2] = glyph_index + MENU_CLASS_FIRST_GLYPH + 2;
        gs.glyphs.codes[3] = glyph_index + MENU_CLASS_FIRST_GLYPH + 3;
        gs.glyphs.codes[4] = MENU_TEXT_END;
        menu_draw_string(&menu_assets.glyph_atlas, &gs);

        gs.position.x = 0x46;
        gs.position.y += summary_row_step;
        menu_format_number(player_state.vitals.current_hp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &gs);
        gs.glyphs.codes[0] = MENU_NUMBER_SLASH;
        gs.glyphs.codes[1] = MENU_TEXT_END;
        gs.position.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
        menu_draw_number(&menu_assets.number_atlas, &gs);
        gs.position.x += MENU_NUMBER_ADVANCE;
        menu_format_number(player_state.vitals.maximum_hp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &gs);

        gs.position.x = 0x46;
        gs.position.y += summary_row_step;
        menu_format_number(player_state.vitals.current_mp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &gs);
        gs.glyphs.codes[0] = MENU_NUMBER_SLASH;
        gs.glyphs.codes[1] = MENU_TEXT_END;
        gs.position.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
        menu_draw_number(&menu_assets.number_atlas, &gs);
        gs.position.x += MENU_NUMBER_ADVANCE;
        menu_format_number(player_state.vitals.maximum_mp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &gs);

        gs.position.x = 0x3f;
        gs.glyphs.codes[0] = MENU_TEXT_BLANK;
        gs.glyphs.codes[1] = MENU_TEXT_BLANK;
        gs.glyphs.codes[2] = MENU_TEXT_BLANK;
        gs.glyphs.codes[3] = MENU_TEXT_BLANK;
        gs.glyphs.codes[4] = MENU_TEXT_BLANK;
        gs.glyphs.codes[5] = MENU_TEXT_END;
        gs.position.y += summary_row_step;
        if (player_state.status_effect_flags == KF_PLAYER_STATUS_NONE) {
            gs.glyphs.codes[3] = 0xc5;
            gs.glyphs.codes[4] = 0xc6;
        }
        glyph_index = 4;
        if ((player_state.status_effect_flags & KF_PLAYER_STATUS_SLOWED) != KF_PLAYER_STATUS_NONE) {
            gs.glyphs.codes[4] = 0xc9;
            glyph_index = 3;
        }
        if ((player_state.status_effect_flags & KF_PLAYER_STATUS_POISON) != KF_PLAYER_STATUS_NONE) {
            gs.glyphs.codes[glyph_index] = 0x88;
            glyph_index--;
        }
        if ((player_state.status_effect_flags & KF_PLAYER_STATUS_DARKNESS) != KF_PLAYER_STATUS_NONE) {
            gs.glyphs.codes[glyph_index] = 199;
            glyph_index--;
        }
        if ((player_state.status_effect_flags & KF_PLAYER_STATUS_CURSE) != KF_PLAYER_STATUS_NONE) {
            gs.glyphs.codes[glyph_index] = 200;
        }
        menu_draw_string(&menu_assets.glyph_atlas, &gs);

        gs.position.x = 0x5b;
        gs.position.y += summary_row_step;
        menu_format_number(player_state.gold, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &gs);
        gs.position.y += summary_row_step;
        menu_format_number(player_state.physical_power, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &gs);
        gs.position.y += summary_row_step;
        menu_format_number(player_state.magic, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &gs);
        rating = (((u32)player_state.cutting_attack + player_state.striking_attack +
                   player_state.piercing_attack) * STATUS_PHYSICAL_ATTACK_MULTIPLIER
                   >> STATUS_PHYSICAL_ATTACK_DOWNSHIFT) +
                 (player_state.holy_attack + player_state.fire_attack)
                 * STATUS_ELEMENTAL_ATTACK_MULTIPLIER;
        rating = rating * STATUS_ATTACK_SCALE_NUMERATOR
            / STATUS_ATTACK_SCALE_DENOMINATOR;
        gs.position.y += summary_row_step;
        menu_format_number(rating, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &gs);
        rating = player_state.cutting_defense + player_state.striking_defense +
                 player_state.piercing_defense + player_state.poison_resistance / STATUS_POISON_RESISTANCE_DIVISOR +
                 player_state.magic_defense + player_state.fire_defense;
        rating = rating * STATUS_DEFENSE_SCALE_NUMERATOR / STATUS_DEFENSE_SCALE_DENOMINATOR;
        gs.position.y += summary_row_step;
        menu_format_number(rating, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &gs);
    }
    gs.position.x = 0xb5;
    gs.position.y = 0x1e;
    gs.glyphs.codes[0] = 0x89;
    gs.glyphs.codes[1] = 0x8a;
    gs.glyphs.codes[2] = 0x8b;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = MENU_TEXT_BLANK;
    gs.glyphs.codes[1] = 0xd1;
    gs.glyphs.codes[2] = 0x6a;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xd2;
    gs.glyphs.codes[2] = 0x51;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xd3;
    gs.glyphs.codes[2] = 0x4c;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xbf;
    gs.glyphs.codes[2] = 0x58;
    gs.glyphs.codes[3] = 0x78;
    gs.glyphs.codes[4] = 0x79;
    gs.glyphs.codes[5] = MENU_TEXT_END;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xd4;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x7a;
    gs.glyphs.codes[1] = 0xd0;
    gs.glyphs.codes[2] = 0x8b;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = MENU_TEXT_BLANK;
    gs.glyphs.codes[1] = 0xd1;
    gs.glyphs.codes[2] = 0x6a;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xd2;
    gs.glyphs.codes[2] = 0x51;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xd3;
    gs.glyphs.codes[2] = 0x4c;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0x88;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0x78;
    gs.glyphs.codes[2] = 0x58;
    gs.glyphs.codes[3] = 0x78;
    gs.glyphs.codes[4] = 0x79;
    gs.glyphs.codes[5] = MENU_TEXT_END;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xd4;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = 0xfb;
    gs.position.y = 0x2c;
    menu_format_number(player_state.cutting_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.striking_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.piercing_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.holy_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.fire_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += 0x1e;
    menu_format_number(player_state.cutting_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.striking_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.piercing_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.poison_resistance, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.magic_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.fire_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
}

enum {
    EQUIPMENT_NAME_FIRST_Y = 40,
    EQUIPMENT_NAME_ROW_STEP = 20
};

/*
 * Draw the equipment column of the status panel: the equipped weapon and the
 * selected magic (from their own name tables) followed by the six worn armour
 * / accessory slots. Each category retains its row even when empty. Every
 * displayed name is copied whole from its table into the shared glyph workspace.
 */
ADDRESS(0x8002718c, 0x838)
void menu_draw_name_list(void)
{
    MenuGlyphString gs;

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    gs.position.x = MENU_ITEM_NAME_X;
    gs.position.y = EQUIPMENT_NAME_FIRST_Y;
    if (player_state.equipped_weapon_id != KF_OBJECT_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_weapon_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.selected_magic_id != KF_MAGIC_NONE) {
        gs.glyphs =
            magic_name_rows[KF_ENUM_ENCODE(u8, player_state.selected_magic_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_shield_id != KF_OBJECT_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_shield_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_head_armor_id != KF_OBJECT_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_head_armor_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_body_armor_id != KF_OBJECT_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_body_armor_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_arm_armor_id != KF_OBJECT_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_arm_armor_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_leg_armor_id != KF_OBJECT_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_leg_armor_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_accessory_id != KF_OBJECT_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_accessory_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
}

enum {
    MENU_INVENTORY_QUANTITY_LABEL_X = 230,
    MENU_INVENTORY_QUANTITY_VALUE_X = 279
};

/*
 * Draw a spinning 3D model preview of the inventory item plus its name and held
 * quantity.  Builds the GTE rotation from the shared spin angle and a fixed
 * local light matrix, projects the TMD, then writes the name glyph string, a
 * fixed label pair and the two-digit held count into the glyph workspace.
 * `item_id` 0xff selects the empty slot and draws nothing.
 */
ADDRESS(0x800279c4, 0x1b8)
void menu_item_model_preview(KF_ENUM_PARAM(KfObjectId, s32) item_id)
{
    MenuGlyphString gs;
    MATRIX rot;
    MATRIX lsrc;
    MATRIX lres;
    const MenuGlyphRow *rows;
    const MenuGlyphRow *name;
    s32 i;

    if (item_id != KF_OBJECT_NONE) {
        rot.t[0] = MENU_ITEM_PREVIEW_TRANSLATION_X;
        rot.t[1] = MENU_ITEM_PREVIEW_TRANSLATION_Y;
        rot.t[2] = MENU_ITEM_PREVIEW_TRANSLATION_Z;
        menu_item_preview_rotation.vy =
            (menu_item_preview_rotation.vy + MENU_ITEM_PREVIEW_YAW_STEP)
            & KF_ANGLE_WRAP_MASK;
        RotMatrix(&menu_item_preview_rotation, &rot);

        lsrc.m[0][0] = -KF_FIXED12_ONE;
        lsrc.m[0][1] = -KF_FIXED12_ONE;
        lsrc.m[0][2] = -KF_FIXED12_ONE;
        lsrc.m[1][0] = -KF_FIXED12_ONE;
        lsrc.m[1][1] = -KF_FIXED12_ONE;
        lsrc.m[1][2] = -KF_FIXED12_ONE;
        lsrc.m[2][0] = 0;
        lsrc.m[2][1] = 0;
        lsrc.m[2][2] = 0;
        MulMatrix0(&lsrc, &rot, &lres);
        SetLightMatrix(&lres);
        SetRotMatrix(&rot);
        SetTransMatrix(&rot);
        menu_render_item_model();

        rows = item_name_rows;
        name = &rows[KF_ENUM_ENCODE(s32, item_id)];
        current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

        gs.position.x = MENU_ITEM_NAME_X;
        gs.position.y = MENU_ITEM_PREVIEW_NAME_Y;
        for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
            gs.glyphs.codes[i] = name->codes[i];
        }
        menu_draw_string(&menu_assets.glyph_atlas, &gs);

        gs.position.x = MENU_INVENTORY_QUANTITY_LABEL_X;
        gs.glyphs.codes[0] = 0xca;
        gs.glyphs.codes[1] = 0xcb;
        gs.glyphs.codes[2] = MENU_TEXT_END;
        gs.position.y += MENU_ITEM_PREVIEW_LINE_HEIGHT;
        menu_draw_string(&menu_assets.glyph_atlas, &gs);

        gs.position.x = MENU_INVENTORY_QUANTITY_VALUE_X;
        menu_format_number(item_stock[KF_ENUM_ENCODE(u8, KF_ITEM_STOCK_PLAYER)][KF_ENUM_ENCODE(s32, item_id)], MENU_ITEM_PREVIEW_QUANTITY_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &gs);
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

/*
 * Draw the selected inventory item's detail: a spinning 3D TMD preview (built
 * from the GTE rotation and a fixed local light matrix), the item name, its
 * price for the active shop column/table, the held quantity, and the player's
 * gold.  `item_id` selects the item and `shop_id` the one-based price column.
 * `price_mode` selects buy prices at zero and sell prices for any nonzero value.
 */
ADDRESS(0x80027b7c, 0x2dc)
void menu_draw_item_detail(KF_ENUM_PARAM(KfObjectId, s32) item_id, KF_ENUM_PARAM(KfItemStockBank, s32) shop_id, KfTradeMode price_mode)
{
    MenuGlyphString gs;
    MATRIX rot;
    MATRIX lsrc;
    MATRIX lres;
    s32 price;
    MenuGlyphRow *rows;
    s16 *name;
    s32 i;

    if (item_id == KF_OBJECT_NONE) {
        return;
    }

    rot.t[0] = MENU_ITEM_PREVIEW_TRANSLATION_X;
    rot.t[1] = MENU_ITEM_PREVIEW_TRANSLATION_Y;
    rot.t[2] = MENU_ITEM_PREVIEW_TRANSLATION_Z;
    menu_item_preview_rotation.vy =
        (menu_item_preview_rotation.vy + MENU_ITEM_PREVIEW_YAW_STEP)
        & KF_ANGLE_WRAP_MASK;
    RotMatrix(&menu_item_preview_rotation, &rot);

    lsrc.m[0][0] = -KF_FIXED12_ONE;
    lsrc.m[0][1] = -KF_FIXED12_ONE;
    lsrc.m[0][2] = -KF_FIXED12_ONE;
    lsrc.m[1][0] = -KF_FIXED12_ONE;
    lsrc.m[1][1] = -KF_FIXED12_ONE;
    lsrc.m[1][2] = -KF_FIXED12_ONE;
    lsrc.m[2][0] = 0;
    lsrc.m[2][1] = 0;
    lsrc.m[2][2] = 0;
    MulMatrix0(&lsrc, &rot, &lres);
    SetLightMatrix(&lres);
    SetRotMatrix(&rot);
    SetTransMatrix(&rot);
    menu_render_item_model();

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    gs.position.x = MENU_ITEM_NAME_X;
    gs.position.y = MENU_ITEM_PREVIEW_NAME_Y;
    rows = item_name_rows;
    name = rows[KF_ENUM_ENCODE(s32, item_id)].codes;
    {
        s16 *glyph = gs.glyphs.codes;

        for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
            *glyph = *name++;
            glyph++;
        }
    }
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = MENU_ITEM_DETAIL_PRICE_X;
    gs.position.y += MENU_ITEM_PREVIEW_LINE_HEIGHT;
    if (price_mode == KF_TRADE_BUY) {
        price = item_buy_prices[KF_ENUM_ENCODE(s32, item_id)]
            [KF_ENUM_ENCODE(s32, shop_id) - KF_ENUM_ENCODE(s32, KF_ITEM_STOCK_FIRST_SHOP)];
    } else {
        price = item_sell_prices[KF_ENUM_ENCODE(s32, item_id)]
            [KF_ENUM_ENCODE(s32, shop_id) - KF_ENUM_ENCODE(s32, KF_ITEM_STOCK_FIRST_SHOP)];
    }
    menu_format_number(price, MENU_ITEM_DETAIL_PRICE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = MENU_ITEM_DETAIL_LABEL_X;
    gs.glyphs.codes[0] = MENU_TEXT_DAKUTEN | 0x9;
    gs.glyphs.codes[1] = 0x2d;
    gs.glyphs.codes[2] = 0x2a;
    gs.glyphs.codes[3] = MENU_TEXT_DAKUTEN | 0x13;
    gs.glyphs.codes[4] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = MENU_ITEM_DETAIL_LABEL_X;
    gs.glyphs.codes[0] = 0xca;
    gs.glyphs.codes[1] = 0xcb;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += MENU_ITEM_PREVIEW_LINE_HEIGHT;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = MENU_ITEM_DETAIL_QUANTITY_X;
    menu_format_number(item_stock[KF_ENUM_ENCODE(u8, KF_ITEM_STOCK_PLAYER)][KF_ENUM_ENCODE(s32, item_id)], MENU_ITEM_PREVIEW_QUANTITY_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    menu_blit_sprite_translucent(
        &menu_assets.row_background,
        &menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD].position);
    menu_draw_string(&menu_assets.glyph_atlas, &menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD]);

    gs.position.x = menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD].position.x + MENU_ITEM_DETAIL_GOLD_X_OFFSET;
    gs.position.y = menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD].position.y;
    menu_format_number(player_state.gold, MENU_ITEM_DETAIL_GOLD_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
}

/*
 * Two shared menu ordering-table primitives: double-buffered POLY_FT4 quads
 * that the surrounding panels prime and these helpers link into the current
 * frame's ordering table at a fixed depth. The packets are loaded from
 * STAT.DAT; both helpers select the active buffer's copy and enqueue it.  Both are shared by the magic, list, save and load panels.
 */

/* Link the shared mid-depth menu quad at ordering-table slot 500. */
ADDRESS(0x80027e58, 0x48)
void menu_add_marker_quad(void)
{
    AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_MARKER_OT_DEPTH,
            &menu_assets.mid_depth_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)]);
}

/* Link the shared front menu quad at ordering-table slot 0. */
ADDRESS(0x80027ea0, 0x44)
void menu_add_frame_quad(void)
{
    AddPrim(game_graphics_runtime.display_state.ordering_table,
            &menu_assets.foreground_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)]);
}

/*
 * Draw save-slot overlays plus optional summary rows. The highlighted slot
 * omits its overlay; negative indices suppress all overlays and indices beyond
 * the three slots draw all overlays.  When `rows` is non-null,
 * up to three slot summaries are drawn, each gated by positive saved current
 * HP; every row prints an icon label plus its numeric fields through the
 * shared glyph-string workspace.  Used by the save/load panels, the save
 * confirmation, and the two-option confirm dialog.
 */
ADDRESS(0x80027ee4, 0x49c)
void menu_draw_dialog_frame(const KfSaveSlotSummary *rows, KfSaveSlotOverlay overlay)
{
    MenuGlyphString gs;
    s32 i;

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    if (overlay == KF_SAVE_OVERLAY_SKIP_FIRST) {
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH,
                &menu_assets.dialog_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT1_QUAD]);
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH,
                &menu_assets.dialog_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT2_QUAD]);
    }
    if (overlay == KF_SAVE_OVERLAY_SKIP_SECOND) {
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH,
                &menu_assets.dialog_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT0_QUAD]);
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH,
                &menu_assets.dialog_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT2_QUAD]);
    }
    if (overlay == KF_SAVE_OVERLAY_SKIP_THIRD) {
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH,
                &menu_assets.dialog_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT0_QUAD]);
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH,
                &menu_assets.dialog_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT1_QUAD]);
    }
    if (overlay >= KF_SAVE_OVERLAY_ALL) {
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH,
                &menu_assets.dialog_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT0_QUAD]);
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH,
                &menu_assets.dialog_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT1_QUAD]);
        AddPrim(game_graphics_runtime.display_state.ordering_table + MENU_CONTENT_OT_DEPTH,
                &menu_assets.dialog_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][MENU_SAVE_SLOT2_QUAD]);
    }

    if (rows == NULL) {
        return;
    }

    for (i = 0; i < KF_SAVE_SLOT_COUNT; i++) {
        gs.position.y = i * MENU_SAVE_SUMMARY_ROW_HEIGHT + 30;
        if ((s32)rows[i].current_hp > 0) {
            gs.position.x = MENU_SAVE_SUMMARY_LABEL_X;
            gs.glyphs.codes[0] = 0x82;
            gs.glyphs.codes[1] = 0x83;
            gs.glyphs.codes[2] = 0x84;
            gs.glyphs.codes[3] = MENU_TEXT_END;
            menu_draw_string(&menu_assets.glyph_atlas, &gs);

            gs.position.x = 251;
            menu_format_number(rows[i].experience, 6, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.position.x = MENU_SAVE_SUMMARY_LABEL_X;
            gs.glyphs.codes[0] = 0xcc;
            gs.glyphs.codes[1] = 0xcd;
            gs.glyphs.codes[2] = MENU_TEXT_END;
            gs.position.y += MENU_SAVE_SUMMARY_LINE_HEIGHT;
            menu_draw_string(&menu_assets.glyph_atlas, &gs);

            gs.position.x = 286;
            menu_format_number(KF_ENUM_ENCODE(u32, rows[i].current_floor), 1, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.position.x = MENU_SAVE_SUMMARY_LABEL_X;
            gs.glyphs.codes[0] = 0xf0;
            gs.glyphs.codes[1] = 242;
            gs.glyphs.codes[2] = MENU_TEXT_END;
            gs.position.y += MENU_SAVE_SUMMARY_LINE_HEIGHT;
            menu_draw_string(&menu_assets.glyph_atlas, &gs);

            gs.position.x = MENU_SAVE_SUMMARY_VALUE_X;
            menu_format_number(rows[i].current_hp, MENU_SAVE_STATUS_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.glyphs.codes[0] = MENU_NUMBER_SLASH;
            gs.glyphs.codes[1] = MENU_TEXT_END;
            gs.position.x += MENU_SAVE_STATUS_DIGITS * MENU_NUMBER_ADVANCE;
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.position.x += MENU_NUMBER_ADVANCE;
            menu_format_number(rows[i].maximum_hp, MENU_SAVE_STATUS_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.position.x = MENU_SAVE_SUMMARY_LABEL_X;
            gs.glyphs.codes[0] = 0xf1;
            gs.glyphs.codes[1] = 242;
            gs.glyphs.codes[2] = MENU_TEXT_END;
            gs.position.y += MENU_SAVE_SUMMARY_LINE_HEIGHT;
            menu_draw_string(&menu_assets.glyph_atlas, &gs);

            gs.position.x = MENU_SAVE_SUMMARY_VALUE_X;
            menu_format_number(rows[i].current_mp, MENU_SAVE_STATUS_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.glyphs.codes[0] = MENU_NUMBER_SLASH;
            gs.glyphs.codes[1] = MENU_TEXT_END;
            gs.position.x += MENU_SAVE_STATUS_DIGITS * MENU_NUMBER_ADVANCE;
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.position.x += MENU_NUMBER_ADVANCE;
            menu_format_number(rows[i].maximum_mp, MENU_SAVE_STATUS_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);
        }
    }
}

#define KF_MENU_LIST_IMPLEMENTATION

enum {
    MENU_LIST_CONFIRM_ACCEPT_Y = 185,
    MENU_LIST_CONFIRM_DECLINE_Y = MENU_LIST_CONFIRM_ACCEPT_Y + MENU_CONFIRM_ROW_STEP
};

/* Confirm the current list entry using a two-option footer. Up/down toggles
 * the footer; confirm accepts its selected option, and cancel declines.
 * The final highlighted frame is presented before waiting for button release.
 */
ADDRESS(0x80028380, 0x354)
KfMenuResult menu_list_interact(
    const KfMenuList *list, KfMenuConfirmKind kind, KfMenuPreviewMode preview_mode,
    s32 item_id, KF_ENUM_PARAM(KfItemStockBank, u32) shop_id, KfTradeMode price_mode)
{
    MenuGlyphString opt0;
    MenuGlyphString opt1;
    KfMenuConfirmChoice selected;
    KF_ENUM_STORAGE(KfMenuConfirmState, u32) highlight;
    u32 pad;
    u32 prev_pad;
    KfMenuResult result;

    selected = KF_MENU_CHOICE_ACCEPT;
    highlight = KF_MENU_CONFIRM_IDLE;
    pad = 0;
    result = KF_MENU_RESULT_PENDING;
    while (PadRead(1) != 0) {
    }

    opt0.position.x = MENU_CONFIRM_TEXT_X;
    opt0.position.y = MENU_LIST_CONFIRM_ACCEPT_Y;
    opt1.position.x = MENU_CONFIRM_TEXT_X;
    opt1.position.y = MENU_LIST_CONFIRM_DECLINE_Y;
    if (kind == KF_MENU_CONFIRM_USE) {
        opt0.glyphs.codes[0] = 0x72;
        opt0.glyphs.codes[1] = 0x42;
        opt0.glyphs.codes[2] = MENU_TEXT_END;
    } else if (kind == KF_MENU_CONFIRM_DROP) {
        opt0.glyphs.codes[0] = 0x75;
        opt0.glyphs.codes[1] = 0x52;
        opt0.glyphs.codes[2] = 0x6a;
        opt0.glyphs.codes[3] = MENU_TEXT_END;
    } else if (kind == KF_MENU_CONFIRM_YES_NO) {
        opt0.glyphs.codes[0] = 0x59;
        opt0.glyphs.codes[1] = 0x41;
        opt0.glyphs.codes[2] = MENU_TEXT_END;
    } else if (kind == KF_MENU_CONFIRM_BUY) {
        opt0.glyphs.codes[0] = 0x74;
        opt0.glyphs.codes[1] = 0x42;
        opt0.glyphs.codes[2] = MENU_TEXT_END;
    } else if (kind == KF_MENU_CONFIRM_SELL) {
        opt0.glyphs.codes[0] = 0x73;
        opt0.glyphs.codes[1] = 0x6a;
        opt0.glyphs.codes[2] = MENU_TEXT_END;
    } else {
        opt0.glyphs.codes[0] = 0x70;
        opt0.glyphs.codes[1] = 0x71;
        opt0.glyphs.codes[2] = MENU_TEXT_END;
    }
    if (kind == KF_MENU_CONFIRM_YES_NO) {
        opt1.glyphs.codes[0] = 0x41;
        opt1.glyphs.codes[1] = 0x41;
        opt1.glyphs.codes[2] = 0x43;
    } else {
        opt1.glyphs.codes[0] = 99;
        opt1.glyphs.codes[1] = 0x61;
        opt1.glyphs.codes[2] = 0x6a;
    }
    opt1.glyphs.codes[3] = MENU_TEXT_END;

    menu_frame_begin();
    if (preview_mode == KF_MENU_PREVIEW_ITEM_MODEL) {
        menu_item_model_preview(KF_ENUM_DECODE(KF_ENUM_PARAM(KfObjectId, s32), item_id));
    } else if (preview_mode == KF_MENU_PREVIEW_ITEM_DETAIL) {
        menu_draw_item_detail(KF_ENUM_DECODE(KF_ENUM_PARAM(KfObjectId, s32), item_id), shop_id, price_mode);
    } else if (preview_mode == KF_MENU_PREVIEW_MAGIC_ICON
            && item_id != KF_ENUM_ENCODE(s32, KF_MAGIC_NONE)) {
        menu_add_marker_quad();
    }
    menu_list_render(list);
    menu_draw_two_option(&opt0, &opt1, selected, highlight);
    menu_present_frame();
    do {
        if (result != KF_MENU_RESULT_PENDING) {
            menu_frame_begin();
            if (preview_mode == KF_MENU_PREVIEW_ITEM_MODEL) {
                menu_item_model_preview(KF_ENUM_DECODE(KF_ENUM_PARAM(KfObjectId, s32), item_id));
            } else if (preview_mode == KF_MENU_PREVIEW_ITEM_DETAIL) {
                menu_draw_item_detail(KF_ENUM_DECODE(KF_ENUM_PARAM(KfObjectId, s32), item_id), shop_id, price_mode);
            } else if (preview_mode == KF_MENU_PREVIEW_MAGIC_ICON
                    && item_id != KF_ENUM_ENCODE(s32, KF_MAGIC_NONE)) {
                menu_add_marker_quad();
            }
            menu_list_render(list);
            menu_draw_two_option(&opt0, &opt1, selected, highlight);
            menu_present_frame();
            while (PadRead(1) != 0) {
            }
            return result;
        }

        highlight = KF_MENU_CONFIRM_IDLE;
        menu_frame_begin();
        prev_pad = pad;
        pad = PadRead(1);
        if (((pad & PADLup) != 0 && (prev_pad & PADLup) == 0) ||
            ((pad & PADLdown) != 0 && (prev_pad & PADLdown) == 0)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (selected == KF_MENU_CHOICE_ACCEPT) {
                selected = KF_MENU_CHOICE_DECLINE;
            } else {
                selected = KF_MENU_CHOICE_ACCEPT;
            }
        } else if ((pad & PADRright) != 0 && (prev_pad & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            highlight = KF_MENU_CONFIRM_REQUESTED;
            result = menu_confirm_result_from_choice(selected);
        } else if ((pad & PADRdown) != 0 && (prev_pad & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_CANCELLED;
        }
        if (preview_mode == KF_MENU_PREVIEW_ITEM_MODEL) {
            menu_item_model_preview(KF_ENUM_DECODE(KF_ENUM_PARAM(KfObjectId, s32), item_id));
        } else if (preview_mode == KF_MENU_PREVIEW_ITEM_DETAIL) {
            menu_draw_item_detail(KF_ENUM_DECODE(KF_ENUM_PARAM(KfObjectId, s32), item_id), shop_id, price_mode);
        } else if (preview_mode == KF_MENU_PREVIEW_MAGIC_ICON
                && item_id != KF_ENUM_ENCODE(s32, KF_MAGIC_NONE)) {
            menu_add_marker_quad();
        }
        menu_list_render(list);
        menu_draw_two_option(&opt0, &opt1, selected, highlight);
        menu_present_frame();
    } while (1);
}

enum {
    MENU_PROMPT_ACCEPT_Y_OFFSET = 44,
    MENU_PROMPT_DECLINE_Y_OFFSET = MENU_PROMPT_ACCEPT_Y_OFFSET + MENU_CONFIRM_ROW_STEP
};

/*
 * Interactive two-option confirm dialog.  Draws the window and the two labels
 * every frame, toggling the selected option on an up/down edge, and returns
 * once the player confirms (result -selected) or cancels (result -1).  Save
 * and load windows forward the caller's save summaries to the frame builder; all others
 * suppress the composite-frame selection (-1).
 */
ADDRESS(0x800286d4, 0x240)
KfMenuResult menu_two_option_prompt(
    KfMenuWindowKind kind, s32 count, s32 highlight_row,
    const KfSaveSlotSummary *summaries)
{
    MenuGlyphString label_a;
    MenuGlyphString label_b;
    KfMenuConfirmChoice selected = KF_MENU_CHOICE_ACCEPT;
    KfMenuConfirmState highlight = KF_MENU_CONFIRM_IDLE;
    KfSaveSlotOverlay overlay = KF_SAVE_OVERLAY_NONE;
    s32 input = 0;
    s32 prev;
    KfMenuResult result = KF_MENU_RESULT_PENDING;

    while (PadRead(1) != 0)
        ;

    if (kind == KF_MENU_WINDOW_SAVE || kind == KF_MENU_WINDOW_LOAD)
        overlay = KF_ENUM_DECODE(KfSaveSlotOverlay, highlight_row);

    label_a.position.x = MENU_CONFIRM_TEXT_X;
    label_a.position.y = count * MENU_CONFIRM_ROW_STEP + MENU_PROMPT_ACCEPT_Y_OFFSET;
    label_a.glyphs.codes[0] = 0x59;
    label_a.glyphs.codes[1] = 0x41;
    label_a.glyphs.codes[2] = MENU_TEXT_END;
    label_b.position.x = MENU_CONFIRM_TEXT_X;
    label_b.position.y = count * MENU_CONFIRM_ROW_STEP + MENU_PROMPT_DECLINE_Y_OFFSET;
    label_b.glyphs.codes[0] = 0x41;
    label_b.glyphs.codes[1] = 0x41;
    label_b.glyphs.codes[2] = 0x43;
    label_b.glyphs.codes[3] = MENU_TEXT_END;

    for (;;) {
        if (result != KF_MENU_RESULT_PENDING) {
            menu_frame_begin();
            menu_draw_dialog_frame(summaries, overlay);
            menu_draw_window(kind, count, highlight_row, KF_MENU_CONFIRM_REQUESTED);
            menu_draw_two_option(&label_a, &label_b, selected, highlight);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
            return result;
        }

        highlight = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = PadRead(1);
        if (((input & PADLup) != 0 && (prev & PADLup) == 0) ||
            ((input & PADLdown) != 0 && (prev & PADLdown) == 0)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (selected != KF_MENU_CHOICE_ACCEPT)
                selected = KF_MENU_CHOICE_ACCEPT;
            else
                selected = KF_MENU_CHOICE_DECLINE;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            highlight = KF_MENU_CONFIRM_REQUESTED;
            result = menu_confirm_result_from_choice(selected);
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_CANCELLED;
        }

        menu_frame_begin();
        menu_draw_dialog_frame(summaries, overlay);
        menu_draw_window(kind, count, highlight_row, KF_MENU_CONFIRM_REQUESTED);
        menu_draw_two_option(&label_a, &label_b, selected, highlight);
        menu_present_frame();
    }
}

/*
 * Draw one menu window: an optional title label (drawn when the record's first
 * halfword is non-zero), then `count` selectable rows, then the shared
 * translucent backdrop for every window kind but the configuration window.  The highlighted row
 * (index `highlight`) takes the confirmed-selection background when confirmation is
 * requested and always gets the selection-cursor sprite overlaid.  Rows advance one
 * MenuGlyphString per step starting at the record's first row.
 */
ADDRESS(0x80028914, 0x15c)
void menu_draw_window(KfMenuWindowKind kind, s32 count, s32 highlight, KfMenuConfirmState confirmation)
{
    const MenuWindowLayout *layout;
    s32 row;

    layout = &menu_window_layouts[KF_ENUM_ENCODE(s32, kind)];
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    if (layout->title.position.x != 0) {
        menu_blit_sprite_translucent(
            &menu_assets.row_background, &layout->title.position);
        menu_draw_string(&menu_assets.glyph_atlas, &layout->title);
    }
    if (count > 0) {
        row = 0;
        do {
            if (row == highlight && confirmation == KF_MENU_CONFIRM_REQUESTED) {
                menu_blit_sprite_translucent(&menu_assets.row_confirmed_background,
                    &layout->rows[row].position);
            } else {
                menu_blit_sprite_translucent(&menu_assets.row_background,
                    &layout->rows[row].position);
            }
            if (row == highlight) {
                menu_blit_sprite(&menu_assets.selection_cursor, &layout->rows[row].position);
            }
            menu_draw_string(&menu_assets.glyph_atlas, &layout->rows[row]);
            row++;
        } while (row < count);
    }
    if (kind != KF_MENU_WINDOW_CONFIG) {
        menu_draw_window_backdrop();
    }
}

enum {
    MENU_LIST_TEXT_INSET = 3,
    MENU_LIST_ROW_HEIGHT = 12,
    MENU_LIST_QUANTITY_X_OFFSET = 110,
    MENU_LIST_QUANTITY_Y_OFFSET = 1
};

/*
 * Render a scrollable menu list: the optional title label, the visible glyph
 * rows (each with an optional two-digit quantity), the list backdrop tile, the
 * per-row highlight tiles (the cursor row taking the selected descriptor), the
 * scroll-end tile, and the shared window backdrop.  Reached from the item
 * buy/sell hubs and the other list menus.
 */
ADDRESS(0x80028a70, 0x77c)
void menu_list_render(const KfMenuList *list)
{
    MenuGlyphString gs;
    const MenuTileSprite *tile;
    s16 *src;
    u8 *counts;
    s32 row;
    s32 i;
    s32 yoff;
    u32 tens;
    u32 ones;

    src = list->glyph_rows;
    counts = list->quantities;
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    if (list->title.position.x != 0) {
        menu_blit_sprite_translucent(
            &menu_assets.row_background, &list->title.position);
        menu_draw_string(
            &menu_assets.glyph_atlas, &list->title);
    }

    counts = counts + list->scroll_offset;
    src = src + list->scroll_offset * list->glyphs_per_entry;
    row = 0;
    if (row < list->visible_rows && row < list->entry_count) {
        do {
            gs.position.x = list->list_x + MENU_LIST_TEXT_INSET;
            gs.position.y = list->list_y + MENU_LIST_TEXT_INSET;
            gs.position.y += row * MENU_LIST_ROW_HEIGHT;
            for (i = 0; i < list->glyphs_per_entry; i++) {
                gs.glyphs.codes[i] = *src++;
            }
            menu_draw_string(
                &menu_assets.glyph_atlas, &gs);
            if (list->quantities != NULL) {
                tens = *counts / 10u;
                ones = *counts % 10u;
                gs.position.x += MENU_LIST_QUANTITY_X_OFFSET;
                gs.position.y += MENU_LIST_QUANTITY_Y_OFFSET;
                gs.glyphs.codes[0] = tens;
                if (tens == 0) {
                    gs.glyphs.codes[0] = MENU_NUMBER_BLANK;
                }
                gs.glyphs.codes[1] = ones;
                gs.glyphs.codes[2] = MENU_TEXT_END;
                menu_draw_number(
                    &menu_assets.number_atlas, &gs);
                counts++;
            }
        } while (++row < list->visible_rows && row < list->entry_count);
    }

    tile = &menu_assets.list_tiles[MENU_LIST_TILE_BACKDROP];
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = tile->tpage;
    current_poly_ft4->clut = tile->clut;
    setXYWH(current_poly_ft4, list->list_x, list->list_y, tile->width, tile->height);
    setUVWH(current_poly_ft4, tile->u, tile->v, tile->width, tile->height);
    SetSemiTrans(current_poly_ft4, 1);
    primitive_buffer_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);

    row = 0;
    if (row < list->visible_rows) {
        yoff = 0;
        do {
            tile = &menu_assets.list_tiles[MENU_LIST_TILE_ROW];
            if (row == list->cursor_row) {
                tile = &menu_assets.list_tiles[MENU_LIST_TILE_SELECTED];
            }
            row++;
            primitive_buffer_begin_poly_ft4();
            current_poly_ft4->tpage = tile->tpage;
            current_poly_ft4->clut = tile->clut;
            setXYWH(current_poly_ft4,
                list->list_x,
                list->list_y + yoff + MENU_LIST_TEXT_INSET,
                tile->width,
                tile->height);
            setUVWH(current_poly_ft4, tile->u, tile->v, tile->width, tile->height);
            SetSemiTrans(current_poly_ft4, 1);
            primitive_buffer_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);
            yoff += MENU_LIST_ROW_HEIGHT;
        } while (row < list->visible_rows);
    }

    tile = &menu_assets.list_tiles[MENU_LIST_TILE_END];
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = tile->tpage;
    current_poly_ft4->clut = tile->clut;
    setXYWH(current_poly_ft4,
        list->list_x,
        list->list_y + list->visible_rows * MENU_LIST_ROW_HEIGHT + MENU_LIST_TEXT_INSET,
        tile->width,
        tile->height);
    setUVWH(current_poly_ft4, tile->u, tile->v, tile->width, tile->height);
    SetSemiTrans(current_poly_ft4, 1);
    primitive_buffer_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);

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

/* COM\\MIX.TIM font geometry; STAT.DAT descriptors confirm the cell extents. */
enum {
    MENU_FONT_COLUMNS = 16,
    MENU_FONT_CELL_WIDTH = 14,
    MENU_FONT_CELL_HEIGHT = 12,
    MENU_NUMBER_CELL_HEIGHT = 11,
    /* Kana marks occupy columns 14/15 of row 2 in the text atlas. */
    MENU_DAKUTEN_U = 14 * MENU_FONT_CELL_WIDTH,
    MENU_HANDAKUTEN_U = 15 * MENU_FONT_CELL_WIDTH,
    MENU_KANA_MARK_V = 2 * MENU_FONT_CELL_HEIGHT,
};

/*
 * Draw a two-option selection widget: one window frame around the selected
 * option, an option box on each of the two points (the selected point getting
 * the highlight box when highlighting is active), and a label under each.
 */
ADDRESS(0x800291ec, 0x10c)
void menu_draw_two_option(
    const MenuGlyphString *option0, const MenuGlyphString *option1,
    KfMenuConfirmChoice selected, KfMenuConfirmState highlight)
{
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    if (selected == KF_MENU_CHOICE_ACCEPT) {
        menu_blit_sprite(&menu_assets.selection_cursor, &option0->position);
    } else {
        menu_blit_sprite(&menu_assets.selection_cursor, &option1->position);
    }
    if (highlight == KF_MENU_CONFIRM_REQUESTED) {
        if (selected == KF_MENU_CHOICE_ACCEPT) {
            menu_blit_sprite_translucent(
                &menu_assets.option_highlight, &option0->position);
            menu_blit_sprite_translucent(
                &menu_assets.option_background, &option1->position);
        } else {
            menu_blit_sprite_translucent(
                &menu_assets.option_background, &option0->position);
            menu_blit_sprite_translucent(
                &menu_assets.option_highlight, &option1->position);
        }
    } else {
        menu_blit_sprite_translucent(
            &menu_assets.option_background, &option0->position);
        menu_blit_sprite_translucent(
            &menu_assets.option_background, &option1->position);
    }
    menu_draw_string(&menu_assets.glyph_atlas, option0);
    menu_draw_string(&menu_assets.glyph_atlas, option1);
}

/*
 * Draw the item-pickup preview: a spinning 3D model, its ten-glyph name, and four
 * mirrored window-sprite quads. The shared spin angle advances eight units.
 */
ADDRESS(0x800292f8, 0x7b8)
void menu_draw_item_name_frame(KF_ENUM_PARAM(KfObjectId, s32) item_id)
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

    light_source.m[0][0] = -KF_FIXED12_ONE;
    light_source.m[0][1] = -KF_FIXED12_ONE;
    light_source.m[0][2] = -KF_FIXED12_ONE;
    light_source.m[1][0] = -KF_FIXED12_ONE;
    light_source.m[1][1] = -KF_FIXED12_ONE;
    light_source.m[1][2] = -KF_FIXED12_ONE;
    light_source.m[2][0] = 0;
    light_source.m[2][1] = 0;
    light_source.m[2][2] = 0;
    MulMatrix0(&light_source, &rotation, &light_result);
    SetLightMatrix(&light_result);
    SetRotMatrix(&rotation);
    SetTransMatrix(&rotation);
    menu_render_item_model();

    rows = item_name_rows;
    name = &rows[KF_ENUM_ENCODE(s32, item_id)];
    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    string.position.x = 0x80;
    string.position.y = 0x24;
    for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
        string.glyphs.codes[i] = name->codes[i];
    }
    menu_draw_string(&menu_assets.glyph_atlas, &string);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_PICKUP_BACKDROP_LEFT_X,
        MENU_BACKDROP_TOP_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUVWH(current_poly_ft4,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_PICKUP_BACKDROP_RIGHT_X,
        MENU_BACKDROP_TOP_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUVWH(current_poly_ft4,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v,
        -menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_PICKUP_BACKDROP_LEFT_X,
        MENU_BACKDROP_BOTTOM_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUVWH(current_poly_ft4,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        menu_assets.window_backdrop.width,
        -menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_PICKUP_BACKDROP_RIGHT_X,
        MENU_BACKDROP_BOTTOM_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUVWH(current_poly_ft4,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        -menu_assets.window_backdrop.width,
        -menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][3]);
    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][2]);
    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][1]);
    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][0]);
}

/*
 * Build a translucent textured quad four pixels left and three above a menu point, and
 * link it at ordering-table depth 2000.
 */
ADDRESS(0x80029ab0, 0x1a0)
void menu_blit_sprite_translucent(
    const MenuSpriteDef *sprite, const MenuPoint *position)
{
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = sprite->tpage;
    current_poly_ft4->clut = sprite->clut;
    setXYWH(current_poly_ft4, position->x - MENU_TRANSLUCENT_SPRITE_X_OFFSET, position->y - MENU_TRANSLUCENT_SPRITE_Y_OFFSET,
        sprite->width, sprite->height);
    setUVWH(current_poly_ft4, sprite->u, sprite->v, sprite->width, sprite->height);
    SetSemiTrans(current_poly_ft4, 1);
    primitive_buffer_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);
}

/*
 * Build an opaque textured quad eighteen pixels left and two above a menu point, and link
 * it at ordering-table depth 2000.
 */
ADDRESS(0x80029c50, 0x190)
void menu_blit_sprite(
    const MenuSpriteDef *sprite, const MenuPoint *position)
{
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = sprite->tpage;
    current_poly_ft4->clut = sprite->clut;
    setXYWH(current_poly_ft4, position->x - MENU_OPAQUE_SPRITE_X_OFFSET, position->y - MENU_OPAQUE_SPRITE_Y_OFFSET,
        sprite->width, sprite->height);
    setUVWH(current_poly_ft4, sprite->u, sprite->v, sprite->width, sprite->height);
    primitive_buffer_commit_poly_ft4(MENU_WIDGET_OT_DEPTH);
}

/*
 * Render a positioned glyph string. The low 12 code bits select a 14x12 atlas
 * cell; bits 0x1000 and 0x2000 overlay the two decoration cells. Each emitted
 * quad links at ordering-table depth 1000.
 */
ADDRESS(0x80029de0, 0x530)
void menu_draw_string(
    const MenuSpriteDef *font, const MenuGlyphString *string)
{
    s32 i;
    s32 x_offset;

    for (i = 0; string->glyphs.codes[i] != MENU_TEXT_END; i++) {
        s32 glyph;

        x_offset = i * MENU_FONT_CELL_WIDTH;
        primitive_buffer_begin_poly_ft4();
        current_poly_ft4->tpage = font->tpage;
        current_poly_ft4->clut = font->clut;
        setXYWH(current_poly_ft4,
            string->position.x + x_offset,
            string->position.y,
            font->width,
            font->height);
        glyph = string->glyphs.codes[i] & MENU_TEXT_GLYPH_MASK;
        setUVWH(current_poly_ft4,
            (glyph % MENU_FONT_COLUMNS) * MENU_FONT_CELL_WIDTH,
            (glyph / MENU_FONT_COLUMNS) * MENU_FONT_CELL_HEIGHT,
            font->width,
            font->height);
        primitive_buffer_commit_poly_ft4(MENU_CONTENT_OT_DEPTH);

        if (string->glyphs.codes[i] & MENU_TEXT_DAKUTEN) {
            primitive_buffer_begin_poly_ft4();
            current_poly_ft4->tpage = font->tpage;
            current_poly_ft4->clut = font->clut;
            setXYWH(current_poly_ft4,
                string->position.x + x_offset,
                string->position.y,
                font->width,
                font->height);
            setUVWH(current_poly_ft4, MENU_DAKUTEN_U, MENU_KANA_MARK_V, font->width, font->height);
            primitive_buffer_commit_poly_ft4(MENU_CONTENT_OT_DEPTH);
        }

        if (string->glyphs.codes[i] & MENU_TEXT_HANDAKUTEN) {
            primitive_buffer_begin_poly_ft4();
            current_poly_ft4->tpage = font->tpage;
            current_poly_ft4->clut = font->clut;
            setXYWH(current_poly_ft4,
                string->position.x + x_offset,
                string->position.y,
                font->width,
                font->height);
            setUVWH(current_poly_ft4,
                MENU_HANDAKUTEN_U,
                MENU_KANA_MARK_V,
                font->width,
                font->height);
            primitive_buffer_commit_poly_ft4(MENU_CONTENT_OT_DEPTH);
        }
    }
}

/*
 * Render a fixed-pitch digit run from a single-column atlas. Each signed code
 * selects an eleven-texel row, screen positions advance seven pixels, and all
 * quads link at ordering-table depth 1000.
 */
ADDRESS(0x8002a310, 0x200)
void menu_draw_number(
    const MenuSpriteDef *atlas, const MenuGlyphString *string)
{
    const MenuSpriteDef *font = atlas;
    const MenuGlyphString *label = string;
    s32 i;
    s32 xoff;

    for (i = 0; label->glyphs.codes[i] != MENU_TEXT_END; i++) {
        xoff = i * MENU_NUMBER_ADVANCE;
        primitive_buffer_begin_poly_ft4();
        current_poly_ft4->tpage = font->tpage;
        current_poly_ft4->clut = font->clut;
        setXYWH(current_poly_ft4,
            label->position.x + xoff,
            label->position.y,
            font->width,
            font->height);
        setUVWH(current_poly_ft4,
            font->u,
            label->glyphs.codes[i] * MENU_NUMBER_CELL_HEIGHT,
            font->width,
            font->height);
        primitive_buffer_commit_poly_ft4(MENU_CONTENT_OT_DEPTH);
    }
}

/*
 * Draw the shared translucent menu-window backdrop.  Four semi-transparent
 * textured tiles built in the primitive workspace cover the window interior in
 * a 2x2 grid at (166,16), (237,16), (166,120), (237,120) -- all linked at
 * ordering-table depth 2900 -- then the four persistent border quads are
 * enqueued at depth 3000.  Shared by the status panel, the option-window
 * renderer and the scrollable list widget.
 */
ADDRESS(0x8002a510, 0x6a4)
void menu_draw_window_backdrop(void)
{
    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_BACKDROP_LEFT_X,
        MENU_BACKDROP_TOP_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUVWH(current_poly_ft4,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_BACKDROP_RIGHT_X,
        MENU_BACKDROP_TOP_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUVWH(current_poly_ft4,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v,
        -menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_BACKDROP_LEFT_X,
        MENU_BACKDROP_BOTTOM_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUVWH(current_poly_ft4,
        menu_assets.window_backdrop.u,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        menu_assets.window_backdrop.width,
        -menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        MENU_BACKDROP_RIGHT_X,
        MENU_BACKDROP_BOTTOM_Y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUVWH(current_poly_ft4,
        menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        -menu_assets.window_backdrop.width,
        -menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);

    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][3]);
    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][2]);
    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][1]);
    AddPrim(
        game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH,
        &menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][0]);
}

/*
 * Begin a menu frame: flip the double buffer, re-anchor the active primitive
 * buffer and ordering table for the new buffer index, clear the ordering
 * table, and rewind the primitive-buffer cursor.
 */
ADDRESS(0x8002abb4, 0x80)
void menu_frame_begin(void)
{
    game_graphics_runtime.display_state.buffer_index = display_next_buffer(game_graphics_runtime.display_state.buffer_index);
    game_graphics_runtime.display_state.primitive_buffer =
        &game_graphics_runtime.display_state.primitive_buffers[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)];
    game_graphics_runtime.display_state.ordering_table =
        game_graphics_runtime.display_state.ordering_tables[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)].entries;
    ClearOTagR(game_graphics_runtime.display_state.ordering_table, KF_ORDERING_TABLE_LENGTH);
    game_graphics_runtime.display_state.primitive_buffer->cursor =
        game_graphics_runtime.display_state.primitive_buffer->start;
}

/* Wait for the GPU/retrace, install the active environments, and submit OT. */
ADDRESS(0x8002ac34, 0x98)
void menu_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&game_graphics_runtime.display_draw_environments[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)]);
    PutDispEnv(&game_graphics_runtime.display_disp_environments[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)]);
    DrawOTag(game_graphics_runtime.display_state.ordering_table + (KF_ORDERING_TABLE_LENGTH - 1));
}

ADDRESS(0x8002accc, 0x50)
void primitive_buffer_begin_poly_ft4(void)
{
    SetPolyFT4(current_poly_ft4);
    setRGB0(current_poly_ft4,
        MENU_PRIMITIVE_BRIGHTNESS,
        MENU_PRIMITIVE_BRIGHTNESS,
        MENU_PRIMITIVE_BRIGHTNESS);
}

ADDRESS(0x8002ad1c, 0x50)
void primitive_buffer_commit_poly_ft4(s32 depth)
{
    AddPrim(
        &game_graphics_runtime.display_state.ordering_table[depth],
        current_poly_ft4);
    current_poly_ft4++;
    game_graphics_runtime.display_state.primitive_buffer->cursor = (u8 *)current_poly_ft4;
}

/* Initialize a menu list header and copy its label glyphs from the table. */
ADDRESS(0x8002ad6c, 0x8c)
void menu_list_init(KfMenuList *list, KfMenuWindowKind kind, s32 row)
{
    s32 i;

    list->title.position.x = 12;
    list->title.position.y = 19;
    for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
        list->title.glyphs.codes[i] = menu_window_layouts[KF_ENUM_ENCODE(s32, kind)].rows[row].glyphs.codes[i];
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

/* Expand a signed value into decimal glyphs and a trailing -1 sentinel. */
ADDRESS(0x8002adf8, 0xac)
void menu_format_number(s32 value, s32 count, KF_ENUM_PARAM(KfFormatPaddingMode, s32) pad_zero, s16 *out)
{
    s32 i = 0;
    s32 blank;

    blank = (pad_zero == KF_FORMAT_PAD_SPACES) ? MENU_NUMBER_BLANK : 0;
    for (; i < count; i++) {
        out[i] = blank;
    }
    out[count] = MENU_TEXT_END;
    for (i = count - 1; i >= 0; i--) {
        out[i] = value % 10;
        value /= 10;
        if (value == 0) {
            i = -1;
        }
    }
}

/* Release the previous item model, then load and register the selection. */
ADDRESS(0x8002aea4, 0x68)
KF_ENUM_PARAM(KfResourceLoadResult, u32) menu_load_item_model(KF_ENUM_PARAM(KfObjectId, s32) id)
{
    void *asset;

    menu_release_item_model();
    if (id != KF_OBJECT_NONE) {
        if (cd_file_load_table_entry(&asset, KF_ENUM_ENCODE(s32, id)) != KF_RESOURCE_LOADED) {
            return KF_RESOURCE_LOAD_FAILED;
        }
        tmd_register(KF_TMD_SLOT_MENU_ITEM, (KfTmdHeader *)asset);
        menu_item_model_allocation_pending = KF_MENU_MODEL_ALLOCATED;
    }
    menu_item_preview_rotation.vy = 0;
    return KF_RESOURCE_LOADED;
}

ADDRESS(0x8002af0c, 0x3c)
void menu_release_item_model(void)
{
    if (menu_item_model_allocation_pending == KF_MENU_MODEL_ALLOCATED) {
        tmd_release_last_allocation(KF_TMD_SLOT_MENU_ITEM);
        menu_item_model_allocation_pending = KF_MENU_MODEL_RELEASED;
    }
}

/* Load the numbered item TIM into the active primitive buffer and VRAM. */
ADDRESS(0x8002af48, 0x130)
KF_ENUM_PARAM(KfResourceLoadResult, u32) menu_load_item_texture(KfMenuTextureId id)
{
    char name[16] = "TIM\\M000.";
    void *destination;
    s32 number;
    s32 remainder;

    if (id != KF_MENU_TEXTURE_NONE) {
        number = KF_ENUM_ENCODE(s32, id) + 1;
        name[5] = number / 100 + '0';
        remainder = number % 100;
        name[6] = remainder / 10 + '0';
        name[7] = remainder % 10 + '0';
        destination = game_graphics_runtime.display_state.primitive_buffer->cursor;
        if (cd_file_load_into(destination, name) != KF_RESOURCE_LOADED) {
            return KF_RESOURCE_LOAD_FAILED;
        }
        tim_upload_images(destination);
    }
    return KF_RESOURCE_LOADED;
}
