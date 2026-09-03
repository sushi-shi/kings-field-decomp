#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/* Current textured-quad cursor in the shared primitive workspace. */
extern POLY_FT4 *current_poly_ft4;

/*
 * Shared translucent menu-window backdrop sprite descriptor.  The retail image
 * materializes a fresh HI16 for each field, so every field is its own curated
 * BSS global rather than one struct base register: texture page and CLUT of
 * the backdrop atlas, its top-left texel (u, v) and the tile size (w, h).
 */

/*
 * Persistent double-buffered window-border quads, four per buffer, linked at
 * ordering-table depth 3000.  Filled elsewhere (shared with the map viewer);
 * this drawer only enqueues the active buffer's copies.
 */
extern POLY_FT4 DAT_800580e8[2][4];

/*
 * Draw the shared translucent menu-window backdrop.  Four semi-transparent
 * textured tiles built in the primitive workspace cover the window interior in
 * a 2x2 grid at (166,16), (237,16), (166,120), (237,120) -- all linked at
 * ordering-table depth 2900 -- then the four persistent border quads are
 * enqueued at depth 3000.  Shared by the status panel, the option-window
 * renderer and the scrollable list widget.
 *
 * Residue: gcc257-o2 keeps a single saved-register base anchored on the sprite
 * descriptor (0x80058400) and reaches the border-quad array through negative
 * offsets from it, where retail's register schedule differs slightly; the
 * u16-modular constant/base-sharing class documented in
 * docs/patterns/source-shapes-gcc257.md.
 */
ADDRESS(0x8002a510, 0x6a4)
void menu_draw_window_backdrop(void)
{
    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = DAT_80058400;
    current_poly_ft4->clut = DAT_80058402;
    current_poly_ft4->x0 = 166;
    current_poly_ft4->y0 = 16;
    current_poly_ft4->x1 = DAT_80058408 + 166;
    current_poly_ft4->y1 = 16;
    current_poly_ft4->x2 = 166;
    current_poly_ft4->y2 = DAT_8005840a + 16;
    current_poly_ft4->x3 = DAT_80058408 + 166;
    current_poly_ft4->y3 = DAT_8005840a + 16;
    current_poly_ft4->u0 = DAT_80058404;
    current_poly_ft4->v0 = DAT_80058406;
    current_poly_ft4->u1 = DAT_80058404 + DAT_80058408;
    current_poly_ft4->v1 = DAT_80058406;
    current_poly_ft4->u2 = DAT_80058404;
    current_poly_ft4->v2 = DAT_80058406 + DAT_8005840a;
    current_poly_ft4->u3 = DAT_80058404 + DAT_80058408;
    current_poly_ft4->v3 = DAT_80058406 + DAT_8005840a;
    primitive_buffer_commit_poly_ft4(2900);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = DAT_80058400;
    current_poly_ft4->clut = DAT_80058402;
    current_poly_ft4->x0 = 237;
    current_poly_ft4->y0 = 16;
    current_poly_ft4->x1 = DAT_80058408 + 237;
    current_poly_ft4->y1 = 16;
    current_poly_ft4->x2 = 237;
    current_poly_ft4->y2 = DAT_8005840a + 16;
    current_poly_ft4->x3 = DAT_80058408 + 237;
    current_poly_ft4->y3 = DAT_8005840a + 16;
    current_poly_ft4->u0 = DAT_80058404;
    current_poly_ft4->v0 = DAT_80058406;
    current_poly_ft4->u1 = DAT_80058404 + DAT_80058408;
    current_poly_ft4->v1 = DAT_80058406;
    current_poly_ft4->u2 = DAT_80058404;
    current_poly_ft4->v2 = DAT_80058406 + DAT_8005840a;
    current_poly_ft4->u3 = DAT_80058404 + DAT_80058408;
    current_poly_ft4->v3 = DAT_80058406 + DAT_8005840a;
    primitive_buffer_commit_poly_ft4(2900);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = DAT_80058400;
    current_poly_ft4->clut = DAT_80058402;
    current_poly_ft4->x0 = 166;
    current_poly_ft4->y0 = 120;
    current_poly_ft4->x1 = DAT_80058408 + 166;
    current_poly_ft4->y1 = 120;
    current_poly_ft4->x2 = 166;
    current_poly_ft4->y2 = DAT_8005840a + 120;
    current_poly_ft4->x3 = DAT_80058408 + 166;
    current_poly_ft4->y3 = DAT_8005840a + 120;
    current_poly_ft4->u0 = DAT_80058404;
    current_poly_ft4->v0 = DAT_80058406;
    current_poly_ft4->u1 = DAT_80058404 + DAT_80058408;
    current_poly_ft4->v1 = DAT_80058406;
    current_poly_ft4->u2 = DAT_80058404;
    current_poly_ft4->v2 = DAT_80058406 + DAT_8005840a;
    current_poly_ft4->u3 = DAT_80058404 + DAT_80058408;
    current_poly_ft4->v3 = DAT_80058406 + DAT_8005840a;
    primitive_buffer_commit_poly_ft4(2900);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = DAT_80058400;
    current_poly_ft4->clut = DAT_80058402;
    current_poly_ft4->x0 = 237;
    current_poly_ft4->y0 = 120;
    current_poly_ft4->x1 = DAT_80058408 + 237;
    current_poly_ft4->y1 = 120;
    current_poly_ft4->x2 = 237;
    current_poly_ft4->y2 = DAT_8005840a + 120;
    current_poly_ft4->x3 = DAT_80058408 + 237;
    current_poly_ft4->y3 = DAT_8005840a + 120;
    current_poly_ft4->u0 = DAT_80058404;
    current_poly_ft4->v0 = DAT_80058406;
    current_poly_ft4->u1 = DAT_80058404 + DAT_80058408;
    current_poly_ft4->v1 = DAT_80058406;
    current_poly_ft4->u2 = DAT_80058404;
    current_poly_ft4->v2 = DAT_80058406 + DAT_8005840a;
    current_poly_ft4->u3 = DAT_80058404 + DAT_80058408;
    current_poly_ft4->v3 = DAT_80058406 + DAT_8005840a;
    primitive_buffer_commit_poly_ft4(2900);

    AddPrim(display_state.ordering_table + 3000, &DAT_800580e8[display_state.buffer_index][3]);
    AddPrim(display_state.ordering_table + 3000, &DAT_800580e8[display_state.buffer_index][2]);
    AddPrim(display_state.ordering_table + 3000, &DAT_800580e8[display_state.buffer_index][1]);
    AddPrim(display_state.ordering_table + 3000, &DAT_800580e8[display_state.buffer_index][0]);
}
