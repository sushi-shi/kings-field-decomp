#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfRenderState render_state;

extern void ReadColorMatrix(struct KfMatrix *matrix);
extern void lighting_set_color_matrix(
    const struct KfMatrix *from, const struct KfMatrix *to, s32 blend);
extern void SetFogNear(s32 distance, s32 projection);

extern struct KfMatrix color_matrix_table[7];

ADDRESS(0x800187a4, 0x4c)
void lighting_apply_weapon9_environment(void)
{
    struct KfMatrix current;

    ReadColorMatrix(&current);
    lighting_set_color_matrix(&current, &color_matrix_table[4], 0x9c4);
    SetFogNear(render_state.fog_near_distance - (render_state.fog_near_distance >> 1), 200);
}
