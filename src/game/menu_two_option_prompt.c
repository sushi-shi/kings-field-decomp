#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

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
KfMenuConfirmResult menu_two_option_prompt(
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
    KfMenuConfirmResult result = KF_MENU_CONFIRM_PENDING;

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
        if (result != KF_MENU_CONFIRM_PENDING) {
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
            result = KF_MENU_CONFIRM_CANCELLED;
        }

        menu_frame_begin();
        menu_draw_dialog_frame(summaries, overlay);
        menu_draw_window(kind, count, highlight_row, KF_MENU_CONFIRM_REQUESTED);
        menu_draw_two_option(&label_a, &label_b, selected, highlight);
        menu_present_frame();
    }
}
