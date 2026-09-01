#include <kf/address.h>
#include <kf/semantic_types.h>

extern void ReadColorMatrix(struct KfMatrix *matrix);
extern void lighting_set_color_matrix(
    const struct KfMatrix *from, const struct KfMatrix *to, s32 blend);
extern void SetFogNear(s32 distance, s32 projection);

extern struct KfMatrix color_matrix_table[7];
extern s32 fog_near_distance;

ADDRESS(0x800187a4, 0x4c)
void lighting_apply_weapon9_environment(void)
{
    struct KfMatrix current;

    ReadColorMatrix(&current);
    lighting_set_color_matrix(&current, &color_matrix_table[4], 0x9c4);
    SetFogNear(fog_near_distance - (fog_near_distance >> 1), 200);
}
