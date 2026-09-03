#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Two menu sprite blitters that assemble one POLY_FT4 per call from a sprite
 * descriptor and a destination point.  Structure is exact; the residue is the
 * gcc257 probe materializing the reused corner-inset constants into registers
 * (`li 0xfffc; addu`) where retail keeps them as `addiu` immediates -- the
 * u16-modular constant-CSE class documented in
 * docs/patterns/source-shapes-gcc257.md.
 */

/* Current textured-quad cursor in the shared primitive workspace. */
extern POLY_FT4 *current_poly_ft4;

extern void primitive_buffer_begin_poly_ft4(void);
extern void primitive_buffer_commit_poly_ft4(s32 depth);

/*
 * Sprite descriptor read by the menu blitters: texture page and CLUT, the
 * top-left texel, and the sprite's width and height in texels.  Widths and
 * heights are halfwords for the screen-space corner arithmetic; the compiler
 * narrows the same fields to byte loads for the texel-coordinate stores.
 */
typedef struct MenuSpriteDef {
    u16 tpage;
    u16 clut;
    u16 u;
    u16 v;
    u16 width;
    u16 height;
} MenuSpriteDef;

/* Destination point on the frame. */
typedef struct MenuPoint {
    u16 x;
    u16 y;
} MenuPoint;

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
