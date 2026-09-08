#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

/* Zero-based TIM/Mddd. indices; menu_load_item_texture adds one to the ID. */
enum {
    MENU_TEXTURE_LOADING_DATA = 0x67,
    MENU_TEXTURE_SAVING_DATA = 0x68,
    MENU_TEXTURE_FORMATTING_CARD = 0x69,
    MENU_TEXTURE_CONFIRM_CARD_FORMAT = 0x72,
    MENU_TEXTURE_POWER_OFF = 0x3e6
};

/*
 * Inventory drop and save/load menu panels, one contiguous run
 * (0x800249a8..0x8002589c): the drop-item list panel, the save/load hub, and
 * the save and load slot panels. They share the windowed-cursor input loop and
 * the shared menu draw primitives. Module boundary is WIP.
 */

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
    KfItemId codes[KF_ITEM_COUNT];
    u8 *inv;
    s32 found;
    s32 code;
    s32 j;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = KF_MENU_LIST_PENDING;

    while (PadRead(1) != 0)
        ;
    menu_list_init(&ctx, KF_MENU_WINDOW_ROOT, KF_ENUM_ENCODE(s32, KF_ROOT_CHOICE_DROP_ITEM));

    found = 0;
    code = 0;
    inv = item_stock[KF_ITEM_STOCK_PLAYER];
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
                codes[found] = KF_ENUM_DECODE(KfItemId, code);
                found++;
            }
        }
    }

    ctx.entry_count = found;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = 0;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_model(codes[ctx.selected_index]) != 0)
            return;
        menu_item_model_preview(codes[ctx.selected_index]);
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            selection = KF_ENUM_ENCODE(s32, menu_list_interact(&ctx, KF_MENU_CONFIRM_DROP,
                    KF_MENU_PREVIEW_ITEM_MODEL, KF_ENUM_ENCODE(u8, codes[ctx.selected_index]), 0, KF_ITEM_PRICE_BUY));
            if (selection == KF_ENUM_ENCODE(s32, KF_MENU_CONFIRM_CANCELLED))
                selection = KF_MENU_LIST_PENDING;
            else
                selection = KF_ENUM_ENCODE(u8, codes[ctx.selected_index]);
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (selection != KF_MENU_LIST_PENDING) {
            while (PadRead(1) != 0)
                ;
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = KF_MENU_LIST_NO_SELECTION;
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
            if (menu_load_item_model(codes[ctx.selected_index]) != 0)
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
            if (menu_load_item_model(codes[ctx.selected_index]) != 0)
                return;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = KF_MENU_LIST_NO_SELECTION;
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_item_model_preview(codes[ctx.selected_index]);
        menu_list_render(&ctx);
    }

    menu_release_item_model();
    if (selection != KF_MENU_LIST_NO_SELECTION)
        inv[selection]--;
}

/*
 * Three-row hub dispatched by menu_root. The load action maps result 0 to -3;
 * accepting the second action loads texture 0x3e6, stops music, and redraws
 * forever. Cancellation of a sub-action keeps this hub open.
 */
ADDRESS(0x80024e64, 0x260)
s32 menu_save_load_hub(void)
{
    KfSaveHeader header;
    KfSavePayload payload;
    s32 cursor = KF_MENU_SYSTEM_LOAD_ROW;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 result = KF_MENU_ROOT_PENDING;
    s32 action = -1;

    save_payload_buffer = &payload;
    save_header_buffer = &header;

    for (;;) {
        if (action != -1 || result == action) {
            menu_frame_begin();
            menu_draw_window(KF_MENU_WINDOW_SYSTEM, KF_MENU_SYSTEM_ROW_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }

        switch (action) {
        case KF_MENU_SYSTEM_LOAD_ROW:
            result = KF_ENUM_ENCODE(s32, menu_load_panel());
            if (result == KF_ENUM_ENCODE(s32, KF_MENU_CONFIRM_ACCEPTED))
                result = KF_MENU_ROOT_GAME_LOADED;
            break;
        case KF_MENU_SYSTEM_QUIT_ROW:
            result = KF_ENUM_ENCODE(s32, menu_two_option_prompt(
                KF_MENU_WINDOW_SYSTEM, KF_MENU_SYSTEM_ROW_COUNT, cursor, 0));
            if (result == KF_ENUM_ENCODE(s32, KF_MENU_CONFIRM_ACCEPTED)) {
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

        if (action != -1 && result == KF_MENU_ROOT_NO_ITEM)
            result = KF_MENU_ROOT_PENDING;
        action = -1;
        if (result != KF_MENU_ROOT_PENDING)
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
                cursor = KF_MENU_SYSTEM_LOAD_ROW;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (cursor == KF_MENU_SYSTEM_RETURN_ROW) {
                result = KF_MENU_ROOT_NO_ITEM;
            } else {
                action = cursor;
            }
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_ROOT_NO_ITEM;
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
KfMenuConfirmResult menu_save_panel(void)
{
    KfSaveSlotSummary summaries[KF_SAVE_SLOT_COUNT];
    s32 cursor = 0;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    KfMenuConfirmResult result = KF_MENU_CONFIRM_PENDING;
    union {
        KfSaveResult operation;
        KfSaveCleanupResult cleanup;
    } status;
    s32 i;

    status.operation = save_system_read_catalog(summaries);
    if (status.operation != KF_SAVE_RESULT_OK && status.operation != KF_ENUM_DECODE(KfSaveResult, 3)) {
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
        return KF_MENU_CONFIRM_CANCELLED;
    }

    for (;;) {
        if (confirm == KF_MENU_CONFIRM_REQUESTED || result == KF_MENU_CONFIRM_CANCELLED) {
            menu_frame_begin();
            menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
            menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }

        if (confirm == KF_MENU_CONFIRM_REQUESTED && cursor != KF_MENU_SAVE_RETURN_ROW) {
            if (cursor == KF_MENU_SAVE_FORMAT_ROW) {
                status.cleanup = save_file_cleanup_temporary();
                if (status.cleanup == KF_SAVE_CLEANUP_TEMP_OPENED) {
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
            if (result == KF_MENU_CONFIRM_CANCELLED) {
                result = KF_MENU_CONFIRM_PENDING;
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
                    status.operation = save_system_write_slot(KF_ENUM_DECODE(KfSaveSlotArgument, cursor + KF_ENUM_ENCODE(s16, KF_SAVE_SLOT_FIRST)));
                } else if (cursor == KF_MENU_SAVE_FORMAT_ROW) {
                    menu_load_item_texture(MENU_TEXTURE_FORMATTING_CARD);
                    for (i = 0; i < 3; i++) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
                        menu_draw_window(KF_MENU_WINDOW_SAVE, KF_MENU_SAVE_ROW_COUNT, cursor, confirm);
                        menu_present_frame();
                    }
                    status.operation = memory_card_check_or_format(KF_CARD_FORMAT_CONFIRMED);
                    memset(summaries, 0, sizeof(summaries));
                }

                if (status.operation != KF_SAVE_RESULT_OK) {
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
                    result = KF_MENU_CONFIRM_PENDING;
                } else if (cursor == KF_MENU_SAVE_FORMAT_ROW) {
                    result = KF_MENU_CONFIRM_PENDING;
                }
            }
        }

        confirm = KF_MENU_CONFIRM_IDLE;
        if (result != KF_MENU_CONFIRM_PENDING)
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
                result = KF_MENU_CONFIRM_CANCELLED;
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_CONFIRM_CANCELLED;
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
KfMenuConfirmResult menu_load_panel(void)
{
    KfSaveSlotSummary summaries[KF_SAVE_SLOT_COUNT];
    s32 cursor = 0;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    KfMenuConfirmResult result = KF_MENU_CONFIRM_PENDING;
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
        return KF_MENU_CONFIRM_CANCELLED;
    }

    for (;;) {
        if (confirm == KF_MENU_CONFIRM_REQUESTED || result == KF_MENU_CONFIRM_CANCELLED) {
            menu_frame_begin();
            menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
            menu_draw_window(KF_MENU_WINDOW_LOAD, KF_MENU_LOAD_ROW_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }

        if (confirm == KF_MENU_CONFIRM_REQUESTED && cursor != KF_MENU_LOAD_RETURN_ROW) {
            result = menu_two_option_prompt(KF_MENU_WINDOW_LOAD, KF_MENU_LOAD_ROW_COUNT, cursor, summaries);
            if (result == KF_MENU_CONFIRM_CANCELLED) {
                result = KF_MENU_CONFIRM_PENDING;
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
                    result = KF_MENU_CONFIRM_PENDING;
                }
            }
        }

        confirm = KF_MENU_CONFIRM_IDLE;
        if (result != KF_MENU_CONFIRM_PENDING)
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
                result = KF_MENU_CONFIRM_CANCELLED;
            } else if (summaries[cursor].current_hp == 0) {
                menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            } else {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                confirm = KF_MENU_CONFIRM_REQUESTED;
            }
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_CONFIRM_CANCELLED;
        }

        menu_frame_begin();
        menu_draw_dialog_frame(summaries, KF_ENUM_DECODE(KfSaveSlotOverlay, cursor));
        menu_draw_window(KF_MENU_WINDOW_LOAD, KF_MENU_LOAD_ROW_COUNT, cursor, confirm);
        menu_present_frame();
    }
    return result;
}
