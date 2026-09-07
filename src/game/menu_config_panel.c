#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

enum {
    CONFIG_OPTION_ON_X = 180,
    CONFIG_OPTION_OFF_X = 240,
    CONFIG_OPTION_FIRST_Y = 41,
    CONFIG_OPTION_ROW_STEP = 22
};

/* The two labels are private by-value copies; only their Y positions advance. */
void menu_config_panel_draw(
    MenuGlyphString option_a, MenuGlyphString option_b, KfPlayerOption *values);


/*
 * Four editable config rows and an exit row. Both exit inputs publish the
 * working values after the final redraw/release wait; a changed music flag
 * starts or stops the current map sequence.
 */
ADDRESS(0x8002589c, 0x504)
void menu_config_panel(void)
{
    KfPlayerOption states[KF_MENU_CONFIG_SETTING_COUNT];
    MenuGlyphString option_a;
    MenuGlyphString option_b;
    s32 row = 0;
    s32 confirm = 0;
    u32 pad = 0;
    u32 prev;
    KfMenuPanelPhase phase = KF_MENU_PANEL_OPEN;
    KfPlayerOption music_orig;

    while (PadRead(1) != 0) {
    }

    option_a.x = CONFIG_OPTION_ON_X;
    option_a.y = CONFIG_OPTION_FIRST_Y;
    option_a.codes[0] = 0xf9;
    option_a.codes[1] = 0xfa;
    option_a.codes[2] = MENU_TEXT_END;
    option_b.x = CONFIG_OPTION_OFF_X;
    option_b.y = CONFIG_OPTION_FIRST_Y;
    option_b.codes[0] = 0xf9;
    option_b.codes[1] = 0xfb;
    option_b.codes[2] = 0xfb;
    option_b.codes[3] = MENU_TEXT_END;
    states[KF_MENU_CONFIG_EFFECTS_ROW] = player_state.audio_effects_enabled;
    states[KF_MENU_CONFIG_MUSIC_ROW] = player_state.audio_music_enabled;
    states[KF_MENU_CONFIG_GAUGES_ROW] = player_state.hud_gauges_enabled;
    states[KF_MENU_CONFIG_COMPASS_ROW] = player_state.compass_enabled;
    music_orig = states[KF_MENU_CONFIG_MUSIC_ROW];

    menu_frame_begin();
    menu_config_panel_draw(option_a, option_b, states);
    menu_draw_window(KF_MENU_WINDOW_CONFIG, KF_MENU_CONFIG_ROW_COUNT, row, confirm);
    menu_present_frame();
    do {
        if (confirm == 1 || phase == KF_MENU_PANEL_CLOSED) {
            menu_frame_begin();
            menu_config_panel_draw(option_a, option_b, states);
            menu_draw_window(KF_MENU_WINDOW_CONFIG, KF_MENU_CONFIG_ROW_COUNT, row, confirm);
            menu_present_frame();
            while (PadRead(1) != 0) {
            }
        }
        if (phase != KF_MENU_PANEL_OPEN) {
            break;
        }
        confirm = 0;
        menu_frame_begin();
        prev = pad;
        pad = PadRead(1);
        if ((pad & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (row != 0) {
                row--;
            } else {
                row = KF_MENU_CONFIG_RETURN_ROW;
            }
        } else if ((pad & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (row != KF_MENU_CONFIG_RETURN_ROW) {
                row++;
            } else {
                row = 0;
            }
        } else if (((pad & PADLright) != 0 && (prev & PADLright) == 0) ||
                   ((pad & PADLleft) != 0 && (prev & PADLleft) == 0)) {
            if (row != KF_MENU_CONFIG_RETURN_ROW) {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                goto toggle;
            }
        } else if ((pad & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            if (row == KF_MENU_CONFIG_RETURN_ROW) {
                confirm = 1;
                phase = KF_MENU_PANEL_CLOSED;
            } else {
            toggle:
                states[row] = KF_ENUM_DECODE(KfPlayerOption, states[row] == KF_PLAYER_OPTION_OFF);
            }
        } else if ((pad & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            phase = KF_MENU_PANEL_CLOSED;
        }
        menu_config_panel_draw(option_a, option_b, states);
        menu_draw_window(KF_MENU_WINDOW_CONFIG, KF_MENU_CONFIG_ROW_COUNT, row, confirm);
        menu_present_frame();
    } while (1);

    player_state.audio_effects_enabled = states[KF_MENU_CONFIG_EFFECTS_ROW];
    player_state.audio_music_enabled = states[KF_MENU_CONFIG_MUSIC_ROW];
    player_state.hud_gauges_enabled = states[KF_MENU_CONFIG_GAUGES_ROW];
    player_state.compass_enabled = states[KF_MENU_CONFIG_COMPASS_ROW];
    if (player_state.audio_music_enabled != music_orig) {
        if (player_state.audio_music_enabled == KF_PLAYER_OPTION_OFF) {
            audio_stop_sequence_fade();
        } else {
            audio_play_current_map_sequence();
        }
    }
}

/*
 * Draw four config rows.  Each row places two option boxes at the row's left
 * and right anchors -- highlighting the one whose per-row state equals 1 -- and
 * a label under each, then steps both anchors down 22 pixels.  Finally links
 * the four double-buffered map-viewer frame quads for the current buffer at
 * ordering-table slot 3000.
 */
ADDRESS(0x80025da0, 0x198)
void menu_config_panel_draw(
    MenuGlyphString option_a, MenuGlyphString option_b, KfPlayerOption *values)
{
    s32 i;
    KfPlayerOption *states;
    const MenuSpriteDef *box_b;

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    states = values;
    for (i = 0; i < KF_MENU_CONFIG_SETTING_COUNT; i++) {
        if (*states == KF_PLAYER_OPTION_ON) {
            menu_blit_sprite_translucent(&menu_assets.option_highlight, (const MenuPoint *)&option_a);
            box_b = &menu_assets.option_background;
        } else {
            menu_blit_sprite_translucent(&menu_assets.option_background, (const MenuPoint *)&option_a);
            box_b = &menu_assets.option_highlight;
        }
        menu_blit_sprite_translucent(box_b, (const MenuPoint *)&option_b);
        menu_draw_string(
            &menu_assets.glyph_atlas,
            &option_a);
        menu_draw_string(
            &menu_assets.glyph_atlas,
            &option_b);
        states++;
        option_a.y += CONFIG_OPTION_ROW_STEP;
        option_b.y += CONFIG_OPTION_ROW_STEP;
    }
    AddPrim(&display_state.ordering_table[MENU_BACKGROUND_OT_DEPTH],
            &menu_assets.background_quads[display_state.buffer_index][3]);
    AddPrim(&display_state.ordering_table[MENU_BACKGROUND_OT_DEPTH],
            &menu_assets.background_quads[display_state.buffer_index][2]);
    AddPrim(&display_state.ordering_table[MENU_BACKGROUND_OT_DEPTH],
            &menu_assets.background_quads[display_state.buffer_index][1]);
    AddPrim(&display_state.ordering_table[MENU_BACKGROUND_OT_DEPTH],
            &menu_assets.background_quads[display_state.buffer_index][0]);
}
