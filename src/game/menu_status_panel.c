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
            current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
            current_poly_ft4->clut = menu_assets.window_backdrop.clut;
            current_poly_ft4->x0 = 6;
            current_poly_ft4->y0 = 0x10;
            current_poly_ft4->y1 = 0x10;
            current_poly_ft4->x2 = 6;
            current_poly_ft4->x1 = menu_assets.window_backdrop.width + 6;
            current_poly_ft4->y2 = menu_assets.window_backdrop.height + 0x10;
            current_poly_ft4->x3 = menu_assets.window_backdrop.width + 6;
            current_poly_ft4->y3 = menu_assets.window_backdrop.height + 0x10;
            current_poly_ft4->u0 = menu_assets.window_backdrop.u;
            current_poly_ft4->v0 = menu_assets.window_backdrop.v;
            current_poly_ft4->u1 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
            current_poly_ft4->v1 = menu_assets.window_backdrop.v;
            current_poly_ft4->u2 = menu_assets.window_backdrop.u;
            current_poly_ft4->v2 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
            current_poly_ft4->u3 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
            current_poly_ft4->v3 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
            primitive_buffer_commit_poly_ft4(0xb54);

            primitive_buffer_begin_poly_ft4();
            SetSemiTrans(current_poly_ft4, 1);
            current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
            current_poly_ft4->clut = menu_assets.window_backdrop.clut;
            current_poly_ft4->x0 = 0x4d;
            current_poly_ft4->y0 = 0x10;
            current_poly_ft4->y1 = 0x10;
            current_poly_ft4->x2 = 0x4d;
            current_poly_ft4->x1 = menu_assets.window_backdrop.width + 0x4d;
            current_poly_ft4->y2 = menu_assets.window_backdrop.height + 0x10;
            current_poly_ft4->x3 = menu_assets.window_backdrop.width + 0x4d;
            current_poly_ft4->y3 = menu_assets.window_backdrop.height + 0x10;
            current_poly_ft4->u0 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
            current_poly_ft4->v0 = menu_assets.window_backdrop.v;
            current_poly_ft4->u1 = menu_assets.window_backdrop.u;
            current_poly_ft4->v1 = menu_assets.window_backdrop.v;
            current_poly_ft4->u2 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
            current_poly_ft4->v2 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
            current_poly_ft4->u3 = menu_assets.window_backdrop.u;
            current_poly_ft4->v3 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
            primitive_buffer_commit_poly_ft4(0xb54);

            primitive_buffer_begin_poly_ft4();
            SetSemiTrans(current_poly_ft4, 1);
            current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
            current_poly_ft4->clut = menu_assets.window_backdrop.clut;
            current_poly_ft4->x0 = 6;
            current_poly_ft4->y0 = 0x78;
            current_poly_ft4->y1 = 0x78;
            current_poly_ft4->x2 = 6;
            current_poly_ft4->x1 = menu_assets.window_backdrop.width + 6;
            current_poly_ft4->y2 = menu_assets.window_backdrop.height + 0x78;
            current_poly_ft4->x3 = menu_assets.window_backdrop.width + 6;
            current_poly_ft4->y3 = menu_assets.window_backdrop.height + 0x78;
            current_poly_ft4->u0 = menu_assets.window_backdrop.u;
            current_poly_ft4->v0 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
            current_poly_ft4->u1 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
            current_poly_ft4->v1 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
            current_poly_ft4->u2 = menu_assets.window_backdrop.u;
            current_poly_ft4->v2 = menu_assets.window_backdrop.v;
            current_poly_ft4->u3 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
            current_poly_ft4->v3 = menu_assets.window_backdrop.v;
            primitive_buffer_commit_poly_ft4(0xb54);

            primitive_buffer_begin_poly_ft4();
            SetSemiTrans(current_poly_ft4, 1);
            current_poly_ft4->tpage = menu_assets.window_backdrop.tpage;
            current_poly_ft4->clut = menu_assets.window_backdrop.clut;
            current_poly_ft4->x0 = 0x4d;
            current_poly_ft4->y0 = 0x78;
            current_poly_ft4->y1 = 0x78;
            current_poly_ft4->x2 = 0x4d;
            current_poly_ft4->x1 = menu_assets.window_backdrop.width + 0x4d;
            current_poly_ft4->y2 = menu_assets.window_backdrop.height + 0x78;
            current_poly_ft4->x3 = menu_assets.window_backdrop.width + 0x4d;
            current_poly_ft4->y3 = menu_assets.window_backdrop.height + 0x78;
            current_poly_ft4->u0 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
            current_poly_ft4->v0 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
            current_poly_ft4->u1 = menu_assets.window_backdrop.u;
            current_poly_ft4->v1 = menu_assets.window_backdrop.v + menu_assets.window_backdrop.height;
            current_poly_ft4->u2 = menu_assets.window_backdrop.u + menu_assets.window_backdrop.width;
            current_poly_ft4->v2 = menu_assets.window_backdrop.v;
            current_poly_ft4->u3 = menu_assets.window_backdrop.u;
            current_poly_ft4->v3 = menu_assets.window_backdrop.v;
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
            while (PadRead(1) != 0) {
            }
            goto advance;
        }
        input = PadRead(1);
        if (input != 0) {
            menu_play_input_sound(2);
            while (PadRead(1) != 0) {
            }
            return;
        }
    } while (1);
}
