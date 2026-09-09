#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

enum {
    STATUS_SUMMARY_ROW_STEP = 16,
    STATUS_COMPONENT_ROW_STEP = 14
};

enum {
    STATUS_PHYSICAL_ATTACK_MULTIPLIER = 3,
    STATUS_PHYSICAL_ATTACK_DOWNSHIFT = 1,
    STATUS_ELEMENTAL_ATTACK_MULTIPLIER = 2,
    STATUS_ATTACK_SCALE_NUMERATOR = 10,
    STATUS_ATTACK_SCALE_DENOMINATOR = 8,
    STATUS_POISON_RESISTANCE_DIVISOR = 5,
    STATUS_DEFENSE_SCALE_NUMERATOR = 10,
    STATUS_DEFENSE_SCALE_DENOMINATOR = 7
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

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    gs.position.x = 0x15;
    gs.position.y = 0x23;
    gs.glyphs.codes[0] = 0x82;
    gs.glyphs.codes[1] = 0x83;
    gs.glyphs.codes[2] = 0x84;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x2b;
    gs.glyphs.codes[1] = MENU_TEXT_DAKUTEN | 0x1c;
    gs.glyphs.codes[2] = 0x2a;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 7;
    gs.glyphs.codes[1] = 0x28;
    gs.glyphs.codes[2] = 0xc;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0xf0;
    gs.glyphs.codes[1] = 0xf2;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0xf1;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x85;
    gs.glyphs.codes[1] = 0x86;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = MENU_TEXT_DAKUTEN | 0x9;
    gs.glyphs.codes[1] = 0x2d;
    gs.glyphs.codes[2] = 0x2a;
    gs.glyphs.codes[3] = MENU_TEXT_DAKUTEN | 0x13;
    gs.glyphs.codes[4] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x8c;
    gs.glyphs.codes[1] = 0x8b;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x78;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0xce;
    gs.glyphs.codes[1] = 0xcf;
    gs.glyphs.codes[2] = 0x89;
    gs.glyphs.codes[3] = 0x8a;
    gs.glyphs.codes[4] = 0x8b;
    gs.glyphs.codes[5] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[2] = 0x7a;
    gs.glyphs.codes[3] = 0xd0;
    gs.glyphs.codes[5] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = 0x5b;
    gs.position.y = 0x23;
    menu_format_number(player_state.experience, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.progress_state.level, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0x4d;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
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
    gs.glyphs.codes[0] = glyph_index + MENU_CLASS_FIRST_GLYPH;
    gs.glyphs.codes[1] = glyph_index + MENU_CLASS_FIRST_GLYPH + 1;
    gs.glyphs.codes[2] = glyph_index + MENU_CLASS_FIRST_GLYPH + 2;
    gs.glyphs.codes[3] = glyph_index + MENU_CLASS_FIRST_GLYPH + 3;
    gs.glyphs.codes[4] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = 0x46;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.vitals.current_hp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.glyphs.codes[0] = MENU_NUMBER_SLASH;
    gs.glyphs.codes[1] = MENU_TEXT_END;
    gs.position.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.x += MENU_NUMBER_ADVANCE;
    menu_format_number(player_state.vitals.maximum_hp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0x46;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.vitals.current_mp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.glyphs.codes[0] = MENU_NUMBER_SLASH;
    gs.glyphs.codes[1] = MENU_TEXT_END;
    gs.position.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.x += MENU_NUMBER_ADVANCE;
    menu_format_number(player_state.vitals.maximum_mp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0x3f;
    gs.glyphs.codes[0] = MENU_TEXT_BLANK;
    gs.glyphs.codes[1] = MENU_TEXT_BLANK;
    gs.glyphs.codes[2] = MENU_TEXT_BLANK;
    gs.glyphs.codes[3] = MENU_TEXT_BLANK;
    gs.glyphs.codes[4] = MENU_TEXT_BLANK;
    gs.glyphs.codes[5] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    if (player_state.status_effect_flags == 0) {
        gs.glyphs.codes[3] = 0xc5;
        gs.glyphs.codes[4] = 0xc6;
    }
    glyph_index = 4;
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_SLOWED) != 0) {
        gs.glyphs.codes[4] = 0xc9;
        glyph_index = 3;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_POISON) != 0) {
        gs.glyphs.codes[glyph_index] = 0x88;
        glyph_index--;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_DARKNESS) != 0) {
        gs.glyphs.codes[glyph_index] = 199;
        glyph_index--;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_CURSE) != 0) {
        gs.glyphs.codes[glyph_index] = 200;
    }
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = 0x5b;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.gold, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.physical_power, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(player_state.magic, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    rating = (((u32)player_state.cutting_attack + player_state.striking_attack +
               player_state.piercing_attack) * STATUS_PHYSICAL_ATTACK_MULTIPLIER
               >> STATUS_PHYSICAL_ATTACK_DOWNSHIFT) +
             (player_state.holy_attack + player_state.fire_attack)
             * STATUS_ELEMENTAL_ATTACK_MULTIPLIER;
    rating = rating * STATUS_ATTACK_SCALE_NUMERATOR
        / STATUS_ATTACK_SCALE_DENOMINATOR;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(rating, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    rating = player_state.cutting_defense + player_state.striking_defense +
             player_state.piercing_defense + player_state.poison_resistance / STATUS_POISON_RESISTANCE_DIVISOR +
             player_state.magic_defense + player_state.fire_defense;
    rating = rating * STATUS_DEFENSE_SCALE_NUMERATOR / STATUS_DEFENSE_SCALE_DENOMINATOR;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_format_number(rating, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0xb5;
    gs.position.y = 0x1e;
    gs.glyphs.codes[0] = 0x89;
    gs.glyphs.codes[1] = 0x8a;
    gs.glyphs.codes[2] = 0x8b;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = MENU_TEXT_BLANK;
    gs.glyphs.codes[1] = 0xd1;
    gs.glyphs.codes[2] = 0x6a;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xd2;
    gs.glyphs.codes[2] = 0x51;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xd3;
    gs.glyphs.codes[2] = 0x4c;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xbf;
    gs.glyphs.codes[2] = 0x58;
    gs.glyphs.codes[3] = 0x78;
    gs.glyphs.codes[4] = 0x79;
    gs.glyphs.codes[5] = MENU_TEXT_END;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xd4;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x7a;
    gs.glyphs.codes[1] = 0xd0;
    gs.glyphs.codes[2] = 0x8b;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += STATUS_SUMMARY_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = MENU_TEXT_BLANK;
    gs.glyphs.codes[1] = 0xd1;
    gs.glyphs.codes[2] = 0x6a;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xd2;
    gs.glyphs.codes[2] = 0x51;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xd3;
    gs.glyphs.codes[2] = 0x4c;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0x88;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0x78;
    gs.glyphs.codes[2] = 0x58;
    gs.glyphs.codes[3] = 0x78;
    gs.glyphs.codes[4] = 0x79;
    gs.glyphs.codes[5] = MENU_TEXT_END;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[1] = 0xd4;
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = 0xfb;
    gs.position.y = 0x2c;
    menu_format_number(player_state.cutting_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.striking_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.piercing_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.holy_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.fire_attack, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += 0x1e;
    menu_format_number(player_state.cutting_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.striking_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.piercing_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.poison_resistance, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.magic_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.y += STATUS_COMPONENT_ROW_STEP;
    menu_format_number(player_state.fire_defense, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
}
