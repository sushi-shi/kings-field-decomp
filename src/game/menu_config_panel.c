#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Config-panel draw request passed by value.  Only the two option-row anchors
 * and the per-row selection array are consumed; the reserved gaps preserve the
 * caller's layout so the anchors and the state pointer land in the original
 * argument slots.
 */
typedef struct ConfigPanelParams {
    MenuPoint pt_a;
    u8 reserved0[20];
    MenuPoint pt_b;
    u8 reserved1[20];
    s32 *states;
} ConfigPanelParams;

void menu_config_panel_draw(ConfigPanelParams params);

extern u32 pad_read();
extern void menu_draw_window(s32 kind, s32 count, s32 highlight, s32 flag);

/*
 * Config/options panel top loop reached from the option root: sets up the two
 * option-column labels and the four audio/config toggles copied from the player
 * state, then each frame draws the panel and its window, reads the pad, moves
 * the selected row on left/right, toggles the row's state on up/down, and
 * commits (L1/R1).  On commit it writes the toggles back and starts or stops
 * the map music when the music flag changed.
 */
ADDRESS(0x8002589c, 0x504)
void menu_config_panel(void)
{
    ConfigPanelParams params;
    s32 states[4];
    MenuGlyphString *la;
    MenuGlyphString *lb;
    s32 row;
    s32 confirm;
    s32 phase;
    u32 prev;
    u32 pad;
    s32 music_orig;

    row = 0;
    confirm = 0;
    phase = -99;
    prev = 0;
    while (pad_read(1) != 0) {
    }

    la = (MenuGlyphString *)&params.pt_a;
    lb = (MenuGlyphString *)&params.pt_b;
    la->x = 0xb4;
    la->y = 0x29;
    la->codes[0] = 0xf9;
    la->codes[1] = 0xfa;
    la->codes[2] = -1;
    lb->x = 0xf0;
    lb->y = 0x29;
    lb->codes[0] = 0xf9;
    lb->codes[1] = 0xfb;
    lb->codes[2] = 0xfb;
    lb->codes[3] = -1;
    states[0] = player_state.audio_effects_enabled;
    music_orig = player_state.audio_music_enabled;
    states[1] = music_orig;
    states[2] = player_state.unknown_98[0];
    states[3] = player_state.unknown_98[1];
    params.states = states;

    menu_frame_begin();
    do {
        menu_config_panel_draw(params);
        menu_draw_window(6, 5, row, confirm);
        menu_present_frame();
        if (confirm == 1 || phase == -1) {
            menu_frame_begin();
            menu_config_panel_draw(params);
            menu_draw_window(6, 5, row, confirm);
            menu_present_frame();
            while (pad_read(1) != 0) {
            }
        }
        if (phase != -99) {
            player_state.audio_music_enabled = states[1];
            player_state.unknown_98[0] = states[2];
            player_state.unknown_98[1] = states[3];
            player_state.audio_effects_enabled = states[0];
            if ((states[1] & 0xff) != (u32)music_orig) {
                if ((states[1] & 0xff) == 0) {
                    audio_stop_sequence_fade();
                } else {
                    audio_play_current_map_sequence();
                }
            }
            return;
        }
        confirm = 0;
        menu_frame_begin();
        pad = pad_read();
        if ((pad & 0x1000) == 0 || (prev & 0x1000) != 0) {
            if ((pad & 0x4000) == 0 || (prev & 0x4000) != 0) {
                if (((pad & 0x2000) == 0 || (prev & 0x2000) != 0) &&
                    ((pad & 0x8000) == 0 || (prev & 0x8000) != 0)) {
                    if ((pad & 0x20) == 0 || (prev & 0x20) != 0) {
                        if ((pad & 0x40) == 0 || (prev & 0x40) != 0) {
                            goto no_input;
                        }
                        menu_play_input_sound(0);
                    } else {
                        menu_play_input_sound(0);
                        if (row != 4) {
                            goto toggle;
                        }
                        confirm = 1;
                    }
                    phase = -1;
                } else if (row != 4) {
                    menu_play_input_sound(0);
                toggle:
                    states[row] = (states[row] == 0);
                }
            } else {
                menu_play_input_sound(0);
                row = (row == 4) ? 0 : row + 1;
            }
        } else {
            menu_play_input_sound(0);
            row = (row == 0) ? 4 : row - 1;
        }
    no_input:
        prev = pad;
    } while (1);
}

/*
 * Draw four config rows.  Each row places two option boxes at the row's left
 * and right anchors -- highlighting the one whose per-row state equals 1 -- and
 * a label under each, then steps both anchors down 22 pixels.  Finally links
 * the four double-buffered map-viewer frame quads for the current buffer at
 * ordering-table slot 3000.
 */
ADDRESS(0x80025da0, 0x198)
void menu_config_panel_draw(ConfigPanelParams params)
{
    s32 i;
    s32 *states;
    const MenuSpriteDef *box_b;

    current_poly_ft4 = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    states = params.states;
    for (i = 0; i < 4; i++) {
        if (*states == 1) {
            menu_blit_sprite_translucent(&DAT_80058418, &params.pt_a);
            box_b = &DAT_8005840c;
        } else {
            menu_blit_sprite_translucent(&DAT_8005840c, &params.pt_a);
            box_b = &DAT_80058418;
        }
        menu_blit_sprite_translucent(box_b, &params.pt_b);
        menu_draw_string(
            &DAT_800583f4,
            (const MenuGlyphString *)&params.pt_a);
        menu_draw_string(
            &DAT_800583f4,
            (const MenuGlyphString *)&params.pt_b);
        states++;
        params.pt_a.y += 22;
        params.pt_b.y += 22;
    }
    AddPrim(&display_state.ordering_table[3000],
            &DAT_800580e8[display_state.buffer_index][3]);
    AddPrim(&display_state.ordering_table[3000],
            &DAT_800580e8[display_state.buffer_index][2]);
    AddPrim(&display_state.ordering_table[3000],
            &DAT_800580e8[display_state.buffer_index][1]);
    AddPrim(&display_state.ordering_table[3000],
            &DAT_800580e8[display_state.buffer_index][0]);
}
