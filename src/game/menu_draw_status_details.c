#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

enum {
    STATUS_SUMMARY_ROW_STEP = 16,
    STATUS_COMPONENT_ROW_STEP = 14
};

/*
 * Draw the full status page: the left column of vitals (experience, level,
 * displayed class, HP/MP pairs, status icons, gold, the derived
 * attack and defence ratings) and the right column of per-element attack and
 * defence component values, all through the shared glyph workspace.
 */
ADDRESS(0x800264d8, 0xcb4)
void menu_draw_status_details(void)
{
    MenuGlyphString gs;
    s32 glyph_index;
    s32 rating;

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;

    gs.x = 0x15;
    gs.y = 0x23;
    gs.codes[0] = 0x82;
    gs.codes[1] = 0x83;
    gs.codes[2] = 0x84;
    gs.codes[3] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[0] = 0x2b;
    gs.codes[1] = MENU_TEXT_DAKUTEN | 0x1c;
    gs.codes[2] = 0x2a;
    gs.codes[3] = MENU_TEXT_END;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[0] = 7;
    gs.codes[1] = 0x28;
    gs.codes[2] = 0xc;
    gs.codes[3] = MENU_TEXT_END;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[0] = 0xf0;
    gs.codes[1] = 0xf2;
    gs.codes[2] = MENU_TEXT_END;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[0] = 0xf1;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[0] = 0x85;
    gs.codes[1] = 0x86;
    gs.codes[2] = MENU_TEXT_END;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[0] = MENU_TEXT_DAKUTEN | 0x9;
    gs.codes[1] = 0x2d;
    gs.codes[2] = 0x2a;
    gs.codes[3] = MENU_TEXT_DAKUTEN | 0x13;
    gs.codes[4] = MENU_TEXT_END;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[0] = 0x8c;
    gs.codes[1] = 0x8b;
    gs.codes[2] = MENU_TEXT_END;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[0] = 0x78;
    gs.codes[2] = MENU_TEXT_END;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[0] = 0xce;
    gs.codes[1] = 0xcf;
    gs.codes[2] = 0x89;
    gs.codes[3] = 0x8a;
    gs.codes[4] = 0x8b;
    gs.codes[5] = MENU_TEXT_END;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[2] = 0x7a;
    gs.codes[3] = 0xd0;
    gs.codes[5] = MENU_TEXT_END;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.x = 0x5b;
    gs.y = 0x23;
    menu_format_number(player_state.experience, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.progress_state.level, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.x = 0x4d;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    if (player_state.base_magic < MENU_CLASS_MIDDLE_STAT_MIN) {
        glyph_index = 0;
    } else {
        glyph_index = 2;
        if (player_state.base_magic < MENU_CLASS_HIGH_STAT_MIN) {
            glyph_index = 1;
        }
    }
    if (player_state.base_physical_power > (MENU_CLASS_MIDDLE_STAT_MIN - 1)) {
        if (player_state.base_physical_power < MENU_CLASS_HIGH_STAT_MIN) {
            glyph_index += MENU_CLASS_MAGIC_TIER_COUNT;
        } else {
            glyph_index += 2 * MENU_CLASS_MAGIC_TIER_COUNT;
        }
    }
    glyph_index *= MENU_CLASS_LABEL_GLYPHS;
    gs.codes[0] = glyph_index + MENU_CLASS_FIRST_GLYPH;
    gs.codes[1] = glyph_index + MENU_CLASS_FIRST_GLYPH + 1;
    gs.codes[2] = glyph_index + MENU_CLASS_FIRST_GLYPH + 2;
    gs.codes[3] = glyph_index + MENU_CLASS_FIRST_GLYPH + 3;
    gs.codes[4] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.x = 0x46;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.vitals.current_hp, MENU_STATS_VITAL_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.codes[0] = MENU_NUMBER_SLASH;
    gs.codes[1] = MENU_TEXT_END;
    gs.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.x += MENU_NUMBER_ADVANCE;
    menu_format_number(player_state.vitals.maximum_hp, MENU_STATS_VITAL_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.x = 0x46;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.vitals.current_mp, MENU_STATS_VITAL_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.codes[0] = MENU_NUMBER_SLASH;
    gs.codes[1] = MENU_TEXT_END;
    gs.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.x += MENU_NUMBER_ADVANCE;
    menu_format_number(player_state.vitals.maximum_mp, MENU_STATS_VITAL_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.x = 0x3f;
    gs.codes[0] = MENU_TEXT_BLANK;
    gs.codes[1] = MENU_TEXT_BLANK;
    gs.codes[2] = MENU_TEXT_BLANK;
    gs.codes[3] = MENU_TEXT_BLANK;
    gs.codes[4] = MENU_TEXT_BLANK;
    gs.codes[5] = MENU_TEXT_END;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    if (player_state.status_effect_flags == 0) {
        gs.codes[3] = 0xc5;
        gs.codes[4] = 0xc6;
    }
    glyph_index = 4;
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_SLOWED) != 0) {
        gs.codes[4] = 0xc9;
        glyph_index = 3;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_POISON) != 0) {
        gs.codes[glyph_index] = 0x88;
        glyph_index--;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_DARKNESS) != 0) {
        gs.codes[glyph_index] = 199;
        glyph_index--;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_CURSE) != 0) {
        gs.codes[glyph_index] = 200;
    }
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.x = 0x5b;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.gold, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.physical_power, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.magic, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    rating = (((u32)player_state.cutting_attack + player_state.striking_attack +
               player_state.piercing_attack) * 3 >> 1) +
             (player_state.holy_attack + player_state.fire_attack) * 2;
    rating = rating * 10 / 8;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(rating, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    rating = player_state.cutting_defense + player_state.striking_defense +
             player_state.piercing_defense + player_state.poison_resistance / 5 +
             player_state.magic_defense + player_state.fire_defense;
    rating = rating * 10 / 7;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(rating, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.x = 0xb5;
    gs.y = 0x1e;
    gs.codes[0] = 0x89;
    gs.codes[1] = 0x8a;
    gs.codes[2] = 0x8b;
    gs.codes[3] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[0] = MENU_TEXT_BLANK;
    gs.codes[1] = 0xd1;
    gs.codes[2] = 0x6a;
    gs.codes[3] = MENU_TEXT_END;
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[1] = 0xd2;
    gs.codes[2] = 0x51;
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[1] = 0xd3;
    gs.codes[2] = 0x4c;
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[1] = 0xbf;
    gs.codes[2] = 0x58;
    gs.codes[3] = 0x78;
    gs.codes[4] = 0x79;
    gs.codes[5] = MENU_TEXT_END;
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[1] = 0xd4;
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[0] = 0x7a;
    gs.codes[1] = 0xd0;
    gs.codes[2] = 0x8b;
    gs.codes[3] = MENU_TEXT_END;
    gs.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[0] = MENU_TEXT_BLANK;
    gs.codes[1] = 0xd1;
    gs.codes[2] = 0x6a;
    gs.codes[3] = MENU_TEXT_END;
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[1] = 0xd2;
    gs.codes[2] = 0x51;
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[1] = 0xd3;
    gs.codes[2] = 0x4c;
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[1] = 0x88;
    gs.codes[2] = MENU_TEXT_END;
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[1] = 0x78;
    gs.codes[2] = 0x58;
    gs.codes[3] = 0x78;
    gs.codes[4] = 0x79;
    gs.codes[5] = MENU_TEXT_END;
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.codes[1] = 0xd4;
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.x = 0xfb;
    gs.y = 0x2c;
    menu_format_number(player_state.cutting_attack, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.striking_attack, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.piercing_attack, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.holy_attack, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.fire_attack, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.y += 0x1e;
    menu_format_number(player_state.cutting_defense, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.striking_defense, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.piercing_defense, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.poison_resistance, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.magic_defense, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.fire_defense, MENU_STATS_VALUE_DIGITS, 0, gs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
}
