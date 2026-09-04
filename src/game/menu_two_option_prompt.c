#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

/*
 * Interactive two-option confirm dialog.  Draws the window and the two labels
 * every frame, toggling the selected option on an up/down edge, and returns
 * once the player confirms (result -selected) or cancels (result -1).  Kinds 4
 * and 5 forward the caller's save summaries to the frame builder; all others
 * suppress the composite-frame selection (-1).
 */
ADDRESS(0x800286d4, 0x240)
s32 menu_two_option_prompt(
    s32 kind, s32 count, s32 highlight_row,
    const KfSaveSlotSummary *summaries)
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
        composite = highlight_row;

    label_a.x = 0x60;
    label_a.y = count * 20 + 44;
    label_a.codes[0] = 0x59;
    label_a.codes[1] = 0x41;
    label_a.codes[2] = -1;
    label_b.x = 0x60;
    label_b.y = count * 20 + 64;
    label_b.codes[0] = 0x41;
    label_b.codes[1] = 0x41;
    label_b.codes[2] = 0x43;
    label_b.codes[3] = -1;

    for (;;) {
        if (result != -99) {
            menu_frame_begin();
            menu_draw_dialog_frame(summaries, composite);
            menu_draw_window(kind, count, highlight_row, 1);
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
        menu_draw_dialog_frame(summaries, composite);
        menu_draw_window(kind, count, highlight_row, 1);
        menu_draw_two_option(&label_a, &label_b, selected, highlight);
        menu_present_frame();
    }
}
