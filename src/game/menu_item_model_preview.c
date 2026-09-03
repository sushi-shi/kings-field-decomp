#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>


/* Positioned glyph run: origin then codes terminated by -1. */
typedef struct MenuGlyphString {
    u16 x;
    u16 y;
    s16 codes[10];
} MenuGlyphString;

extern void menu_draw_string(const MenuSpriteDef *font, const MenuGlyphString *str);

/* Shared menu font atlas and number atlas descriptors. */
extern MenuSpriteDef DAT_800583f4;
extern MenuSpriteDef DAT_800583e8;

/* Item name table (ten glyph codes per item). Runtime-indexed. */
extern s16 DAT_80058dc0[][10];

/* Per-item held quantity (one byte each). */
extern u8 DAT_800652a8[];

/* Y-axis spin angle carried alongside the preview rotation SVECTOR. */
extern u16 DAT_80057b72;

/*
 * Draw a spinning 3D model preview of the inventory item plus its name and held
 * quantity.  Builds the GTE rotation from the shared spin angle and a fixed
 * local light matrix, projects the TMD, then writes the name glyph string, a
 * fixed label pair and the two-digit held count into the glyph workspace.
 * `item_id` 0xff selects the empty slot and draws nothing.
 */
ADDRESS(0x800279c4, 0x1b8)
void menu_item_model_preview(s32 item_id)
{
    MenuGlyphString gs;
    MATRIX rot;
    MATRIX lsrc;
    MATRIX lres;
    u16 *angle;
    s16 *name;
    s32 i;

    if (item_id != 0xff) {
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
        name = DAT_80058dc0[item_id];
        for (i = 0; i < 10; i++) {
            gs.codes[i] = name[i];
        }
        menu_draw_string(&DAT_800583f4, &gs);

        gs.x = 0xe6;
        gs.codes[0] = 0xca;
        gs.codes[1] = 0xcb;
        gs.codes[2] = -1;
        gs.y += 18;
        menu_draw_string(&DAT_800583f4, &gs);

        gs.x = 0x117;
        menu_format_number(DAT_800652a8[item_id], 2, 0, gs.codes);
        menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);
    }
}
