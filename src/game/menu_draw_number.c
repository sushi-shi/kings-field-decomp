#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/* Current textured-quad cursor in the shared primitive workspace. */

/*
 * Font-atlas sprite descriptor: texture page and CLUT of the glyph atlas, the
 * atlas texel origin (u, v) of the digit column, and one glyph cell's width
 * and height in texels.  Same layout as menu_draw_string's font descriptor;
 * the callers hand this in as a u16 * to the atlas record at DAT_800583e8.
 */
typedef struct MenuSpriteDef {
    u16 tpage;
    u16 clut;
    u16 u;
    u16 v;
    u16 width;
    u16 height;
} MenuSpriteDef;

/*
 * A positioned digit run: the screen origin of the first glyph followed by a
 * run of glyph codes terminated by -1.  Shares its {x, y} prefix with the menu
 * glyph string so a single scratch object feeds both drawers.
 */
typedef struct MenuGlyphString {
    u16 x;
    u16 y;
    s16 codes[1];
} MenuGlyphString;

/*
 * Render a fixed-pitch number/glyph run from a single-column font atlas.  Each
 * code selects one atlas row (v = code * 11); every quad shares the same texel
 * column (font->u) and one cell size, is advanced seven pixels per glyph on
 * screen, and links at ordering-table depth 1000.  Unlike menu_draw_string the
 * texel origin comes from the descriptor, not the code, and there are no
 * decoration overlays.  The callers cast the atlas record and the scratch
 * label object to the declared raw pointer parameters.
 *
 * Residue: one instruction only.  gcc257-o2 (-mcpu=r2000) hoists the
 * dependency-free `xoff = 0` init (`move s3,zero`) into the register-save
 * prologue, where retail leaves it in the loop preheader after the first
 * codes[] guard test.  An unattributed second-pass scheduling choice; every
 * field store, the per-vertex codes[] reload/multiply, register allocation and
 * all delay slots match retail (objdiff-100 masked, 99.3% raw).
 */
ADDRESS(0x8002a310, 0x200)
void menu_draw_number(u16 *atlas, s16 *str)
{
    const MenuSpriteDef *font = (const MenuSpriteDef *)atlas;
    const MenuGlyphString *label = (const MenuGlyphString *)str;
    s32 i;
    s32 xoff;

    for (i = 0, xoff = 0; label->codes[i] != -1; i++, xoff += 7) {
        primitive_buffer_begin_poly_ft4();
        current_poly_ft4->tpage = font->tpage;
        current_poly_ft4->clut = font->clut;
        current_poly_ft4->x0 = label->x + xoff;
        current_poly_ft4->y0 = label->y;
        current_poly_ft4->x1 = label->x + xoff + font->width;
        current_poly_ft4->y1 = label->y;
        current_poly_ft4->x2 = label->x + xoff;
        current_poly_ft4->y2 = label->y + font->height;
        current_poly_ft4->x3 = label->x + xoff + font->width;
        current_poly_ft4->y3 = label->y + font->height;
        current_poly_ft4->u0 = font->u;
        current_poly_ft4->v0 = label->codes[i] * 11;
        current_poly_ft4->u1 = font->u + font->width;
        current_poly_ft4->v1 = label->codes[i] * 11;
        current_poly_ft4->u2 = font->u;
        current_poly_ft4->v2 = label->codes[i] * 11 + font->height;
        current_poly_ft4->u3 = font->u + font->width;
        current_poly_ft4->v3 = label->codes[i] * 11 + font->height;
        primitive_buffer_commit_poly_ft4(1000);
    }
}
