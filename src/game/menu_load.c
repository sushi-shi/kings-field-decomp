#include <kf/address.h>
#include <kf/semantic_types.h>

/* Shared menu primitives: frame begin/flush, header/list/menu draw, input
 * sound cue, and the vsync/pad poll. */
extern void menu_frame_begin(void);
extern void menu_present_frame(void);
extern void menu_add_frame_quad(void);
extern void func_80027ee4(void *summaries, s32 cursor);
extern void menu_draw_window(s32 object, s32 arg1, s32 arg2, s32 arg3);
extern void menu_play_input_sound(s32 cue);
extern u32 pad_read();

/* Save-catalogue / slot workers and the load effect hook. */
extern s32 save_system_read_catalog(KfSaveSlotSummary *summaries);
extern s32 save_system_read_slot(s16 slot_id);
extern s32 menu_two_option_prompt(s32 arg0, u32 arg1, u32 arg2, u32 arg3);
extern u32 menu_load_item_texture(s32 arg0);

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
    KfSaveSlotSummary summaries[3];
    s32 cursor = 0;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 result = -99;
    s32 i;

    if (save_system_read_catalog(summaries) != 1) {
        menu_play_input_sound(0);
        while (pad_read(1) == 0) {
            menu_frame_begin();
            menu_add_frame_quad();
            func_80027ee4(summaries, cursor);
            menu_draw_window(5, 4, cursor, confirm);
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
            func_80027ee4(summaries, cursor);
            menu_draw_window(5, 4, cursor, confirm);
            menu_present_frame();
            while (pad_read(1) != 0)
                ;
        }

        if (confirm == 1 && cursor != 3) {
            result = menu_two_option_prompt(5, 4, cursor, (u32)summaries);
            if (result == -1) {
                result = -99;
            } else {
                menu_load_item_texture(0x67);
                for (i = 0; i < 3; i++) {
                    menu_frame_begin();
                    menu_add_frame_quad();
                    func_80027ee4(summaries, cursor);
                    menu_draw_window(5, 4, cursor, confirm);
                    menu_present_frame();
                }
                if (save_system_read_slot(cursor + 1) != 1) {
                    while (pad_read(1) == 0) {
                        menu_frame_begin();
                        menu_add_frame_quad();
                        func_80027ee4(summaries, cursor);
                        menu_draw_window(5, 4, cursor, confirm);
                        menu_present_frame();
                    }
                    menu_play_input_sound(2);
                    while (pad_read(1) != 0)
                        ;
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
                cursor = 3;
        } else if ((input & 0x4000) != 0 && (prev & 0x4000) == 0) {
            menu_play_input_sound(0);
            if (cursor != 3)
                cursor++;
            else
                cursor = 0;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            if (cursor == 3) {
                menu_play_input_sound(1);
                confirm = 1;
                result = -1;
            } else if (summaries[cursor].fields[2] != 0) {
                menu_play_input_sound(1);
                confirm = 1;
            } else {
                menu_play_input_sound(2);
            }
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            result = -1;
        }

        menu_frame_begin();
        func_80027ee4(summaries, cursor);
        menu_draw_window(5, 4, cursor, confirm);
        menu_present_frame();
    }
}
