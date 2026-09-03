#include <kf/address.h>
#include <kf/semantic_types.h>

/* Psy-Q MEMORY.H declares memset without a prototype. */
extern void *memset();

/* Shared menu primitives: frame begin/flush, header/list/menu draw, input
 * sound cue, and the vsync/pad poll. */
extern void menu_frame_begin(void);
extern void menu_present_frame(void);
extern void menu_add_frame_quad(void);
extern void menu_draw_dialog_frame(void *summaries, s32 cursor);
extern void menu_draw_window(s32 object, s32 arg1, s32 arg2, s32 arg3);
extern void menu_play_input_sound(s32 cue);
extern u32 pad_read();

/* Save-catalogue / slot / card workers and the save effect hook. */
extern s32 save_system_read_catalog(KfSaveSlotSummary *summaries);
extern s32 save_system_write_slot(s16 slot_id);
extern s32 save_file_cleanup_temporary(void);
extern s32 memory_card_check_or_format(s16 allow_format);
extern s32 menu_two_option_prompt(s32 arg0, u32 arg1, u32 arg2, u32 arg3);
extern u32 menu_load_item_texture(s32 arg0);

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
    KfSaveSlotSummary summaries[3];
    s32 cursor = 0;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 result = -99;
    s32 status;
    s32 i;

    status = save_system_read_catalog(summaries);
    if (status != 1 && status != 3) {
        menu_play_input_sound(0);
        while (pad_read(1) == 0) {
            menu_frame_begin();
            menu_add_frame_quad();
            menu_draw_dialog_frame(summaries, cursor);
            menu_draw_window(4, 5, cursor, confirm);
            menu_present_frame();
        }
        menu_play_input_sound(2);
        while (pad_read(1) != 0)
            ;
        return -1;
    }

    for (;;) {
        if (confirm == 1 || result == -1) {
            menu_frame_begin();
            menu_draw_dialog_frame(summaries, cursor);
            menu_draw_window(4, 5, cursor, confirm);
            menu_present_frame();
            while (pad_read(1) != 0)
                ;
        }

        if (confirm == 1 && cursor != 4) {
            if (cursor == 3) {
                status = save_file_cleanup_temporary();
                if (status == 1) {
                    menu_load_item_texture(0x72);
                    while (pad_read(1) == 0) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, cursor);
                        menu_draw_window(4, 5, cursor, confirm);
                        menu_present_frame();
                    }
                }
                menu_play_input_sound(0);
                while (pad_read(1) != 0)
                    ;
            }

            result = menu_two_option_prompt(4, 5, cursor, (u32)summaries);
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
                    while (pad_read(1) == 0) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        menu_draw_dialog_frame(summaries, cursor);
                        menu_draw_window(4, 5, cursor, confirm);
                        menu_present_frame();
                    }
                    menu_play_input_sound(2);
                    while (pad_read(1) != 0)
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
        input = pad_read(1);
        if ((input & 0x1000) != 0 && (prev & 0x1000) == 0) {
            menu_play_input_sound(0);
            if (cursor != 0)
                cursor--;
            else
                cursor = 4;
        } else if ((input & 0x4000) != 0 && (prev & 0x4000) == 0) {
            menu_play_input_sound(0);
            if (cursor != 4)
                cursor++;
            else
                cursor = 0;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            confirm = 1;
            if (cursor == 4)
                result = -1;
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            result = -1;
        }

        menu_frame_begin();
        menu_draw_dialog_frame(summaries, cursor);
        menu_draw_window(4, 5, cursor, confirm);
        menu_present_frame();
    }
}
