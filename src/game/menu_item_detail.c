#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

enum {
    MENU_ITEM_DETAIL_PRICE_X = 200,
    MENU_ITEM_DETAIL_LABEL_X = 242,
    MENU_ITEM_DETAIL_QUANTITY_X = 284,
    MENU_ITEM_DETAIL_GOLD_X_OFFSET = 28,
    MENU_ITEM_DETAIL_PRICE_DIGITS = 6,
    MENU_ITEM_DETAIL_GOLD_DIGITS = 6,
    MENU_MARKER_OT_DEPTH = 500,
    MENU_DIALOG_OT_DEPTH = 1000,
    MENU_SAVE_SLOT0_QUAD = 2,
    MENU_SAVE_SLOT1_QUAD = 3,
    MENU_SAVE_SLOT2_QUAD = 4,
    MENU_SAVE_SUMMARY_ROW_HEIGHT = 65,
    MENU_SAVE_SUMMARY_LINE_HEIGHT = 14,
    MENU_SAVE_SUMMARY_LABEL_X = 181,
    MENU_SAVE_SUMMARY_VALUE_X = 230,
    MENU_SAVE_STATUS_DIGITS = 4
};

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
    u16 (*prices)[KF_ITEM_SHOP_COUNT];
    s16 *name;
    s32 i;

    if (item_id == KF_ITEM_NONE) {
        return;
    }

    rot.t[0] = MENU_ITEM_PREVIEW_TRANSLATION_X;
    rot.t[1] = MENU_ITEM_PREVIEW_TRANSLATION_Y;
    rot.t[2] = MENU_ITEM_PREVIEW_TRANSLATION_Z;
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

    gs.x = MENU_ITEM_PREVIEW_NAME_X;
    gs.y = MENU_ITEM_PREVIEW_NAME_Y;
    name = item_name_rows[item_id].codes;
    for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
        gs.codes[i] = name[i];
    }
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.x = MENU_ITEM_DETAIL_PRICE_X;
    gs.y += MENU_ITEM_PREVIEW_LINE_HEIGHT;
    prices = item_sell_prices;
    if (price_mode == KF_ITEM_PRICE_BUY) {
        prices = item_buy_prices;
    }
    menu_format_number(prices[item_id][shop_id - KF_ITEM_STOCK_FIRST_SHOP], MENU_ITEM_DETAIL_PRICE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.x = MENU_ITEM_DETAIL_LABEL_X;
    gs.codes[0] = MENU_TEXT_DAKUTEN | 0x9;
    gs.codes[1] = 0x2d;
    gs.codes[2] = 0x2a;
    gs.codes[3] = MENU_TEXT_DAKUTEN | 0x13;
    gs.codes[4] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.x = MENU_ITEM_DETAIL_LABEL_X;
    gs.codes[0] = 0xca;
    gs.codes[1] = 0xcb;
    gs.codes[2] = MENU_TEXT_END;
    gs.y += MENU_ITEM_PREVIEW_LINE_HEIGHT;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.x = MENU_ITEM_DETAIL_QUANTITY_X;
    menu_format_number(item_stock[KF_ITEM_STOCK_PLAYER][item_id], MENU_ITEM_PREVIEW_QUANTITY_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    menu_blit_sprite_translucent(
        &menu_assets.row_background,
        (const MenuPoint *)&menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD]);
    menu_draw_string(&menu_assets.glyph_atlas, &menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD]);

    gs.x = menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD].x + MENU_ITEM_DETAIL_GOLD_X_OFFSET;
    gs.y = menu_window_layouts[KF_ENUM_ENCODE(s32, KF_MENU_WINDOW_SHOP)].rows[KF_SHOP_ROW_GOLD].y;
    menu_format_number(player_state.gold, MENU_ITEM_DETAIL_GOLD_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
}

/*
 * Two shared menu ordering-table primitives: double-buffered POLY_FT4 quads
 * that the surrounding panels prime and these helpers link into the current
 * frame's ordering table at a fixed depth. The packets are loaded from
 * STAT.DAT; both helpers select the active buffer's copy and enqueue it.  Both are shared by the magic, list, save and load panels.
 */

/* Link the shared mid-depth menu quad at ordering-table slot 500. */
ADDRESS(0x80027e58, 0x48)
void menu_add_marker_quad(void)
{
    AddPrim(display_state.ordering_table + MENU_MARKER_OT_DEPTH,
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
 * Draw save-slot overlays plus optional summary rows. The highlighted slot
 * omits its overlay; negative indices suppress all overlays and indices beyond
 * the three slots draw all overlays.  When `rows` is non-null,
 * up to three slot summaries are drawn, each gated by positive saved current
 * HP; every row prints an icon label plus its numeric fields through the
 * shared glyph-string workspace.  Used by the save/load panels, the save
 * confirmation, and the two-option confirm dialog.
 */
ADDRESS(0x80027ee4, 0x49c)
void menu_draw_dialog_frame(const KfSaveSlotSummary *rows, s32 highlighted_slot)
{
    MenuGlyphString gs;
    s32 i;

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;

    if (highlighted_slot == 0) {
        AddPrim(display_state.ordering_table + MENU_DIALOG_OT_DEPTH,
                &menu_assets.dialog_quads[display_state.buffer_index][MENU_SAVE_SLOT1_QUAD]);
        AddPrim(display_state.ordering_table + MENU_DIALOG_OT_DEPTH,
                &menu_assets.dialog_quads[display_state.buffer_index][MENU_SAVE_SLOT2_QUAD]);
    }
    if (highlighted_slot == 1) {
        AddPrim(display_state.ordering_table + MENU_DIALOG_OT_DEPTH,
                &menu_assets.dialog_quads[display_state.buffer_index][MENU_SAVE_SLOT0_QUAD]);
        AddPrim(display_state.ordering_table + MENU_DIALOG_OT_DEPTH,
                &menu_assets.dialog_quads[display_state.buffer_index][MENU_SAVE_SLOT2_QUAD]);
    }
    if (highlighted_slot == 2) {
        AddPrim(display_state.ordering_table + MENU_DIALOG_OT_DEPTH,
                &menu_assets.dialog_quads[display_state.buffer_index][MENU_SAVE_SLOT0_QUAD]);
        AddPrim(display_state.ordering_table + MENU_DIALOG_OT_DEPTH,
                &menu_assets.dialog_quads[display_state.buffer_index][MENU_SAVE_SLOT1_QUAD]);
    }
    if (highlighted_slot >= KF_SAVE_SLOT_COUNT) {
        AddPrim(display_state.ordering_table + MENU_DIALOG_OT_DEPTH,
                &menu_assets.dialog_quads[display_state.buffer_index][MENU_SAVE_SLOT0_QUAD]);
        AddPrim(display_state.ordering_table + MENU_DIALOG_OT_DEPTH,
                &menu_assets.dialog_quads[display_state.buffer_index][MENU_SAVE_SLOT1_QUAD]);
        AddPrim(display_state.ordering_table + MENU_DIALOG_OT_DEPTH,
                &menu_assets.dialog_quads[display_state.buffer_index][MENU_SAVE_SLOT2_QUAD]);
    }

    if (rows == 0) {
        return;
    }

    for (i = 0; i < KF_SAVE_SLOT_COUNT; i++) {
        gs.y = i * MENU_SAVE_SUMMARY_ROW_HEIGHT + 30;
        if ((s32)rows[i].current_hp > 0) {
            gs.x = MENU_SAVE_SUMMARY_LABEL_X;
            gs.codes[0] = 0x82;
            gs.codes[1] = 0x83;
            gs.codes[2] = 0x84;
            gs.codes[3] = MENU_TEXT_END;
            menu_draw_string(&menu_assets.glyph_atlas, &gs);

            gs.x = 251;
            menu_format_number(rows[i].experience, 6, 0, gs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.x = MENU_SAVE_SUMMARY_LABEL_X;
            gs.codes[0] = 0xcc;
            gs.codes[1] = 0xcd;
            gs.codes[2] = MENU_TEXT_END;
            gs.y += MENU_SAVE_SUMMARY_LINE_HEIGHT;
            menu_draw_string(&menu_assets.glyph_atlas, &gs);

            gs.x = 286;
            menu_format_number(KF_ENUM_ENCODE(u32, rows[i].current_floor), 1, 0, gs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.x = MENU_SAVE_SUMMARY_LABEL_X;
            gs.codes[0] = 0xf0;
            gs.codes[1] = 242;
            gs.codes[2] = MENU_TEXT_END;
            gs.y += MENU_SAVE_SUMMARY_LINE_HEIGHT;
            menu_draw_string(&menu_assets.glyph_atlas, &gs);

            gs.x = MENU_SAVE_SUMMARY_VALUE_X;
            menu_format_number(rows[i].current_hp, MENU_SAVE_STATUS_DIGITS, 0, gs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.codes[0] = MENU_NUMBER_SLASH;
            gs.codes[1] = MENU_TEXT_END;
            gs.x += MENU_SAVE_STATUS_DIGITS * MENU_NUMBER_ADVANCE;
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.x += MENU_NUMBER_ADVANCE;
            menu_format_number(rows[i].maximum_hp, MENU_SAVE_STATUS_DIGITS, 0, gs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.x = MENU_SAVE_SUMMARY_LABEL_X;
            gs.codes[0] = 0xf1;
            gs.codes[1] = 242;
            gs.codes[2] = MENU_TEXT_END;
            gs.y += MENU_SAVE_SUMMARY_LINE_HEIGHT;
            menu_draw_string(&menu_assets.glyph_atlas, &gs);

            gs.x = MENU_SAVE_SUMMARY_VALUE_X;
            menu_format_number(rows[i].current_mp, MENU_SAVE_STATUS_DIGITS, 0, gs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.codes[0] = MENU_NUMBER_SLASH;
            gs.codes[1] = MENU_TEXT_END;
            gs.x += MENU_SAVE_STATUS_DIGITS * MENU_NUMBER_ADVANCE;
            menu_draw_number(&menu_assets.number_atlas, &gs);

            gs.x += MENU_NUMBER_ADVANCE;
            menu_format_number(rows[i].maximum_mp, MENU_SAVE_STATUS_DIGITS, 0, gs.codes);
            menu_draw_number(&menu_assets.number_atlas, &gs);
        }
    }
}
