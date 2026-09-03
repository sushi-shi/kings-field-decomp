#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Menu sprite and glyph blitters that each assemble POLY_FT4 primitives from a
 * shared sprite descriptor and a destination point: two single-quad blitters
 * (translucent and opaque) and the positioned glyph-string renderer. They form
 * one contiguous run (0x80029ab0..0x8002a310) over the shared MenuSpriteDef and
 * current_poly_ft4 cursor. Module boundary is WIP.
 *
 * Structure is exact; the residue is the gcc257 probe materializing the reused
 * corner-inset constants into registers (`li 0xfffc; addu`) where retail keeps
 * them as `addiu` immediates -- the u16-modular constant-CSE class documented
 * in docs/patterns/source-shapes-gcc257.md.
 */

/* Current textured-quad cursor in the shared primitive workspace. */

/*
 * Sprite descriptor read by the menu blitters: texture page and CLUT, the
 * top-left texel, and the sprite's width and height in texels.  Widths and
 * heights are halfwords for the screen-space corner arithmetic; the compiler
 * narrows the same fields to byte loads for the texel-coordinate stores. The
 * glyph renderer consumes the same layout as a font atlas descriptor, taking
 * the atlas page/clut and the glyph cell size.
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
 * Translucent menu sprite blit: build a semi-transparent textured quad for the
 * sprite at the destination point, insetting the corners by (4, 3), and link
 * it at ordering-table depth 2000.
 */
ADDRESS(0x80029ab0, 0x1a0)
void menu_blit_sprite_translucent(const MenuSpriteDef *sprite, const MenuPoint *pos)
{
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = sprite->tpage;
    current_poly_ft4->clut = sprite->clut;
    current_poly_ft4->x0 = pos->x - 4;
    current_poly_ft4->y0 = pos->y - 3;
    current_poly_ft4->x1 = pos->x + (sprite->width - 4);
    current_poly_ft4->y1 = pos->y - 3;
    current_poly_ft4->x2 = pos->x - 4;
    current_poly_ft4->y2 = pos->y + (sprite->height - 3);
    current_poly_ft4->x3 = pos->x + (sprite->width - 4);
    current_poly_ft4->y3 = pos->y + (sprite->height - 3);
    current_poly_ft4->u0 = sprite->u;
    current_poly_ft4->v0 = sprite->v;
    current_poly_ft4->u1 = sprite->u + sprite->width;
    current_poly_ft4->v1 = sprite->v;
    current_poly_ft4->u2 = sprite->u;
    current_poly_ft4->v2 = sprite->v + sprite->height;
    current_poly_ft4->u3 = sprite->u + sprite->width;
    current_poly_ft4->v3 = sprite->v + sprite->height;
    SetSemiTrans(current_poly_ft4, 1);
    primitive_buffer_commit_poly_ft4(2000);
}

/*
 * Opaque menu sprite blit: same quad build for the sprite at the destination
 * point but insetting the corners by (18, 2), linked at ordering-table depth
 * 2000.
 */
ADDRESS(0x80029c50, 0x190)
void menu_blit_sprite(const MenuSpriteDef *sprite, const MenuPoint *pos)
{
    primitive_buffer_begin_poly_ft4();
    current_poly_ft4->tpage = sprite->tpage;
    current_poly_ft4->clut = sprite->clut;
    current_poly_ft4->x0 = pos->x - 18;
    current_poly_ft4->y0 = pos->y - 2;
    current_poly_ft4->x1 = pos->x + (sprite->width - 18);
    current_poly_ft4->y1 = pos->y - 2;
    current_poly_ft4->x2 = pos->x - 18;
    current_poly_ft4->y2 = pos->y + (sprite->height - 2);
    current_poly_ft4->x3 = pos->x + (sprite->width - 18);
    current_poly_ft4->y3 = pos->y + (sprite->height - 2);
    current_poly_ft4->u0 = sprite->u;
    current_poly_ft4->v0 = sprite->v;
    current_poly_ft4->u1 = sprite->u + sprite->width;
    current_poly_ft4->v1 = sprite->v;
    current_poly_ft4->u2 = sprite->u;
    current_poly_ft4->v2 = sprite->v + sprite->height;
    current_poly_ft4->u3 = sprite->u + sprite->width;
    current_poly_ft4->v3 = sprite->v + sprite->height;
    primitive_buffer_commit_poly_ft4(2000);
}

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
