#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

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
void menu_item_model_preview(s32 item_id)
{
    MenuGlyphString gs;
    MATRIX rot;
    MATRIX lsrc;
    MATRIX lres;
    s16 *name;
    s32 i;

    if (item_id != KF_ITEM_NONE) {
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

        gs.x = 0xe6;
        gs.codes[0] = 0xca;
        gs.codes[1] = 0xcb;
        gs.codes[2] = MENU_TEXT_END;
        gs.y += 18;
        menu_draw_string(&menu_assets.glyph_atlas, &gs);

        gs.x = 0x117;
        menu_format_number(item_stock[0][item_id], 2, 0, gs.codes);
        menu_draw_number(&menu_assets.number_atlas, &gs);
    }
}
