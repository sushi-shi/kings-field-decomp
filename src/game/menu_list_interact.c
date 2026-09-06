#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

/* Positioned option label: origin then glyph codes terminated by -1. */
typedef struct MenuOption {
    u16 x;
    u16 y;
    s16 codes[4];
} MenuOption;

/*
 * Drive an interactive scrollable list with a two-option confirm footer.
 * `list` is the list widget context, `kind` selects the pair of footer labels,
 * `mode` selects the side preview redrawn each frame (0 = 3D item model, 1 =
 * item detail panel, 2 = map marker), and `item_id` / `arg4` / `arg5` feed that
 * preview.  Returns -selected_row on the up/confirm edge, -1 on cancel.
 */
ADDRESS(0x80028380, 0x354)
s32 menu_list_interact(const KfMenuList *list, s32 kind, s32 mode,
                       s32 item_id, u32 arg4, u32 arg5)
{
    MenuOption opt0;
    MenuOption opt1;
    s32 selected;
    u32 highlight;
    u32 pad;
    u32 prev_pad;
    s32 result;

    selected = 0;
    highlight = 0;
    prev_pad = 0;
    result = -99;
    while (PadRead(1) != 0) {
    }

    opt0.x = 0x60;
    opt0.y = 0xb9;
    opt1.x = 0x60;
    opt1.y = 0xcd;
    if (kind == 0) {
        opt0.codes[0] = 0x72;
        opt0.codes[1] = 0x42;
    } else if (kind == 1) {
        opt0.codes[0] = 0x75;
        opt0.codes[1] = 0x52;
        opt0.codes[2] = 0x6a;
        opt0.codes[3] = MENU_TEXT_END;
        goto opt0_done;
    } else if (kind == 2) {
        opt0.codes[0] = 0x59;
        opt0.codes[1] = 0x41;
    } else if (kind == 3) {
        opt0.codes[0] = 0x74;
        opt0.codes[1] = 0x42;
    } else if (kind == 4) {
        opt0.codes[0] = 0x73;
        opt0.codes[1] = 0x6a;
    } else {
        opt0.codes[0] = 0x70;
        opt0.codes[1] = 0x71;
    }
    opt0.codes[2] = MENU_TEXT_END;
opt0_done:
    if (kind == 2) {
        opt1.codes[0] = 0x41;
        opt1.codes[1] = 0x41;
        opt1.codes[2] = 0x43;
    } else {
        opt1.codes[0] = 99;
        opt1.codes[1] = 0x61;
        opt1.codes[2] = 0x6a;
    }
    opt1.codes[3] = MENU_TEXT_END;

    menu_frame_begin();
    do {
        if (mode == 0) {
            menu_item_model_preview(item_id);
        } else if (mode == 1) {
            menu_draw_item_detail(item_id, arg4, arg5);
        } else if (mode == 2 && item_id != KF_MAGIC_NONE) {
            menu_add_marker_quad();
        }
        menu_list_render(list);
        menu_draw_two_option(
            (const MenuGlyphString *)&opt0,
            (const MenuGlyphString *)&opt1,
            selected, highlight);
        menu_present_frame();

        if (result != -99) {
            menu_frame_begin();
            if (mode == 0) {
                menu_item_model_preview(item_id);
            } else if (mode == 1) {
                menu_draw_item_detail(item_id, arg4, arg5);
            } else if (mode == 2 && item_id != KF_MAGIC_NONE) {
                menu_add_marker_quad();
            }
            menu_list_render(list);
            menu_draw_two_option(
                (const MenuGlyphString *)&opt0,
                (const MenuGlyphString *)&opt1,
                selected, highlight);
            menu_present_frame();
            while (PadRead(1) != 0) {
            }
            return result;
        }

        highlight = 0;
        menu_frame_begin();
        pad = PadRead(1);
        if (((pad & PADLup) == 0 || (prev_pad & PADLup) != 0) &&
            ((pad & PADLdown) == 0 || (prev_pad & PADLdown) != 0)) {
            if ((pad & PADRright) == 0 || (prev_pad & PADRright) != 0) {
                if ((pad & PADRdown) != 0 && (prev_pad & PADRdown) == 0) {
                    menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
                    result = -1;
                }
            } else {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                highlight = 1;
                result = -selected;
            }
        } else {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            selected = (selected == 0);
        }
        prev_pad = pad;
    } while (1);
}
