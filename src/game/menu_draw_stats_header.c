#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

enum {
    STATS_HEADER_ROW_STEP = 23
};

/*
 * Draw the main menu's player statistics: experience, level, displayed class,
 * floor, HP/MP pairs, status-effect icons and gold. The class title occupies
 * four atlas cells selected by the base physical-power and magic tiers.
 */
ADDRESS(0x80025f38, 0x5a0)
void menu_draw_stats_header(void)
{
    MenuGlyphString gs;
    s32 glyph_index;
    s32 row_step = STATS_HEADER_ROW_STEP;

    current_poly_ft4 = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    gs.position.x = 0xb5;
    gs.position.y = 0x24;
    gs.glyphs.codes[0] = 0x82;
    gs.glyphs.codes[1] = 0x83;
    gs.glyphs.codes[2] = 0x84;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x2b;
    gs.glyphs.codes[1] = MENU_TEXT_DAKUTEN | 0x1c;
    gs.glyphs.codes[2] = 0x2a;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 7;
    gs.glyphs.codes[1] = 0x28;
    gs.glyphs.codes[2] = 0xc;
    gs.glyphs.codes[3] = MENU_TEXT_END;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0xcc;
    gs.glyphs.codes[1] = 0xcd;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0xf0;
    gs.glyphs.codes[1] = 0xf2;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0xf1;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = 0x85;
    gs.glyphs.codes[1] = 0x86;
    gs.glyphs.codes[2] = MENU_TEXT_END;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.glyphs.codes[0] = MENU_TEXT_DAKUTEN | 0x9;
    gs.glyphs.codes[1] = 0x2d;
    gs.glyphs.codes[2] = 0x2a;
    gs.glyphs.codes[3] = MENU_TEXT_DAKUTEN | 0x13;
    gs.glyphs.codes[4] = MENU_TEXT_END;
    gs.position.y += row_step;
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = 0xfb;
    gs.position.y = 0x24;
    menu_format_number(player_state.experience, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.y += row_step;
    menu_format_number(player_state.progress_state.level, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0xed;
    gs.position.y += row_step;
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

    gs.position.x = 0xfb;
    gs.position.y += row_step;
    menu_format_number(KF_ENUM_ENCODE(u8, player_state.progress_state.current_floor), MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0xe6;
    gs.position.y += row_step;
    menu_format_number(player_state.vitals.current_hp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.glyphs.codes[0] = MENU_NUMBER_SLASH;
    gs.glyphs.codes[1] = MENU_TEXT_END;
    gs.position.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.x += MENU_NUMBER_ADVANCE;
    menu_format_number(player_state.vitals.maximum_hp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0xe6;
    gs.position.y += row_step;
    menu_format_number(player_state.vitals.current_mp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.glyphs.codes[0] = MENU_NUMBER_SLASH;
    gs.glyphs.codes[1] = MENU_TEXT_END;
    gs.position.x += MENU_STATS_VITAL_DIGITS * MENU_NUMBER_ADVANCE;
    menu_draw_number(&menu_assets.number_atlas, &gs);
    gs.position.x += MENU_NUMBER_ADVANCE;
    menu_format_number(player_state.vitals.maximum_mp, MENU_STATS_VITAL_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);

    gs.position.x = 0xdf;
    gs.glyphs.codes[0] = MENU_TEXT_BLANK;
    gs.glyphs.codes[1] = MENU_TEXT_BLANK;
    gs.glyphs.codes[2] = MENU_TEXT_BLANK;
    gs.glyphs.codes[3] = MENU_TEXT_BLANK;
    gs.glyphs.codes[4] = MENU_TEXT_BLANK;
    gs.glyphs.codes[5] = MENU_TEXT_END;
    gs.position.y += row_step;
    if (player_state.status_effect_flags == KF_PLAYER_STATUS_NONE) {
        gs.glyphs.codes[3] = 0xc5;
        gs.glyphs.codes[4] = 0xc6;
    }
    glyph_index = 4;
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_SLOWED) != KF_PLAYER_STATUS_NONE) {
        gs.glyphs.codes[4] = 0xc9;
        glyph_index = 3;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_POISON) != KF_PLAYER_STATUS_NONE) {
        gs.glyphs.codes[glyph_index] = 0x88;
        glyph_index--;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_DARKNESS) != KF_PLAYER_STATUS_NONE) {
        gs.glyphs.codes[glyph_index] = 199;
        glyph_index--;
    }
    if ((player_state.status_effect_flags & KF_PLAYER_STATUS_CURSE) != KF_PLAYER_STATUS_NONE) {
        gs.glyphs.codes[glyph_index] = 200;
    }
    menu_draw_string(&menu_assets.glyph_atlas, &gs);

    gs.position.x = 0xfb;
    gs.position.y += row_step;
    menu_format_number(player_state.gold, MENU_STATS_VALUE_DIGITS, KF_FORMAT_PAD_SPACES, gs.glyphs.codes);
    menu_draw_number(&menu_assets.number_atlas, &gs);
}
