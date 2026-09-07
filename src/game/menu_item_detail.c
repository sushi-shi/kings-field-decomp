#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

/*
 * Item-detail / dialog-frame menu drawing, one contiguous run
 * (0x80027b7c..0x80028380): the spinning item preview and its labels, the two
 * shared ordering-table quad linkers, and the bordered dialog frame with its
 * save-slot summary rows. All draw through the shared glyph-string workspace
 * and sprite descriptors. Module boundary is WIP.
 */
/*
 * Draw the selected inventory item's detail: a spinning 3D TMD preview (built
 * from the GTE rotation and a fixed local light matrix), the item name, its
 * price for the active shop column/table, the held quantity, and the player's
 * gold.  `item_id` selects the item and `shop_id` the one-based price column.
 * `price_mode` selects buy prices at zero and sell prices for any nonzero value.
 */
ADDRESS(0x80027b7c, 0x2dc)
void menu_draw_item_detail(s32 item_id, s32 shop_id, KfItemPriceMode price_mode)
{
    MenuGlyphString gs;
    MATRIX rot;
    MATRIX lsrc;
    MATRIX lres;
    u16 (*prices)[2];
    s16 *name;
    s32 i;

    if (item_id == KF_ITEM_NONE) {
        return;
    }

    rot.t[0] = 0x230;
    rot.t[1] = 0x8c;
    rot.t[2] = 0x5dc;
    menu_item_preview_rotation.vy =
        (menu_item_preview_rotation.vy + MENU_ITEM_PREVIEW_YAW_STEP)
        & KF_ANGLE_WRAP_MASK;
    RotMatrix(&menu_item_preview_rotation, &rot);

    lsrc.m[0][0] = -KF_FIXED12_ONE;
    lsrc.m[0][1] = -KF_FIXED12_ONE;
    lsrc.m[0][2] = -KF_FIXED12_ONE;
    lsrc.m[1][0] = -KF_FIXED12_ONE;
    lsrc.m[1][1] = -KF_FIXED12_ONE;
    lsrc.m[1][2] = -KF_FIXED12_ONE;
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
    name = item_name_rows[item_id].codes;
    for (i = 0; i < 10; i++) {
        gs.codes[i] = name[i];
    }
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.x = 0xc8;
    gs.y += 18;
    prices = (price_mode != KF_ITEM_PRICE_BUY) ? item_sell_prices : item_buy_prices;
    menu_format_number(prices[item_id][shop_id - 1], 6, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.x = 0xf2;
    gs.codes[0] = MENU_TEXT_DAKUTEN | 0x9;
    gs.codes[1] = 0x2d;
    gs.codes[2] = 0x2a;
    gs.codes[3] = MENU_TEXT_DAKUTEN | 0x13;
    gs.codes[4] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.x = 0xf2;
    gs.codes[0] = 0xca;
    gs.codes[1] = 0xcb;
    gs.codes[2] = MENU_TEXT_END;
    gs.y += 18;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.x = 0x11c;
    menu_format_number(item_stock[0][item_id], 2, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    menu_blit_sprite_translucent(
        &menu_assets.row_background,
        (const MenuPoint *)&menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD]);
    menu_draw_string(&menu_assets.glyph_atlas, &menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD]);

    gs.x = menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD].x + 28;
    gs.y = menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD].y;
    menu_format_number(player_state.gold, 6, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
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
            &menu_assets.mid_depth_quads[display_state.buffer_index]);
}

/* Link the shared front menu quad at ordering-table slot 0. */
ADDRESS(0x80027ea0, 0x44)
void menu_add_frame_quad(void)
{
    AddPrim(display_state.ordering_table,
            &menu_assets.foreground_quads[display_state.buffer_index]);
}

/*
 * Draw a bordered menu dialog frame plus optional save-slot summary rows.
 * `kind` selects which of the composite-frame quads (indices 2..4 of the
 * double-buffered flat-quad array) border the box.  When `rows` is non-null,
 * up to three slot summaries are drawn, each gated by positive saved current
 * HP; every row prints an icon label plus its numeric fields through the
 * shared glyph-string workspace.  Used by the save/load panels, the save
 * confirmation, and the two-option confirm dialog.
 */
ADDRESS(0x80027ee4, 0x49c)
void menu_draw_dialog_frame(const KfSaveSlotSummary *rows, s32 kind)
{
    MenuGlyphString gs;
    s32 i;

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;

    if (kind == 0) {
        AddPrim(display_state.ordering_table + 1000,
                &menu_assets.dialog_quads[display_state.buffer_index][3]);
        AddPrim(display_state.ordering_table + 1000,
                &menu_assets.dialog_quads[display_state.buffer_index][4]);
    }
    if (kind == 1) {
        AddPrim(display_state.ordering_table + 1000,
                &menu_assets.dialog_quads[display_state.buffer_index][2]);
        AddPrim(display_state.ordering_table + 1000,
                &menu_assets.dialog_quads[display_state.buffer_index][4]);
    }
    if (kind == 2) {
        AddPrim(display_state.ordering_table + 1000,
                &menu_assets.dialog_quads[display_state.buffer_index][2]);
        AddPrim(display_state.ordering_table + 1000,
                &menu_assets.dialog_quads[display_state.buffer_index][3]);
    }
    if (kind >= 3) {
        AddPrim(display_state.ordering_table + 1000,
                &menu_assets.dialog_quads[display_state.buffer_index][2]);
        AddPrim(display_state.ordering_table + 1000,
                &menu_assets.dialog_quads[display_state.buffer_index][3]);
        AddPrim(display_state.ordering_table + 1000,
                &menu_assets.dialog_quads[display_state.buffer_index][4]);
    }

    if (rows == 0) {
        return;
    }

    for (i = 0; i < KF_SAVE_SLOT_COUNT; i++) {
        gs.y = i * 65 + 30;
        if ((s32)rows[i].current_hp > 0) {
            gs.x = 181;
            gs.codes[0] = 0x82;
            gs.codes[1] = 0x83;
            gs.codes[2] = 0x84;
            gs.codes[3] = MENU_TEXT_END;
            menu_draw_string(&menu_assets.glyph_atlas, &gs);

            gs.x = 251;
            menu_format_number(rows[i].experience, 6, 0, gs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.x = 181;
            gs.codes[0] = 0xcc;
            gs.codes[1] = 0xcd;
            gs.codes[2] = MENU_TEXT_END;
            gs.y += 14;
            menu_draw_string(&menu_assets.glyph_atlas, &gs);

            gs.x = 286;
            menu_format_number(rows[i].current_floor, 1, 0, gs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.x = 181;
            gs.codes[0] = 0xf0;
            gs.codes[1] = 242;
            gs.codes[2] = MENU_TEXT_END;
            gs.y += 14;
            menu_draw_string(&menu_assets.glyph_atlas, &gs);

            gs.x = 230;
            menu_format_number(rows[i].current_hp, 4, 0, gs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.codes[0] = MENU_NUMBER_SLASH;
            gs.codes[1] = MENU_TEXT_END;
            gs.x += 28;
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.x += 7;
            menu_format_number(rows[i].maximum_hp, 4, 0, gs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.x = 181;
            gs.codes[0] = 0xf1;
            gs.codes[1] = 242;
            gs.codes[2] = MENU_TEXT_END;
            gs.y += 14;
            menu_draw_string(&menu_assets.glyph_atlas, &gs);

            gs.x = 230;
            menu_format_number(rows[i].current_mp, 4, 0, gs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.codes[0] = MENU_NUMBER_SLASH;
            gs.codes[1] = MENU_TEXT_END;
            gs.x += 28;
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.x += 7;
            menu_format_number(rows[i].maximum_mp, 4, 0, gs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);
        }
    }
}
