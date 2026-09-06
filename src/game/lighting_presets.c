#include <kf/address.h>
#include <kf/game_render.h>

enum {
    WEAPON9_COLOR_BLEND = 2500,
    LIGHTING_EFFECT_BLEND = 0xc00
};

ADDRESS(0x800187a4, 0x4c)
void lighting_apply_weapon9_environment(void)
{
    MATRIX current;

    ReadColorMatrix(&current);
    lighting_set_color_matrix(
        &current, &color_matrix_table[KF_GAME_COLOR_BLACK], WEAPON9_COLOR_BLEND);
    SetFogNear(render_state.fog_near_distance - (render_state.fog_near_distance >> 1),
        KF_DEFAULT_PROJECTION_DISTANCE);
}

ADDRESS(0x800187f0, 0x34)
void lighting_apply_timed_player_effect(void)
{
    MATRIX current;

    ReadColorMatrix(&current);
    lighting_set_color_matrix(
        &current, &color_matrix_table[KF_GAME_COLOR_GREEN], LIGHTING_EFFECT_BLEND);
}

ADDRESS(0x80018824, 0x34)
void lighting_apply_color_preset6(void)
{
    MATRIX current;

    ReadColorMatrix(&current);
    lighting_set_color_matrix(
        &current, &color_matrix_table[KF_GAME_COLOR_BLUE], LIGHTING_EFFECT_BLEND);
}
