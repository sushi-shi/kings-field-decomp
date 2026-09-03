#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

extern void lighting_set_color_matrix(
    const MATRIX *from, const MATRIX *to, s32 blend);

ADDRESS(0x80018824, 0x34)
void lighting_apply_color_preset6(void)
{
    MATRIX current;

    ReadColorMatrix(&current);
    lighting_set_color_matrix(&current, &color_matrix_table[6], 0xc00);
}
