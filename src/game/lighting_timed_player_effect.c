#include <kf/address.h>
#include <kf/semantic_types.h>

extern void lighting_set_color_matrix(
    const MATRIX *from, const MATRIX *to, s32 blend);

extern MATRIX color_matrix_table[7];

ADDRESS(0x800187f0, 0x34)
void lighting_apply_timed_player_effect(void)
{
    MATRIX current;

    ReadColorMatrix(&current);
    lighting_set_color_matrix(&current, &color_matrix_table[5], 0xc00);
}
