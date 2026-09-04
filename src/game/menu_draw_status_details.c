#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

/*
 * Draw the full status page: the left column of vitals (experience, level,
 * floor, coloured STR/MAG rating, HP/MP pairs, status icons, gold, the derived
 * attack and defence ratings) and the right column of per-element attack and
 * defence component values, all through the shared glyph workspace.
 */
ADDRESS(0x800264d8, 0xcb4)
void menu_draw_status_details(void)
{
    MenuGlyphString gs;
    s32 color;
    s32 i;

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;

    gs.x = 0x15;
    gs.y = 0x23;
    gs.codes[0] = 0x82;
    gs.codes[1] = 0x83;
    gs.codes[2] = 0x84;
    gs.codes[3] = -1;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0x2b;
    gs.codes[1] = 0x101c;
    gs.codes[2] = 0x2a;
    gs.codes[3] = -1;
    gs.y += 0x10;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 7;
    gs.codes[1] = 0x28;
    gs.codes[2] = 0xc;
    gs.codes[3] = -1;
    gs.y += 0x10;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0xf0;
    gs.codes[1] = 0xf2;
    gs.codes[2] = -1;
    gs.y += 0x10;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0xf1;
    gs.y += 0x10;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0x85;
    gs.codes[1] = 0x86;
    gs.codes[2] = -1;
    gs.y += 0x10;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0x1009;
    gs.codes[1] = 0x2d;
    gs.codes[2] = 0x2a;
    gs.codes[3] = 0x1013;
    gs.codes[4] = -1;
    gs.y += 0x10;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0x8c;
    gs.codes[1] = 0x8b;
    gs.codes[2] = -1;
    gs.y += 0x10;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0x78;
    gs.codes[2] = -1;
    gs.y += 0x10;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0xce;
    gs.codes[1] = 0xcf;
    gs.codes[2] = 0x89;
    gs.codes[3] = 0x8a;
    gs.codes[4] = 0x8b;
    gs.codes[5] = -1;
    gs.y += 0x10;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[2] = 0x7a;
    gs.codes[3] = 0xd0;
    gs.codes[5] = -1;
    gs.y += 0x10;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.x = 0x5b;
    gs.y = 0x23;
    menu_format_number(player_state.experience, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);

    gs.y += 0x10;
    menu_format_number(player_state.progress_state.level, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);

    gs.x = 0x4d;
    gs.y += 0x10;
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

    gs.x = 0x46;
    gs.y += 0x10;
    menu_format_number(player_state.vitals.current_hp, 4, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.codes[0] = 0xb;
    gs.codes[1] = -1;
    gs.x += 0x1c;
    menu_draw_number(&DAT_800583e8, &gs);
    gs.x += 7;
    menu_format_number(player_state.vitals.maximum_hp, 4, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);

    gs.x = 0x46;
    gs.y += 0x10;
    menu_format_number(player_state.vitals.current_mp, 4, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.codes[0] = 0xb;
    gs.codes[1] = -1;
    gs.x += 0x1c;
    menu_draw_number(&DAT_800583e8, &gs);
    gs.x += 7;
    menu_format_number(player_state.vitals.maximum_mp, 4, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);

    gs.x = 0x3f;
    gs.codes[0] = 0xff;
    gs.codes[1] = 0xff;
    gs.codes[2] = 0xff;
    gs.codes[3] = 0xff;
    gs.codes[4] = 0xff;
    gs.codes[5] = -1;
    gs.y += 0x10;
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

    gs.x = 0x5b;
    gs.y += 0x10;
    menu_format_number(player_state.gold, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0x10;
    menu_format_number(player_state.physical_power, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0x10;
    menu_format_number(player_state.magic, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0x10;
    menu_format_number(
        (s32)(((((u32)player_state.attack_component0 + player_state.attack_component1 +
                 player_state.attack_component2) * 3 >> 1) +
               ((u32)player_state.attack_component3 + player_state.attack_component4) * 2) * 10) >> 3,
        6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0x10;
    menu_format_number(
        (((u32)player_state.damage_defense_component0 + player_state.damage_defense_component1 +
          player_state.damage_defense_component2 + player_state.status_effect2_resistance / 5 +
          player_state.damage_defense_component3 + player_state.damage_defense_component4) * 10) / 7,
        6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);

    gs.x = 0xb5;
    gs.y = 0x1e;
    gs.codes[0] = 0x89;
    gs.codes[1] = 0x8a;
    gs.codes[2] = 0x8b;
    gs.codes[3] = -1;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0xff;
    gs.codes[1] = 0xd1;
    gs.codes[2] = 0x6a;
    gs.codes[3] = -1;
    gs.y += 0xe;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[1] = 0xd2;
    gs.codes[2] = 0x51;
    gs.y += 0xe;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[1] = 0xd3;
    gs.codes[2] = 0x4c;
    gs.y += 0xe;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[1] = 0xbf;
    gs.codes[2] = 0x58;
    gs.codes[3] = 0x78;
    gs.codes[4] = 0x79;
    gs.codes[5] = -1;
    gs.y += 0xe;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[1] = 0xd4;
    gs.y += 0xe;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0x7a;
    gs.codes[1] = 0xd0;
    gs.codes[2] = 0x8b;
    gs.codes[3] = -1;
    gs.y += 0x10;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[0] = 0xff;
    gs.codes[1] = 0xd1;
    gs.codes[2] = 0x6a;
    gs.codes[3] = -1;
    gs.y += 0xe;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[1] = 0xd2;
    gs.codes[2] = 0x51;
    gs.y += 0xe;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[1] = 0xd3;
    gs.codes[2] = 0x4c;
    gs.y += 0xe;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[1] = 0x88;
    gs.codes[2] = -1;
    gs.y += 0xe;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[1] = 0x78;
    gs.codes[2] = 0x58;
    gs.codes[3] = 0x78;
    gs.codes[4] = 0x79;
    gs.codes[5] = -1;
    gs.y += 0xe;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.codes[1] = 0xd4;
    gs.y += 0xe;
    menu_draw_string(&DAT_800583f4, &gs);

    gs.x = 0xfb;
    gs.y = 0x2c;
    menu_format_number(player_state.attack_component0, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0xe;
    menu_format_number(player_state.attack_component1, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0xe;
    menu_format_number(player_state.attack_component2, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0xe;
    menu_format_number(player_state.attack_component3, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0xe;
    menu_format_number(player_state.attack_component4, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0x1e;
    menu_format_number(player_state.damage_defense_component0, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0xe;
    menu_format_number(player_state.damage_defense_component1, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0xe;
    menu_format_number(player_state.damage_defense_component2, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0xe;
    menu_format_number(player_state.status_effect2_resistance, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0xe;
    menu_format_number(player_state.damage_defense_component3, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
    gs.y += 0xe;
    menu_format_number(player_state.damage_defense_component4, 6, 0, gs.codes);
    menu_draw_number(&DAT_800583e8, &gs);
}
