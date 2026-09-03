#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

extern POLY_FT4 *current_poly_ft4;

/* Model draw helper (sets up the item TMD after the matrices are loaded). */
/* Decimal formatter: renders `value` right-justified in `width` glyph cells. */
extern void menu_format_number(s32 value, s32 width, s32 flag, u16 *out);

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
extern void menu_blit_sprite_translucent(const MenuSpriteDef *sprite, const void *pos);

/* Shared menu font atlas, number atlas, and the item-icon sprite descriptor. */
extern MenuSpriteDef DAT_800583f4;
extern MenuSpriteDef DAT_800583e8;
extern MenuSpriteDef DAT_80058424;

/*
 * Spinning-model Y angle for the item preview.  Modelled as the standalone
 * halfword identity the rest of the menu shares (menu_list_util resets it);
 * the RotMatrix argument is the SVECTOR that begins one halfword before it.
 */

/* Item name table (ten glyph codes per item) and the two price tables
 * (two price columns per item).  Runtime-indexed, so only the base is
 * relocated. */
extern s16 DAT_80058dc0[][10];
extern u16 DAT_800594b8[][2];
extern u16 DAT_800595f8[][2];

/* Per-item held quantity (one byte each). */
extern u8 DAT_800652a8[];

/* Icon anchor: {x, y} reused as a one-glyph label origin by the icon draw. */
extern MenuGlyphString DAT_80058c10;

/*
 * Draw the selected inventory item's detail: a spinning 3D TMD preview (built
 * from the GTE rotation and a fixed local light matrix), the item name, its
 * price for the active shop column/table, the held quantity, and the player's
 * gold.  `object` is the item id (0xff = empty slot), `col` the price column,
 * and `mode` selects the buy (0) or sell (non-zero) price table.
 *
 * Residue: the local matrix setup and the glyph-workspace stores schedule
 * their GTE/store instructions differently under gcc257-o2; the register-
 * scheduling class documented in docs/patterns/source-shapes-gcc257.md.
 */
ADDRESS(0x80027b7c, 0x2dc)
void menu_draw_item_detail(s32 object, s32 col, s32 mode)
{
    MenuGlyphString gs;
    MATRIX rot;
    MATRIX lsrc;
    MATRIX lres;
    u16 (*prices)[2];
    u16 *angle;
    s16 *name;
    s32 i;

    if (object == 0xff) {
        return;
    }

    rot.t[0] = 0x230;
    rot.t[1] = 0x8c;
    rot.t[2] = 0x5dc;
    angle = &DAT_80057b72;
    *angle = (*angle + 16) & 0xfff;
    RotMatrix((SVECTOR *)((s16 *)angle - 1), &rot);

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

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;

    gs.x = 0xae;
    gs.y = 0x24;
    name = DAT_80058dc0[object];
    for (i = 0; i < 10; i++) {
        gs.codes[i] = name[i];
    }
    menu_draw_string(&DAT_800583f4, &gs);

    gs.x = 0xc8;
    gs.y += 18;
    prices = (mode != 0) ? DAT_800595f8 : DAT_800594b8;
    menu_format_number(prices[object][col - 1], 6, 0, (u16 *)gs.codes);
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

    gs.x = 0xf2;
    gs.codes[0] = 0x1009;
    gs.codes[1] = 0x2d;
    gs.codes[2] = 0x2a;
    gs.codes[3] = 0x1013;
    gs.codes[4] = -1;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.x = 0xf2;
    gs.codes[0] = 0xca;
    gs.codes[1] = 0xcb;
    gs.codes[2] = -1;
    gs.y += 18;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.x = 0x11c;
    menu_format_number(DAT_800652a8[object], 2, 0, (u16 *)gs.codes);
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

    menu_blit_sprite_translucent(&DAT_80058424, &DAT_80058c10);
    menu_draw_string(&DAT_800583f4, &DAT_80058c10);

    gs.x = DAT_80058c10.x + 28;
    gs.y = DAT_80058c10.y;
    menu_format_number(player_state.unknown_2c, 6, 0, (u16 *)gs.codes);
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);
}
