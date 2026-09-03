#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/* Current textured-quad cursor in the shared primitive workspace. */
extern POLY_FT4 *current_poly_ft4;

/*
 * Font-atlas sprite descriptor: texture page and CLUT of the glyph atlas plus
 * one glyph cell's width and height in texels.  Same layout as the menu
 * blitters' sprite descriptor; only the atlas page/clut and the cell size are
 * consumed here because the per-glyph texel origin is derived from the code.
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
 * A positioned label: the screen origin of the first glyph followed by a run
 * of glyph codes terminated by -1.  Each code carries the atlas glyph index in
 * its low 12 bits (16 cells per atlas row, 14x12 texel cells) and two optional
 * decoration bits (0x1000, 0x2000) that overlay two fixed atlas cells on the
 * same quad.  Shares its {x, y} prefix with the menu blitters' point, so the
 * two-option dispatcher hands the same object to both the blitters and here.
 */
typedef struct MenuGlyphString {
    u16 x;
    u16 y;
    s16 codes[1];
} MenuGlyphString;

/*
 * Render a positioned glyph string from a font atlas.  Each glyph becomes one
 * opaque POLY_FT4 whose texel origin is (code%16)*14, (code/16)*12 and whose
 * screen box is one atlas cell advanced 14 pixels per glyph.  The 0x1000 and
 * 0x2000 code bits additionally overlay the two atlas cells at row 2, columns
 * 14 and 15, used for the diacritic/decoration marks.  All quads link at
 * ordering-table depth 1000.
 *
 * Residue: the decoration cells' `u = <196|210> + width` byte stores compile to
 * `addiu ...,196|210` where retail carries the equal-low-byte `addiu ...,-60|-46`
 * form.  Identical value, gcc257-o2 immediate-representation choice only (the
 * u16-modular constant class); a (u8) cast does not flip it.
 */
ADDRESS(0x80029de0, 0x530)
void menu_draw_string(const MenuSpriteDef *font, const MenuGlyphString *str)
{
    s32 i;
    s32 xoff;

    for (i = 0, xoff = 0; str->codes[i] != -1; i++, xoff += 14) {
        s32 glyph;

        primitive_buffer_begin_poly_ft4();
        current_poly_ft4->tpage = font->tpage;
        current_poly_ft4->clut = font->clut;
        current_poly_ft4->x0 = str->x + xoff;
        current_poly_ft4->y0 = str->y;
        current_poly_ft4->x1 = str->x + xoff + font->width;
        current_poly_ft4->y1 = str->y;
        current_poly_ft4->x2 = str->x + xoff;
        current_poly_ft4->y2 = str->y + font->height;
        current_poly_ft4->x3 = str->x + xoff + font->width;
        current_poly_ft4->y3 = str->y + font->height;
        glyph = str->codes[i] & 0xfff;
        current_poly_ft4->u0 = (glyph % 16) * 14;
        current_poly_ft4->v0 = (glyph / 16) * 12;
        current_poly_ft4->u1 = (glyph % 16) * 14 + font->width;
        current_poly_ft4->v1 = (glyph / 16) * 12;
        current_poly_ft4->u2 = (glyph % 16) * 14;
        current_poly_ft4->v2 = (glyph / 16) * 12 + font->height;
        current_poly_ft4->u3 = (glyph % 16) * 14 + font->width;
        current_poly_ft4->v3 = (glyph / 16) * 12 + font->height;
        primitive_buffer_commit_poly_ft4(1000);

        if (str->codes[i] & 0x1000) {
            primitive_buffer_begin_poly_ft4();
            current_poly_ft4->tpage = font->tpage;
            current_poly_ft4->clut = font->clut;
            current_poly_ft4->x0 = str->x + xoff;
            current_poly_ft4->y0 = str->y;
            current_poly_ft4->x1 = str->x + xoff + font->width;
            current_poly_ft4->y1 = str->y;
            current_poly_ft4->x2 = str->x + xoff;
            current_poly_ft4->y2 = str->y + font->height;
            current_poly_ft4->x3 = str->x + xoff + font->width;
            current_poly_ft4->y3 = str->y + font->height;
            current_poly_ft4->u0 = 196;
            current_poly_ft4->v0 = 24;
            current_poly_ft4->u1 = 196 + font->width;
            current_poly_ft4->v1 = 24;
            current_poly_ft4->u2 = 196;
            current_poly_ft4->v2 = 24 + font->height;
            current_poly_ft4->u3 = 196 + font->width;
            current_poly_ft4->v3 = 24 + font->height;
            primitive_buffer_commit_poly_ft4(1000);
        }

        if (str->codes[i] & 0x2000) {
            primitive_buffer_begin_poly_ft4();
            current_poly_ft4->tpage = font->tpage;
            current_poly_ft4->clut = font->clut;
            current_poly_ft4->x0 = str->x + xoff;
            current_poly_ft4->y0 = str->y;
            current_poly_ft4->x1 = str->x + xoff + font->width;
            current_poly_ft4->y1 = str->y;
            current_poly_ft4->x2 = str->x + xoff;
            current_poly_ft4->y2 = str->y + font->height;
            current_poly_ft4->x3 = str->x + xoff + font->width;
            current_poly_ft4->y3 = str->y + font->height;
            current_poly_ft4->u0 = 210;
            current_poly_ft4->v0 = 24;
            current_poly_ft4->u1 = 210 + font->width;
            current_poly_ft4->v1 = 24;
            current_poly_ft4->u2 = 210;
            current_poly_ft4->v2 = 24 + font->height;
            current_poly_ft4->u3 = 210 + font->width;
            current_poly_ft4->v3 = 24 + font->height;
            primitive_buffer_commit_poly_ft4(1000);
        }
    }
}
