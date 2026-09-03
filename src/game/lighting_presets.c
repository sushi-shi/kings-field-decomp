#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

ADDRESS(0x800187a4, 0x4c)
void lighting_apply_weapon9_environment(void)
{
    MATRIX current;

    ReadColorMatrix(&current);
    lighting_set_color_matrix(&current, &color_matrix_table[4], 0x9c4);
    SetFogNear(render_state.fog_near_distance - (render_state.fog_near_distance >> 1), 200);
}

ADDRESS(0x800187f0, 0x34)
void lighting_apply_timed_player_effect(void)
{
    MATRIX current;

    ReadColorMatrix(&current);
    lighting_set_color_matrix(&current, &color_matrix_table[5], 0xc00);
}

ADDRESS(0x80018824, 0x34)
void lighting_apply_color_preset6(void)
{
    MATRIX current;

    ReadColorMatrix(&current);
    lighting_set_color_matrix(&current, &color_matrix_table[6], 0xc00);
}
