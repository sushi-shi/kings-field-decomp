#include <kf/address.h>
#include <kf/semantic_types.h>

extern void ReadColorMatrix(struct KfMatrix *matrix);
extern void lighting_set_color_matrix(
    const struct KfMatrix *from, const struct KfMatrix *to, s32 blend);

extern struct KfMatrix color_matrix_table[7];

ADDRESS(0x80018824, 0x34)
void lighting_apply_color_preset6(void)
{
    struct KfMatrix current;

    ReadColorMatrix(&current);
    lighting_set_color_matrix(&current, &color_matrix_table[6], 0xc00);
}
