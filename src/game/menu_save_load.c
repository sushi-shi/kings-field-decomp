#include <kf/address.h>
#include <kf/semantic_types.h>

/* Shared save workspace pointers (rebound to the caller's frame buffers). */
extern KfSaveHeader *save_header_buffer;
extern KfSavePayload *save_payload_buffer;

/* Shared menu primitives: frame begin/flush, header/menu draw, input sound
 * cue, and the vsync/pad poll. */
extern void menu_frame_begin(void);
extern void func_8002ac34(void);
extern void func_80027ea0(void);
extern void menu_draw_window(s32 object, s32 arg1, s32 arg2, s32 arg3);
extern void menu_play_input_sound(s32 cue);
extern u32 pad_read();

/* Save/load workers and the save-complete effect/audio hooks. */
extern s32 func_8002552c(void);
extern s32 menu_two_option_prompt(s32 kind, s32 menu_id, s32 arg2, void *arg3);
extern u32 func_8002af48(s32 arg0);
extern void audio_stop_sequence_fade(void);

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
s32 func_80024e64(void)
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
            func_8002ac34();
            while (pad_read(1) != 0)
                ;
        }

        if (action == 0) {
            result = func_8002552c();
            if (result == 0)
                result = -3;
        } else if (action == 1) {
            result = menu_two_option_prompt(2, 3, cursor, 0);
            if (result == 0) {
                func_8002af48(0x3e6);
                audio_stop_sequence_fade();
                for (;;) {
                    menu_frame_begin();
                    func_80027ea0();
                    menu_draw_window(2, 3, cursor, confirm);
                    func_8002ac34();
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
        input = pad_read(1);
        if ((input & 0x1000) != 0 && (prev & 0x1000) == 0) {
            menu_play_input_sound(0);
            if (cursor != 0)
                cursor--;
            else
                cursor = 2;
        } else if ((input & 0x4000) != 0 && (prev & 0x4000) == 0) {
            menu_play_input_sound(0);
            if (cursor != 2)
                cursor++;
            else
                cursor = 0;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            if (cursor == 2) {
                result = -1;
            } else {
                confirm = 1;
                action = cursor;
            }
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            result = -1;
        }

        menu_frame_begin();
        menu_draw_window(2, 3, cursor, confirm);
        func_8002ac34();
    }
}
