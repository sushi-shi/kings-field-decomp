#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

extern void menu_draw_dialog_frame(void *arg0, s32 arg1);
extern void menu_draw_window(s32 object, s32 arg1, s32 arg2, s32 arg3);
extern u32 pad_read();

/*
 * Interactive two-option confirm dialog.  Draws the window and the two labels
 * every frame, toggling the selected option on an up/down edge, and returns
 * once the player confirms (result -selected) or cancels (result -1).  Kinds 4
 * and 5 forward the caller's composite argument to the frame builder; all
 * others suppress it (-1).
 */
ADDRESS(0x800286d4, 0x240)
s32 menu_two_option_prompt(s32 kind, s32 menu_id, s32 arg2, void *arg3)
{
    MenuGlyphString label_a;
    MenuGlyphString label_b;
    s32 selected = 0;
    s32 highlight = 0;
    s32 composite = -1;
    s32 input = 0;
    s32 prev;
    s32 result = -99;

    while (pad_read(1) != 0)
        ;

    if ((u32)(kind - 4) < 2)
        composite = arg2;

    label_a.x = 0x60;
    label_a.y = menu_id * 20 + 44;
    label_a.codes[0] = 0x59;
    label_a.codes[1] = 0x41;
    label_a.codes[2] = -1;
    label_b.x = 0x60;
    label_b.y = menu_id * 20 + 64;
    label_b.codes[0] = 0x41;
    label_b.codes[1] = 0x41;
    label_b.codes[2] = 0x43;
    label_b.codes[3] = -1;

    for (;;) {
        if (result != -99) {
            menu_frame_begin();
            menu_draw_dialog_frame(arg3, composite);
            menu_draw_window(kind, menu_id, arg2, 1);
            menu_draw_two_option(&label_a, &label_b, selected, highlight);
            menu_present_frame();
            while (pad_read(1) != 0)
                ;
            return result;
        }

        highlight = 0;
        prev = input;
        input = pad_read(1);
        if (((input & 0x1000) != 0 && (prev & 0x1000) == 0) ||
            ((input & 0x4000) != 0 && (prev & 0x4000) == 0)) {
            menu_play_input_sound(0);
            if (selected)
                selected = 0;
            else
                selected = 1;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            highlight = 1;
            result = -selected;
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            result = -1;
        }

        menu_frame_begin();
        menu_draw_dialog_frame(arg3, composite);
        menu_draw_window(kind, menu_id, arg2, 1);
        menu_draw_two_option(&label_a, &label_b, selected, highlight);
        menu_present_frame();
    }
}
