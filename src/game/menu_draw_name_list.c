#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

enum {
    EQUIPMENT_NAME_X = 174,
    EQUIPMENT_NAME_FIRST_Y = 40,
    EQUIPMENT_NAME_ROW_STEP = 20
};

/*
 * Draw the equipment column of the status panel: the equipped weapon and the
 * selected magic (from their own name tables) followed by the six worn armour
 * / accessory slots. Each category retains its row even when empty. Every
 * displayed name is copied whole from its table into the shared glyph workspace.
 */
ADDRESS(0x8002718c, 0x838)
void menu_draw_name_list(void)
{
    MenuGlyphString gs;

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    gs.position.x = EQUIPMENT_NAME_X;
    gs.position.y = EQUIPMENT_NAME_FIRST_Y;
    if (player_state.equipped_weapon_id != KF_ITEM_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_weapon_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.selected_magic_id != KF_MAGIC_NONE) {
        gs.glyphs =
            magic_name_rows[KF_ENUM_ENCODE(u8, player_state.selected_magic_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_shield_id != KF_ITEM_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_shield_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_head_armor_id != KF_ITEM_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_head_armor_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_body_armor_id != KF_ITEM_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_body_armor_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_arm_armor_id != KF_ITEM_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_arm_armor_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_leg_armor_id != KF_ITEM_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_leg_armor_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
    gs.position.y += EQUIPMENT_NAME_ROW_STEP;
    if (player_state.equipped_accessory_id != KF_ITEM_NONE) {
        gs.glyphs =
            item_name_rows[KF_ENUM_ENCODE(u8, player_state.equipped_accessory_id)];
        menu_draw_string(&menu_assets.glyph_atlas, &gs);
    }
}
