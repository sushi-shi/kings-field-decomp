#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Item-detail / dialog-frame menu drawing, one contiguous run
 * (0x80027b7c..0x80028380): the spinning item preview and its labels, the two
 * shared ordering-table quad linkers, and the bordered dialog frame with its
 * save-slot summary rows. All draw through the shared glyph-string workspace
 * and sprite descriptors. Module boundary is WIP.
 */
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
    menu_render_item_model();

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;

    gs.x = 0xae;
    gs.y = 0x24;
    name = item_name_rows[object].codes;
    for (i = 0; i < 10; i++) {
        gs.codes[i] = name[i];
    }
    menu_draw_string(&DAT_800583f4, &gs);

    gs.x = 0xc8;
    gs.y += 18;
    prices = (mode != 0) ? item_sell_prices : item_buy_prices;
    menu_format_number(prices[object][col - 1], 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);

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
    menu_format_number(DAT_800652a8[object], 2, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);

    menu_blit_sprite_translucent(
        &DAT_80058424, (const MenuPoint *)&DAT_80058c10);
    menu_draw_string(&DAT_800583f4, &DAT_80058c10);

    gs.x = DAT_80058c10.x + 28;
    gs.y = DAT_80058c10.y;
    menu_format_number(player_state.gold, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
}

/*
 * Two shared menu ordering-table primitives: double-buffered POLY_FT4 quads
 * that the surrounding panels prime and these helpers link into the current
 * frame's ordering table at a fixed depth.  BSS; the quad bodies are filled
 * elsewhere, so both helpers only select the active buffer's copy and enqueue
 * it.  Both are shared by the magic, list, save and load panels.
 */

/* Link the shared mid-depth menu quad at ordering-table slot 500. */
ADDRESS(0x80027e58, 0x48)
void menu_add_marker_quad(void)
{
    AddPrim(display_state.ordering_table + 500,
            &DAT_80058228[display_state.buffer_index]);
}

/* Link the shared front menu quad at ordering-table slot 0. */
ADDRESS(0x80027ea0, 0x44)
void menu_add_frame_quad(void)
{
    AddPrim(display_state.ordering_table,
            &DAT_80058278[display_state.buffer_index]);
}

/*
 * Draw a bordered menu dialog frame plus optional save-slot summary rows.
 * `kind` selects which of the composite-frame quads (indices 2..4 of the
 * double-buffered flat-quad array) border the box.  When `rows` is non-null,
 * up to three slot summaries are drawn, each gated by its third word being
 * positive; every row prints an icon label plus its numeric fields through the
 * shared glyph-string workspace.  Used by the save/load panels, the save
 * confirmation, and the two-option confirm dialog.
 *
 * Residue: gcc257-o2 reaches consecutive frame quads and reuses the glyph
 * workspace through shared base registers where retail's schedule differs; the
 * base-sharing / list-widget class in docs/patterns/source-shapes-gcc257.md.
 */
ADDRESS(0x80027ee4, 0x49c)
void menu_draw_dialog_frame(const KfSaveSlotSummary *rows, s32 kind)
{
    MenuGlyphString gs;
    const KfSaveSlotSummary *row;
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
        if ((s32)row->fields[2] > 0) {
            gs.x = 181;
            gs.codes[0] = 0x82;
            gs.codes[1] = 0x83;
            gs.codes[2] = 0x84;
            gs.codes[3] = -1;
            menu_draw_string(&DAT_800583f4, &gs);

            gs.x = 251;
            menu_format_number(row->fields[0], 6, 0, gs.codes);
            menu_draw_number(&DAT_800583e8, &gs);

            gs.x = 181;
            gs.codes[0] = 0xcc;
            gs.codes[1] = 0xcd;
            gs.codes[2] = -1;
            gs.y += 14;
            menu_draw_string(&DAT_800583f4, &gs);

            gs.x = 286;
            menu_format_number(row->fields[1], 1, 0, gs.codes);
            menu_draw_number(&DAT_800583e8, &gs);

            gs.x = 181;
            gs.codes[0] = 0xf0;
            gs.codes[1] = 242;
            gs.codes[2] = -1;
            gs.y += 14;
            menu_draw_string(&DAT_800583f4, &gs);

            gs.x = 230;
            menu_format_number(row->fields[2], 4, 0, gs.codes);
            menu_draw_number(&DAT_800583e8, &gs);

            gs.codes[0] = 11;
            gs.codes[1] = -1;
            gs.x += 28;
            menu_draw_number(&DAT_800583e8, &gs);

            gs.x += 7;
            menu_format_number(row->fields[3], 4, 0, gs.codes);
            menu_draw_number(&DAT_800583e8, &gs);

            gs.x = 181;
            gs.codes[0] = 0xf1;
            gs.codes[1] = 242;
            gs.codes[2] = -1;
            gs.y += 14;
            menu_draw_string(&DAT_800583f4, &gs);

            gs.x = 230;
            menu_format_number(row->fields[4], 4, 0, gs.codes);
            menu_draw_number(&DAT_800583e8, &gs);

            gs.codes[0] = 11;
            gs.codes[1] = -1;
            gs.x += 28;
            menu_draw_number(&DAT_800583e8, &gs);

            gs.x += 7;
            menu_format_number(row->fields[5], 4, 0, gs.codes);
            menu_draw_number(&DAT_800583e8, &gs);
        }
        row++;
    }
}
