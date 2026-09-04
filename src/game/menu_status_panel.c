#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>


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

    frame = 0;
    do {
        while (1) {
            menu_frame_begin();
            menu_draw_status_details();

            primitive_buffer_begin_poly_ft4();
            SetSemiTrans(current_poly_ft4, 1);
            current_poly_ft4->tpage = DAT_80058400;
            current_poly_ft4->clut = DAT_80058402;
            current_poly_ft4->x0 = 6;
            current_poly_ft4->y0 = 0x10;
            current_poly_ft4->y1 = 0x10;
            current_poly_ft4->x2 = 6;
            current_poly_ft4->x1 = DAT_80058408 + 6;
            current_poly_ft4->y2 = DAT_8005840a + 0x10;
            current_poly_ft4->x3 = DAT_80058408 + 6;
            current_poly_ft4->y3 = DAT_8005840a + 0x10;
            current_poly_ft4->u0 = DAT_80058404;
            current_poly_ft4->v0 = DAT_80058406;
            current_poly_ft4->u1 = DAT_80058404 + DAT_80058408;
            current_poly_ft4->v1 = DAT_80058406;
            current_poly_ft4->u2 = DAT_80058404;
            current_poly_ft4->v2 = DAT_80058406 + DAT_8005840a;
            current_poly_ft4->u3 = DAT_80058404 + DAT_80058408;
            current_poly_ft4->v3 = DAT_80058406 + DAT_8005840a;
            primitive_buffer_commit_poly_ft4(0xb54);

            primitive_buffer_begin_poly_ft4();
            SetSemiTrans(current_poly_ft4, 1);
            current_poly_ft4->tpage = DAT_80058400;
            current_poly_ft4->clut = DAT_80058402;
            current_poly_ft4->x0 = 0x4d;
            current_poly_ft4->y0 = 0x10;
            current_poly_ft4->y1 = 0x10;
            current_poly_ft4->x2 = 0x4d;
            current_poly_ft4->x1 = DAT_80058408 + 0x4d;
            current_poly_ft4->y2 = DAT_8005840a + 0x10;
            current_poly_ft4->x3 = DAT_80058408 + 0x4d;
            current_poly_ft4->y3 = DAT_8005840a + 0x10;
            current_poly_ft4->u0 = DAT_80058404 + DAT_80058408;
            current_poly_ft4->v0 = DAT_80058406;
            current_poly_ft4->u1 = DAT_80058404;
            current_poly_ft4->v1 = DAT_80058406;
            current_poly_ft4->u2 = DAT_80058404 + DAT_80058408;
            current_poly_ft4->v2 = DAT_80058406 + DAT_8005840a;
            current_poly_ft4->u3 = DAT_80058404;
            current_poly_ft4->v3 = DAT_80058406 + DAT_8005840a;
            primitive_buffer_commit_poly_ft4(0xb54);

            primitive_buffer_begin_poly_ft4();
            SetSemiTrans(current_poly_ft4, 1);
            current_poly_ft4->tpage = DAT_80058400;
            current_poly_ft4->clut = DAT_80058402;
            current_poly_ft4->x0 = 6;
            current_poly_ft4->y0 = 0x78;
            current_poly_ft4->y1 = 0x78;
            current_poly_ft4->x2 = 6;
            current_poly_ft4->x1 = DAT_80058408 + 6;
            current_poly_ft4->y2 = DAT_8005840a + 0x78;
            current_poly_ft4->x3 = DAT_80058408 + 6;
            current_poly_ft4->y3 = DAT_8005840a + 0x78;
            current_poly_ft4->u0 = DAT_80058404;
            current_poly_ft4->v0 = DAT_80058406 + DAT_8005840a;
            current_poly_ft4->u1 = DAT_80058404 + DAT_80058408;
            current_poly_ft4->v1 = DAT_80058406 + DAT_8005840a;
            current_poly_ft4->u2 = DAT_80058404;
            current_poly_ft4->v2 = DAT_80058406;
            current_poly_ft4->u3 = DAT_80058404 + DAT_80058408;
            current_poly_ft4->v3 = DAT_80058406;
            primitive_buffer_commit_poly_ft4(0xb54);

            primitive_buffer_begin_poly_ft4();
            SetSemiTrans(current_poly_ft4, 1);
            current_poly_ft4->tpage = DAT_80058400;
            current_poly_ft4->clut = DAT_80058402;
            current_poly_ft4->x0 = 0x4d;
            current_poly_ft4->y0 = 0x78;
            current_poly_ft4->y1 = 0x78;
            current_poly_ft4->x2 = 0x4d;
            current_poly_ft4->x1 = DAT_80058408 + 0x4d;
            current_poly_ft4->y2 = DAT_8005840a + 0x78;
            current_poly_ft4->x3 = DAT_80058408 + 0x4d;
            current_poly_ft4->y3 = DAT_8005840a + 0x78;
            current_poly_ft4->u0 = DAT_80058404 + DAT_80058408;
            current_poly_ft4->v0 = DAT_80058406 + DAT_8005840a;
            current_poly_ft4->u1 = DAT_80058404;
            current_poly_ft4->v1 = DAT_80058406 + DAT_8005840a;
            current_poly_ft4->u2 = DAT_80058404 + DAT_80058408;
            current_poly_ft4->v2 = DAT_80058406;
            current_poly_ft4->u3 = DAT_80058404;
            current_poly_ft4->v3 = DAT_80058406;
            primitive_buffer_commit_poly_ft4(0xb54);

            menu_draw_window_backdrop();
            menu_present_frame();
            if (frame > 1) {
                break;
            }
        advance:
            frame++;
        }

        if (frame == 2) {
            while (pad_read(1) != 0) {
            }
            goto advance;
        }
        input = pad_read(1);
        if (input != 0) {
            menu_play_input_sound(2);
            while (pad_read(1) != 0) {
            }
            return;
        }
    } while (1);
}
