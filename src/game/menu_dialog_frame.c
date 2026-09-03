#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

extern POLY_FT4 *current_poly_ft4;

extern void menu_format_number(s32 value, s32 width, s32 flag, u16 *out);

/* Font-atlas / number-atlas sprite descriptors (12-byte cell descriptors). */
typedef struct MenuSpriteDef {
    u16 tpage;
    u16 clut;
    u16 u;
    u16 v;
    u16 width;
    u16 height;
} MenuSpriteDef;

/* Positioned glyph run: origin followed by codes terminated by -1. */
typedef struct MenuGlyphString {
    u16 x;
    u16 y;
    s16 codes[10];
} MenuGlyphString;

extern void menu_draw_string(const MenuSpriteDef *font, const MenuGlyphString *str);

/* Shared menu font atlas and number atlas. */
extern MenuSpriteDef DAT_800583f4;
extern MenuSpriteDef DAT_800583e8;

/*
 * Double-buffered composite dialog-frame flat quads, six per buffer.  The
 * builder links a kind-selected subset (indices 2..4) at ordering-table depth
 * 1000.  The [2][6] extent (144-byte stride) ends exactly at the number atlas
 * 0x800583e8, so the owning object's true base is 0x800582c8 -- the first
 * referenced element 0x800582f8 is index [.][2], not the base.  A naive
 * POLY_F4[2][6] anchored at 0x800582f8 would overrun into the atlas; this
 * anchoring keeps every global's extent disjoint.
 */

/* One save-slot summary row: a 6-digit field, a 1-digit field, and two
 * value/max pairs (separated by the "/" glyph).  field_8 gates the row. */
typedef struct MenuSlotStats {
    s32 field_0;
    s32 field_4;
    s32 field_8;
    s32 field_c;
    s32 field_10;
    s32 field_14;
} MenuSlotStats;

/*
 * Draw a bordered menu dialog frame plus optional save-slot summary rows.
 * `kind` selects which of the composite-frame quads (indices 2..4 of the
 * double-buffered flat-quad array) border the box.  When `rows` is non-null,
 * up to three slot summaries are drawn, each gated by its field_8 being
 * positive; every row prints an icon label plus its numeric fields through the
 * shared glyph-string workspace.  Used by the save/load panels, the save
 * confirmation, and the two-option confirm dialog.
 *
 * Residue: gcc257-o2 reaches consecutive frame quads and reuses the glyph
 * workspace through shared base registers where retail's schedule differs; the
 * base-sharing / list-widget class in docs/patterns/source-shapes-gcc257.md.
 */
ADDRESS(0x80027ee4, 0x49c)
void menu_draw_dialog_frame(const MenuSlotStats *rows, s32 kind)
{
    MenuGlyphString gs;
    const MenuSlotStats *row;
    s32 i;

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;

    if (kind == 0) {
        AddPrim(display_state.ordering_table + 1000,
                &DAT_800582c8[display_state.buffer_index][3]);
        AddPrim(display_state.ordering_table + 1000,
                &DAT_800582c8[display_state.buffer_index][4]);
    }
    if (kind == 1) {
        AddPrim(display_state.ordering_table + 1000,
                &DAT_800582c8[display_state.buffer_index][2]);
        AddPrim(display_state.ordering_table + 1000,
                &DAT_800582c8[display_state.buffer_index][4]);
    }
    if (kind == 2) {
        AddPrim(display_state.ordering_table + 1000,
                &DAT_800582c8[display_state.buffer_index][2]);
        AddPrim(display_state.ordering_table + 1000,
                &DAT_800582c8[display_state.buffer_index][3]);
    }
    if (kind >= 3) {
        AddPrim(display_state.ordering_table + 1000,
                &DAT_800582c8[display_state.buffer_index][2]);
        AddPrim(display_state.ordering_table + 1000,
                &DAT_800582c8[display_state.buffer_index][3]);
        AddPrim(display_state.ordering_table + 1000,
                &DAT_800582c8[display_state.buffer_index][4]);
    }

    if (rows == 0) {
        return;
    }

    row = rows;
    for (i = 0; i < 3; i++) {
        gs.y = i * 65 + 30;
        if (row->field_8 > 0) {
            gs.x = 181;
            gs.codes[0] = 0x82;
            gs.codes[1] = 0x83;
            gs.codes[2] = 0x84;
            gs.codes[3] = -1;
            menu_draw_string(&DAT_800583f4, &gs);

            gs.x = 251;
            menu_format_number(row->field_0, 6, 0, (u16 *)gs.codes);
            menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

            gs.x = 181;
            gs.codes[0] = 0xcc;
            gs.codes[1] = 0xcd;
            gs.codes[2] = -1;
            gs.y += 14;
            menu_draw_string(&DAT_800583f4, &gs);

            gs.x = 286;
            menu_format_number(row->field_4, 1, 0, (u16 *)gs.codes);
            menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

            gs.x = 181;
            gs.codes[0] = 0xf0;
            gs.codes[1] = 242;
            gs.codes[2] = -1;
            gs.y += 14;
            menu_draw_string(&DAT_800583f4, &gs);

            gs.x = 230;
            menu_format_number(row->field_8, 4, 0, (u16 *)gs.codes);
            menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

            gs.codes[0] = 11;
            gs.codes[1] = -1;
            gs.x += 28;
            menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

            gs.x += 7;
            menu_format_number(row->field_c, 4, 0, (u16 *)gs.codes);
            menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

            gs.x = 181;
            gs.codes[0] = 0xf1;
            gs.codes[1] = 242;
            gs.codes[2] = -1;
            gs.y += 14;
            menu_draw_string(&DAT_800583f4, &gs);

            gs.x = 230;
            menu_format_number(row->field_10, 4, 0, (u16 *)gs.codes);
            menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

            gs.codes[0] = 11;
            gs.codes[1] = -1;
            gs.x += 28;
            menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

            gs.x += 7;
            menu_format_number(row->field_14, 4, 0, (u16 *)gs.codes);
            menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);
        }
        row++;
    }
}
