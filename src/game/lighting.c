#include <kf/address.h>
#include <kf/game_render.h>
#include <kf/game.h>

/*
 * Seven colour matrices for lighting/screen fades; entries 0, 3 and 4 are the
 * player-death fade endpoints. Each is a MATRIX (short m[3][3], long t[3]).
 */
DATA(0x80055dbc, 0xe0)
MATRIX color_matrix_table[7] = {
    {{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},
    {{{3000, 1000, 4000}, {200, 70, 400}, {200, 70, 400}}, {0, 0, 0}},
    {{{1000, 350, 2000}, {1000, 350, 2000}, {3000, 1000, 4000}}, {0, 0, 0}},
    {{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},
    {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},
    {{{0, 0, 0}, {4095, 4095, 4095}, {0, 0, 0}}, {0, 0, 0}},
    {{{0, 0, 0}, {0, 0, 0}, {4095, 4095, 4095}}, {0, 0, 0}},
};

ADDRESS(0x80033d80, 0x68)
void lighting_transition_color_matrix(const MATRIX *from, const MATRIX *to)
{
    s32 blend = 0;

    do {
        lighting_set_color_matrix(from, to, blend);
        render_frame(0, 0);
        frame_pacer_wait();
        blend += 0x400;
    } while (blend <= 0x1000);
}

ADDRESS(0x80033de8, 0x28)
void color_matrix_set_rgb(s16 red, s16 green, s16 blue, MATRIX *matrix)
{
    matrix->m[0][2] = red;
    matrix->m[0][1] = red;
    matrix->m[0][0] = red;
    matrix->m[1][2] = green;
    matrix->m[1][1] = green;
    matrix->m[1][0] = green;
    matrix->m[2][2] = blue;
    matrix->m[2][1] = blue;
    matrix->m[2][0] = blue;
}

/* Psy-Q LIBGTE: ReadColorMatrix(MATRIX *). */

/* Cycles the colour matrix green, cyan, white and back while restoring HP and MP. */
ADDRESS(0x80033e10, 0xd4)
void player_restore_vitals_with_color_cycle(void)
{
    MATRIX saved;
    MATRIX first;
    MATRIX second;

    ReadColorMatrix(&saved);
    color_matrix_set_rgb(0, 0xfff, 0, &first);
    lighting_transition_color_matrix(&saved, &first);
    color_matrix_set_rgb(0, 0xfff, 0xfff, &second);
    lighting_transition_color_matrix(&first, &second);
    color_matrix_set_rgb(0xfff, 0xfff, 0xfff, &first);
    lighting_transition_color_matrix(&second, &first);
    lighting_transition_color_matrix(&first, &saved);
    player_state.vitals.current_hp = player_state.vitals.maximum_hp;
    player_state.vitals.current_mp = player_state.vitals.maximum_mp;
    player_state.status_effect_flags &= 0xfff0;
}
