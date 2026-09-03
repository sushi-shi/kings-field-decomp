#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

extern POLY_FT4 *current_poly_ft4;

/* Decimal formatter: renders `value` right-justified in `width` glyph cells. */
extern void menu_format_number(s32 value, s32 width, s32 flag, u16 *out);

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

extern void menu_draw_string(const MenuSpriteDef *font, const MenuGlyphString *str);

/* Shared menu font atlas and number atlas descriptors. */
extern MenuSpriteDef DAT_800583f4;
extern MenuSpriteDef DAT_800583e8;

/*
 * Draw the status-panel header column: the fixed field labels down the left,
 * then the player's experience, level, floor, HP/MP value pairs, the coloured
 * STR/MAG rating glyphs (four cells whose base is chosen from the two base
 * stats), the active status-effect icons, and the gold total.
 */
ADDRESS(0x80025f38, 0x5a0)
void menu_draw_stats_header(void)
{
    MenuGlyphString gs;
    s32 color;
    s32 i;

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;

    gs.x = 0xb5;
    gs.y = 0x24;
    gs.codes[0] = 0x82;
    gs.codes[1] = 0x83;
    gs.codes[2] = 0x84;
    gs.codes[3] = -1;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0x2b;
    gs.codes[1] = 0x101c;
    gs.codes[2] = 0x2a;
    gs.codes[3] = -1;
    gs.y += 0x17;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 7;
    gs.codes[1] = 0x28;
    gs.codes[2] = 0xc;
    gs.codes[3] = -1;
    gs.y += 0x17;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0xcc;
    gs.codes[1] = 0xcd;
    gs.codes[2] = -1;
    gs.y += 0x17;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0xf0;
    gs.codes[1] = 0xf2;
    gs.codes[2] = -1;
    gs.y += 0x17;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0xf1;
    gs.y += 0x17;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0x85;
    gs.codes[1] = 0x86;
    gs.codes[2] = -1;
    gs.y += 0x17;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0x1009;
    gs.codes[1] = 0x2d;
    gs.codes[2] = 0x2a;
    gs.codes[3] = 0x1013;
    gs.codes[4] = -1;
    gs.y += 0x17;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.x = 0xfb;
    gs.y = 0x24;
    menu_format_number(player_state.experience, 6, 0, (u16 *)gs.codes);
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

    gs.y += 0x17;
    menu_format_number(player_state.progress_state.level, 6, 0, (u16 *)gs.codes);
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

    gs.x = 0xed;
    gs.y += 0x17;
    if (player_state.base_magic < 0x28) {
        color = 0;
    } else {
        color = 2;
        if (player_state.base_magic < 0x3c) {
            color = 1;
        }
    }
    if (player_state.base_physical_power > 0x27) {
        if (player_state.base_physical_power < 0x3c) {
            color += 3;
        } else {
            color += 6;
        }
    }
    color *= 4;
    gs.codes[0] = color + 0x100;
    gs.codes[1] = color + 0x101;
    gs.codes[2] = color + 0x102;
    gs.codes[3] = color + 0x103;
    gs.codes[4] = -1;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.x = 0xfb;
    gs.y += 0x17;
    menu_format_number(player_state.progress_state.current_floor, 6, 0, (u16 *)gs.codes);
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

    gs.x = 0xe6;
    gs.y += 0x17;
    menu_format_number(player_state.vitals.current_hp, 4, 0, (u16 *)gs.codes);
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);
    gs.codes[0] = 0xb;
    gs.codes[1] = -1;
    gs.x += 0x1c;
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);
    gs.x += 7;
    menu_format_number(player_state.vitals.maximum_hp, 4, 0, (u16 *)gs.codes);
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

    gs.x = 0xe6;
    gs.y += 0x17;
    menu_format_number(player_state.vitals.current_mp, 4, 0, (u16 *)gs.codes);
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);
    gs.codes[0] = 0xb;
    gs.codes[1] = -1;
    gs.x += 0x1c;
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);
    gs.x += 7;
    menu_format_number(player_state.vitals.maximum_mp, 4, 0, (u16 *)gs.codes);
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);

    gs.x = 0xdf;
    gs.codes[0] = 0xff;
    gs.codes[1] = 0xff;
    gs.codes[2] = 0xff;
    gs.codes[3] = 0xff;
    gs.codes[4] = 0xff;
    gs.codes[5] = -1;
    gs.y += 0x17;
    if (player_state.status_effect_flags == 0) {
        gs.codes[3] = 0xc5;
        gs.codes[4] = 0xc6;
    }
    i = 4;
    if ((player_state.status_effect_flags & 8) != 0) {
        gs.codes[4] = 0xc9;
        i = 3;
    }
    if ((player_state.status_effect_flags & 4) != 0) {
        gs.codes[i] = 0x88;
        i--;
    }
    if ((player_state.status_effect_flags & 2) != 0) {
        gs.codes[i] = 199;
        i--;
    }
    if ((player_state.status_effect_flags & 1) != 0) {
        gs.codes[i] = 200;
    }
    menu_draw_string(&DAT_800583f4, &gs);

    gs.x = 0xfb;
    gs.y += 0x17;
    menu_format_number(player_state.unknown_2c, 6, 0, (u16 *)gs.codes);
    menu_draw_number((u16 *)&DAT_800583e8, (s16 *)&gs);
}
