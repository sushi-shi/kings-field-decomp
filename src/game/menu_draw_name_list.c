#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>


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

/* One ten-glyph name record, copied as a block from the name tables. */
typedef struct MenuName {
    s16 glyphs[10];
} MenuName;

extern void menu_draw_string(const MenuSpriteDef *font, const MenuGlyphString *str);

extern MenuSpriteDef DAT_800583f4;

/* Item and magic name glyph tables (ten glyph codes per entry). */
extern MenuName DAT_80058dc0[];
extern MenuName DAT_80059400[];

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
        *(MenuName *)gs.codes = DAT_80058dc0[player_state.equipped_weapon_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.selected_magic_id != 0xff) {
        *(MenuName *)gs.codes = DAT_80059400[player_state.selected_magic_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.equipped_body_armor_id != 0xff) {
        *(MenuName *)gs.codes = DAT_80058dc0[player_state.equipped_body_armor_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.equipped_shield_id != 0xff) {
        *(MenuName *)gs.codes = DAT_80058dc0[player_state.equipped_shield_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.equipped_head_armor_id != 0xff) {
        *(MenuName *)gs.codes = DAT_80058dc0[player_state.equipped_head_armor_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.equipped_arm_armor_id != 0xff) {
        *(MenuName *)gs.codes = DAT_80058dc0[player_state.equipped_arm_armor_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.equipped_leg_armor_id != 0xff) {
        *(MenuName *)gs.codes = DAT_80058dc0[player_state.equipped_leg_armor_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
    gs.y += 0x14;
    if (player_state.equipped_accessory_id != 0xff) {
        *(MenuName *)gs.codes = DAT_80058dc0[player_state.equipped_accessory_id];
        menu_draw_string(&DAT_800583f4, &gs);
    }
}
