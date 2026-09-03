#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

extern void lighting_set_color_matrix(
    const MATRIX *from, const MATRIX *to, s32 blend);

ADDRESS(0x800187a4, 0x4c)
void lighting_apply_weapon9_environment(void)
{
    MATRIX current;

    ReadColorMatrix(&current);
    lighting_set_color_matrix(&current, &color_matrix_table[4], 0x9c4);
    SetFogNear(render_state.fog_near_distance - (render_state.fog_near_distance >> 1), 200);
}
