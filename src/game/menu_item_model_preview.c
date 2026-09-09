#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

enum {
    MENU_INVENTORY_QUANTITY_LABEL_X = 230,
    MENU_INVENTORY_QUANTITY_VALUE_X = 279
};

DATA(0x80057b70, 0x8)
SVECTOR menu_item_preview_rotation = {0, 0, 0, 0};

/*
 * Draw a spinning 3D model preview of the inventory item plus its name and held
 * quantity.  Builds the GTE rotation from the shared spin angle and a fixed
 * local light matrix, projects the TMD, then writes the name glyph string, a
 * fixed label pair and the two-digit held count into the glyph workspace.
 * `item_id` 0xff selects the empty slot and draws nothing.
 */
ADDRESS(0x800279c4, 0x1b8)
void menu_item_model_preview(KF_ENUM_PARAM(KfItemId, s32) item_id)
{
    MenuGlyphString gs;
    MATRIX rot;
    MATRIX lsrc;
    MATRIX lres;
    s16 *name;
    s32 i;

    if (item_id != KF_ITEM_NONE) {
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

        current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

        gs.position.x = MENU_ITEM_PREVIEW_NAME_X;
        gs.position.y = MENU_ITEM_PREVIEW_NAME_Y;
        name = item_name_rows[KF_ENUM_ENCODE(s32, item_id)].codes;
        for (i = 0; i < MENU_GLYPHS_PER_ROW; i++) {
            gs.glyphs.codes[i] = name[i];
        }
        menu_draw_string(&menu_assets.glyph_atlas, &gs);

        gs.position.x = MENU_INVENTORY_QUANTITY_LABEL_X;
        gs.glyphs.codes[0] = 0xca;
        gs.glyphs.codes[1] = 0xcb;
        gs.glyphs.codes[2] = MENU_TEXT_END;
        gs.position.y += MENU_ITEM_PREVIEW_LINE_HEIGHT;
        menu_draw_string(&menu_assets.glyph_atlas, &gs);

        gs.position.x = MENU_INVENTORY_QUANTITY_VALUE_X;
        menu_format_number(item_stock[KF_ITEM_STOCK_PLAYER][KF_ENUM_ENCODE(s32, item_id)], MENU_ITEM_PREVIEW_QUANTITY_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
        menu_draw_number(&menu_assets.number_atlas, &gs);
    }
}
