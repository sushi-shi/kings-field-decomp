#include <kf/bool.h>
#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

enum {
    MENU_STATUS_BACKDROP_LEFT_X = 6,
    MENU_STATUS_BACKDROP_RIGHT_X = MENU_STATUS_BACKDROP_LEFT_X + MENU_BACKDROP_COLUMN_STEP
};

/* Draw one reflected tile of the status-panel backdrop. */
static inline void menu_status_draw_backdrop_quad(
    s32 x, s32 y, KfBool32 flip_x, KfBool32 flip_y, const u16 *texture_page)
{
    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = *texture_page;
    current_poly_ft4->clut = menu_assets.window_backdrop.clut;
    setXYWH(current_poly_ft4,
        x,
        y,
        menu_assets.window_backdrop.width,
        menu_assets.window_backdrop.height);
    setUV4(current_poly_ft4,
        flip_x
            ? menu_assets.window_backdrop.u + menu_assets.window_backdrop.width
            : menu_assets.window_backdrop.u,
        flip_y
            ? menu_assets.window_backdrop.v + menu_assets.window_backdrop.height
            : menu_assets.window_backdrop.v,
        flip_x
            ? menu_assets.window_backdrop.u
            : menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        flip_y
            ? menu_assets.window_backdrop.v + menu_assets.window_backdrop.height
            : menu_assets.window_backdrop.v,
        flip_x
            ? menu_assets.window_backdrop.u + menu_assets.window_backdrop.width
            : menu_assets.window_backdrop.u,
        flip_y
            ? menu_assets.window_backdrop.v
            : menu_assets.window_backdrop.v + menu_assets.window_backdrop.height,
        flip_x
            ? menu_assets.window_backdrop.u
            : menu_assets.window_backdrop.u + menu_assets.window_backdrop.width,
        flip_y
            ? menu_assets.window_backdrop.v
            : menu_assets.window_backdrop.v + menu_assets.window_backdrop.height);
    primitive_buffer_commit_poly_ft4(MENU_WINDOW_OT_DEPTH);
}

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
    const u16 *texture_page;

    frame = 0;
    texture_page = &menu_assets.window_backdrop.tpage;
    while (1) {
        menu_frame_begin();
        menu_draw_status_details();

        menu_status_draw_backdrop_quad(MENU_STATUS_BACKDROP_LEFT_X, MENU_BACKDROP_TOP_Y, KF_FALSE, KF_FALSE, texture_page);

        menu_status_draw_backdrop_quad(MENU_STATUS_BACKDROP_RIGHT_X, MENU_BACKDROP_TOP_Y, KF_TRUE, KF_FALSE, texture_page);

        menu_status_draw_backdrop_quad(MENU_STATUS_BACKDROP_LEFT_X, MENU_BACKDROP_BOTTOM_Y, KF_FALSE, KF_TRUE, texture_page);

        menu_status_draw_backdrop_quad(MENU_STATUS_BACKDROP_RIGHT_X, MENU_BACKDROP_BOTTOM_Y, KF_TRUE, KF_TRUE, texture_page);

        menu_draw_window_backdrop();
        menu_present_frame();
        if (frame < MENU_PANEL_INPUT_RELEASE_FRAME) {
            frame++;
            continue;
        }
        if (frame == MENU_PANEL_INPUT_RELEASE_FRAME) {
            while (PadRead(1) != 0) {
            }
            frame++;
            continue;
        }
        input = PadRead(1);
        if (input != 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            while (PadRead(1) != 0) {
            }
            return;
        }
    }
}
