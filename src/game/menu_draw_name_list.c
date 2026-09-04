#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

/*
 * Draw the equipment column of the status panel: the equipped weapon and the
 * selected magic (from their own name tables) followed by the six worn armour
 * / accessory slots, one row per non-empty slot (0xff = empty), each advancing
 * the row origin 0x14 pixels.  Every name is copied whole from its table into
 * the shared glyph workspace before drawing.
 */
ADDRESS(0x8002718c, 0x838)
void menu_draw_name_list(void)
{
    MenuGlyphString gs;

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;

    gs.x = 0xae;
    gs.y = 0x28;
    if (player_state.equipped_weapon_id != 0xff) {
        *(MenuGlyphRow *)gs.codes =
            item_name_rows[player_state.equipped_weapon_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.selected_magic_id != 0xff) {
        *(MenuGlyphRow *)gs.codes =
            magic_name_rows[player_state.selected_magic_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.equipped_body_armor_id != 0xff) {
        *(MenuGlyphRow *)gs.codes =
            item_name_rows[player_state.equipped_body_armor_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.equipped_shield_id != 0xff) {
        *(MenuGlyphRow *)gs.codes =
            item_name_rows[player_state.equipped_shield_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.equipped_head_armor_id != 0xff) {
        *(MenuGlyphRow *)gs.codes =
            item_name_rows[player_state.equipped_head_armor_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.equipped_arm_armor_id != 0xff) {
        *(MenuGlyphRow *)gs.codes =
            item_name_rows[player_state.equipped_arm_armor_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.equipped_leg_armor_id != 0xff) {
        *(MenuGlyphRow *)gs.codes =
            item_name_rows[player_state.equipped_leg_armor_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.equipped_accessory_id != 0xff) {
        *(MenuGlyphRow *)gs.codes =
            item_name_rows[player_state.equipped_accessory_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
}
