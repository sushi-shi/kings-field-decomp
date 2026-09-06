#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

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
 *
 * Structurally exact; open residue is the list-widget callee-saved-register
 * permutation plus one loop delay-slot swap (see docs/patterns/
 * source-shapes-gcc257.md, "item / inventory menu panels").
 */
ADDRESS(0x800249a8, 0x4bc)
void menu_drop_item(void)
{
    KfMenuList ctx;
    s16 labels[80][10];
    u8 counts[80];
    u8 codes[80];
    u8 *inv;
    u8 *equip;
    s16 *name;
    s32 found;
    s32 code;
    s32 j;
    s32 input = 0;
    s32 prev;
    s32 confirm = 0;
    s32 selection = -99;

    while (PadRead(1) != 0)
        ;
    menu_list_init(&ctx, 0, 4);

    found = 0;
    /* The seven equipment ids: [0] worn weapon, [0x2c..0x31] shield, head,
     * body, arm and leg armour, and accessory. */
    equip = &player_state.equipped_weapon_id;
    inv = item_stock[0];
    name = item_name_rows[0].codes;
    for (code = 0; code < 80; code++, name += 10) {
        if (inv[code] != 0) {
            counts[found] = inv[code];
            if (code == equip[0x00] || code == equip[0x2c] ||
                code == equip[0x2d] || code == equip[0x2e] ||
                code == equip[0x2f] || code == equip[0x30] ||
                code == equip[0x31])
                counts[found]--;
            if (counts[found] != 0) {
                for (j = 0; j < 10; j++)
                    labels[found][j] = name[j];
                codes[found] = code;
                found++;
            }
        }
    }

    ctx.entry_count = found;
    ctx.glyphs_per_entry = 10;
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
        if (confirm == 1) {
            if (menu_list_interact(&ctx, KF_MENU_CONFIRM_DROP,
                    KF_MENU_PREVIEW_ITEM_MODEL, codes[ctx.selected_index], 0, KF_ITEM_PRICE_BUY)
                    == KF_MENU_CONFIRM_CANCELLED)
                selection = -99;
            else
                selection = codes[ctx.selected_index];
        }
        confirm = 0;
        if (selection != -99) {
            while (PadRead(1) != 0)
                ;
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = -1;
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
            confirm = 1;
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = -1;
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_item_model_preview(codes[ctx.selected_index]);
        menu_list_render(&ctx);
    }

    menu_release_item_model();
    if (selection != -1)
        inv[selection]--;
}

/*
 * Save/load hub dispatched by the option menu (slot 5).  Runs a three-row
 * cursor (0 load, 1 save, 2 exit); confirm on a data row invokes the matching
 * worker.  A successful load returns its slot result (or -3 when the worker
 * reports an empty catalogue); a successful save plays the confirmation cue,
 * stops the map sequence, and holds the save-complete frame.  Returns the
 * chosen result, or -1/-99 on cancel.
 *
 * Structurally exact; open residue is the two-arm action dispatch's basic-block
 * layout (see docs/patterns/source-shapes-gcc257.md, "item / inventory menu
 * panels").
 */
ADDRESS(0x80024e64, 0x260)
s32 menu_save_load_hub(void)
{
    KfSaveHeader header;
    KfSavePayload payload;
    s32 cursor = 0;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 result = -99;
    s32 action = -1;

    save_payload_buffer = &payload;
    save_header_buffer = &header;

    for (;;) {
        if (action != -1 || result == action) {
            menu_frame_begin();
            menu_draw_window(2, 3, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }

        if (action == 0) {
            result = menu_load_panel();
            if (result == 0)
                result = -3;
        } else if (action == 1) {
            result = menu_two_option_prompt(2, 3, cursor, 0);
            if (result == 0) {
                menu_load_item_texture(0x3e6);
                audio_stop_sequence_fade();
                for (;;) {
                    menu_frame_begin();
                    menu_add_frame_quad();
                    menu_draw_window(2, 3, cursor, confirm);
                    menu_present_frame();
                }
            }
        }

        if (action != -1 && result == -1)
            result = -99;
        if (result != -99)
            return result;

        action = -1;
        confirm = 0;
        prev = input;
        input = PadRead(1);
        if ((input & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 0)
                cursor--;
            else
                cursor = 2;
        } else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 2)
                cursor++;
            else
                cursor = 0;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            if (cursor == 2) {
                result = -1;
            } else {
                confirm = 1;
                action = cursor;
            }
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = -1;
        }

        menu_frame_begin();
        menu_draw_window(2, 3, cursor, confirm);
        menu_present_frame();
    }
}

/*
 * Save panel dispatched by the save-confirmation screen.  Reads the catalogue
 * and runs a five-row cursor: three data slots, a card-format row (3), and an
 * exit row (4).  Confirm on a data slot opens the dialog, plays the save
 * effect, holds a three-frame animation, and writes the slot.  The format row
 * first purges any temporary file, then reformats the card and clears the
 * catalogue.  Returns the slot dialog result, or -1 on cancel.
 *
 * Structurally exact; open residue is the callee-saved-register count (retail
 * keeps the reused status result in a dedicated register), which cascades every
 * offset (see docs/patterns/source-shapes-gcc257.md, "item / inventory menu
 * panels").
 */
ADDRESS(0x800250c4, 0x468)
s32 menu_save_panel(void)
{
    KfSaveSlotSummary summaries[KF_SAVE_SLOT_COUNT];
    s32 cursor = 0;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 result = -99;
    s32 status;
    s32 i;

    status = save_system_read_catalog(summaries);
    if (status != 1 && status != 3) {
        menu_play_input_sound(MENU_SOUND_CURSOR);
        while (PadRead(1) == 0) {
            menu_frame_begin();
            menu_add_frame_quad();
            menu_draw_dialog_frame(summaries, cursor);
            menu_draw_window(4, 5, cursor, confirm);
            menu_present_frame();
        }
        menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
        while (PadRead(1) != 0)
            ;
        return -1;
    }

    for (;;) {
        if (confirm == 1 || result == -1) {
            menu_frame_begin();
            menu_draw_dialog_frame(summaries, cursor);
            menu_draw_window(4, 5, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }

        if (confirm == 1 && cursor != 4) {
            if (cursor == 3) {
                status = save_file_cleanup_temporary();
                if (status == 1) {
                    menu_load_item_texture(0x72);
                    while (PadRead(1) == 0) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, cursor);
                        menu_draw_window(4, 5, cursor, confirm);
                        menu_present_frame();
                    }
                }
                menu_play_input_sound(MENU_SOUND_CURSOR);
                while (PadRead(1) != 0)
                    ;
            }

            result = menu_two_option_prompt(4, 5, cursor, summaries);
            if (result == -1) {
                result = -99;
            } else {
                if (cursor < 3) {
                    menu_load_item_texture(0x68);
                    for (i = 0; i < 3; i++) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, cursor);
                        menu_draw_window(4, 5, cursor, confirm);
                        menu_present_frame();
                    }
                    status = save_system_write_slot(cursor + 1);
                } else {
                    menu_load_item_texture(0x69);
                    for (i = 0; i < 3; i++) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, cursor);
                        menu_draw_window(4, 5, cursor, confirm);
                        menu_present_frame();
                    }
                    status = memory_card_check_or_format(1);
                    memset(summaries, 0, sizeof(summaries));
                }

                if (status != 1) {
                    while (PadRead(1) == 0) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, cursor);
                        menu_draw_window(4, 5, cursor, confirm);
                        menu_present_frame();
                    }
                    menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
                    while (PadRead(1) != 0)
                        ;
                    result = -99;
                } else if (cursor == 3) {
                    result = -99;
                }
            }
        }

        if (result != -99)
            return result;

        confirm = 0;
        prev = input;
        input = PadRead(1);
        if ((input & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 0)
                cursor--;
            else
                cursor = 4;
        } else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 4)
                cursor++;
            else
                cursor = 0;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = 1;
            if (cursor == 4)
                result = -1;
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = -1;
        }

        menu_frame_begin();
        menu_draw_dialog_frame(summaries, cursor);
        menu_draw_window(4, 5, cursor, confirm);
        menu_present_frame();
    }
}

/*
 * Load panel: reads the memory-card catalogue into three slot summaries and
 * runs a four-row cursor (three data slots plus an exit row).  Confirm on a
 * populated slot opens the confirmation dialog, plays the load effect, holds a
 * three-frame animation, and reads the slot; a failed read shows the error
 * frame.  Returns the worker's slot result, or -1 on cancel.
 *
 * Structurally exact; open residue is loop-invariant constant hoisting (retail
 * keeps 1 and 3 in callee-saved registers, cc1psx-257 rematerialises them),
 * which cascades every offset (see docs/patterns/source-shapes-gcc257.md,
 * "item / inventory menu panels").
 */
ADDRESS(0x8002552c, 0x370)
s32 menu_load_panel(void)
{
    KfSaveSlotSummary summaries[KF_SAVE_SLOT_COUNT];
    s32 cursor = 0;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 result = -99;
    s32 i;

    if (save_system_read_catalog(summaries) != 1) {
        menu_play_input_sound(MENU_SOUND_CURSOR);
        while (PadRead(1) == 0) {
            menu_frame_begin();
            menu_add_frame_quad();
            menu_draw_dialog_frame(summaries, cursor);
            menu_draw_window(5, 4, cursor, confirm);
            menu_present_frame();
        }
        menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
        while (PadRead(1) != 0)
            ;
        return -1;
    }

    for (;;) {
        if (confirm == 1 || result == -1) {
            menu_frame_begin();
            menu_draw_dialog_frame(summaries, cursor);
            menu_draw_window(5, 4, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }

        if (confirm == 1 && cursor != 3) {
            result = menu_two_option_prompt(5, 4, cursor, summaries);
            if (result == -1) {
                result = -99;
            } else {
                menu_load_item_texture(0x67);
                for (i = 0; i < 3; i++) {
                    menu_frame_begin();
                    menu_add_frame_quad();
                    menu_draw_dialog_frame(summaries, cursor);
                    menu_draw_window(5, 4, cursor, confirm);
                    menu_present_frame();
                }
                if (save_system_read_slot(cursor + 1) != 1) {
                    while (PadRead(1) == 0) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, cursor);
                        menu_draw_window(5, 4, cursor, confirm);
                        menu_present_frame();
                    }
                    menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
                    while (PadRead(1) != 0)
                        ;
                    result = -99;
                }
            }
        }

        if (result != -99)
            return result;

        confirm = 0;
        prev = input;
        input = PadRead(1);
        if ((input & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 0)
                cursor--;
            else
                cursor = 3;
        } else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 3)
                cursor++;
            else
                cursor = 0;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            if (cursor == 3) {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                confirm = 1;
                result = -1;
            } else if (summaries[cursor].current_hp != 0) {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                confirm = 1;
            } else {
                menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            }
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = -1;
        }

        menu_frame_begin();
        menu_draw_dialog_frame(summaries, cursor);
        menu_draw_window(5, 4, cursor, confirm);
        menu_present_frame();
    }
}
