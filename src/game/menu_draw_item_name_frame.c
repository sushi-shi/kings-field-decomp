#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

extern POLY_FT4 *current_poly_ft4;

/* Sprite / atlas cell descriptor (texture page, CLUT, texel origin, size). */
typedef struct MenuSpriteDef {
    u16 tpage;
    u16 clut;
    u16 u;
    u16 v;
    u16 width;
    u16 height;
} MenuSpriteDef;

/* Positioned glyph run: origin then codes terminated by -1. */
typedef struct MenuGlyphString {
    u16 x;
    u16 y;
    s16 codes[10];
} MenuGlyphString;

extern void menu_draw_string(const MenuSpriteDef *font, const MenuGlyphString *str);

extern MenuSpriteDef DAT_800583f4;
extern s16 DAT_80058dc0[][10];
extern u16 DAT_80057b72;

/*
 * Draw the shop preview cell: a spinning 3D model of the selected item, its
 * name row, and the four mirrored window-sprite quads that frame it (a 2x2 grid
 * whose texel corners flip per quadrant).  `param_1` is the item id.
 */
ADDRESS(0x800292f8, 0x7b8)
void menu_draw_item_name_frame(s32 param_1)
{
    MenuGlyphString gs;
    MATRIX rot;
    MATRIX lsrc;
    MATRIX lres;
    s16 *name;
    s32 i;

    rot.t[0] = 0xdc;
    rot.t[1] = 0x8c;
    rot.t[2] = 0x5dc;
    DAT_80057b72 = (DAT_80057b72 + 8) & 0xfff;
    RotMatrix((SVECTOR *)((s16 *)&DAT_80057b72 - 1), &rot);

    lsrc.m[0][0] = -4096;
    lsrc.m[0][1] = -4096;
    lsrc.m[0][2] = -4096;
    lsrc.m[1][0] = -4096;
    lsrc.m[1][1] = -4096;
    lsrc.m[1][2] = -4096;
    lsrc.m[2][0] = 0;
    lsrc.m[2][1] = 0;
    lsrc.m[2][2] = 0;
    MulMatrix0(&lsrc, &rot, &lres);
    SetLightMatrix(&lres);
    SetRotMatrix(&rot);
    SetTransMatrix(&rot);
    func_8001ed38();

    name = DAT_80058dc0[param_1];
    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    gs.x = 0x80;
    gs.y = 0x24;
    for (i = 0; i < 10; i++) {
        gs.codes[i] = name[i];
    }
    menu_draw_string(&DAT_800583f4, &gs);

    primitive_buffer_begin_poly_ft4();
    SetSemiTrans(current_poly_ft4, 1);
    current_poly_ft4->tpage = DAT_80058400;
    current_poly_ft4->clut = DAT_80058402;
    current_poly_ft4->x0 = 0x76;
    current_poly_ft4->y0 = 0x10;
    current_poly_ft4->y1 = 0x10;
    current_poly_ft4->x2 = 0x76;
    current_poly_ft4->x1 = DAT_80058408 + 0x76;
    current_poly_ft4->y2 = DAT_8005840a + 0x10;
    current_poly_ft4->x3 = DAT_80058408 + 0x76;
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
    current_poly_ft4->x0 = 0xbd;
    current_poly_ft4->y0 = 0x10;
    current_poly_ft4->y1 = 0x10;
    current_poly_ft4->x2 = 0xbd;
    current_poly_ft4->x1 = DAT_80058408 + 0xbd;
    current_poly_ft4->y2 = DAT_8005840a + 0x10;
    current_poly_ft4->x3 = DAT_80058408 + 0xbd;
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
    current_poly_ft4->x0 = 0x76;
    current_poly_ft4->y0 = 0x78;
    current_poly_ft4->y1 = 0x78;
    current_poly_ft4->x2 = 0x76;
    current_poly_ft4->x1 = DAT_80058408 + 0x76;
    current_poly_ft4->y2 = DAT_8005840a + 0x78;
    current_poly_ft4->x3 = DAT_80058408 + 0x76;
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
    current_poly_ft4->x0 = 0xbd;
    current_poly_ft4->y0 = 0x78;
    current_poly_ft4->y1 = 0x78;
    current_poly_ft4->x2 = 0xbd;
    current_poly_ft4->x1 = DAT_80058408 + 0xbd;
    current_poly_ft4->y2 = DAT_8005840a + 0x78;
    current_poly_ft4->x3 = DAT_80058408 + 0xbd;
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
}
