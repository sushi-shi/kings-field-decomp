#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

/* The two labels are private by-value copies; only their Y positions advance. */
void menu_config_panel_draw(
    MenuGlyphString option_a, MenuGlyphString option_b, s32 *values);


/*
 * Four editable config rows and an exit row. Both exit inputs publish the
 * working values after the final redraw/release wait; a changed music flag
 * starts or stops the current map sequence.
 */
ADDRESS(0x8002589c, 0x504)
void menu_config_panel(void)
{
    s32 states[4];
    MenuGlyphString option_a;
    MenuGlyphString option_b;
    s32 row = 0;
    s32 confirm = 0;
    u32 pad = 0;
    u32 prev;
    s32 phase = -99;
    s32 music_orig;

    while (PadRead(1) != 0) {
    }

    option_a.x = 0xb4;
    option_a.y = 0x29;
    option_a.codes[0] = 0xf9;
    option_a.codes[1] = 0xfa;
    option_a.codes[2] = MENU_TEXT_END;
    option_b.x = 0xf0;
    option_b.y = 0x29;
    option_b.codes[0] = 0xf9;
    option_b.codes[1] = 0xfb;
    option_b.codes[2] = 0xfb;
    option_b.codes[3] = MENU_TEXT_END;
    states[0] = player_state.audio_effects_enabled;
    states[1] = player_state.audio_music_enabled;
    states[2] = player_state.hud_gauges_enabled;
    states[3] = player_state.compass_enabled;
    music_orig = states[1];

    menu_frame_begin();
    menu_config_panel_draw(option_a, option_b, states);
    menu_draw_window(6, 5, row, confirm);
    menu_present_frame();
    do {
        if (confirm == 1 || phase == -1) {
            menu_frame_begin();
            menu_config_panel_draw(option_a, option_b, states);
            menu_draw_window(6, 5, row, confirm);
            menu_present_frame();
            while (PadRead(1) != 0) {
            }
        }
        if (phase != -99) {
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
                row = 4;
            }
        } else if ((pad & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (row != 4) {
                row++;
            } else {
                row = 0;
            }
        } else if (((pad & PADLright) != 0 && (prev & PADLright) == 0) ||
                   ((pad & PADLleft) != 0 && (prev & PADLleft) == 0)) {
            if (row != 4) {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                goto toggle;
            }
        } else if ((pad & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            if (row == 4) {
                confirm = 1;
                phase = -1;
            } else {
            toggle:
                states[row] = (states[row] == 0);
            }
        } else if ((pad & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            phase = -1;
        }
        menu_config_panel_draw(option_a, option_b, states);
        menu_draw_window(6, 5, row, confirm);
        menu_present_frame();
    } while (1);

    player_state.audio_effects_enabled = states[0];
    player_state.audio_music_enabled = states[1];
    player_state.hud_gauges_enabled = states[2];
    player_state.compass_enabled = states[3];
    if (player_state.audio_music_enabled != music_orig) {
        if (player_state.audio_music_enabled == 0) {
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
    MenuGlyphString option_a, MenuGlyphString option_b, s32 *values)
{
    s32 i;
    s32 *states;
    const MenuSpriteDef *box_b;

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    states = values;
    for (i = 0; i < 4; i++) {
        if (*states == 1) {
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
        option_a.y += 22;
        option_b.y += 22;
    }
    AddPrim(&display_state.ordering_table[3000],
            &menu_assets.background_quads[display_state.buffer_index][3]);
    AddPrim(&display_state.ordering_table[3000],
            &menu_assets.background_quads[display_state.buffer_index][2]);
    AddPrim(&display_state.ordering_table[3000],
            &menu_assets.background_quads[display_state.buffer_index][1]);
    AddPrim(&display_state.ordering_table[3000],
            &menu_assets.background_quads[display_state.buffer_index][0]);
}
