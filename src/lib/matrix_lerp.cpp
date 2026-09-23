#include <kf/platform/prelude.hpp>
#include <kf/lib/math.h>
#include <kf/lib/render_types.h>
#include <kf/lib/graphics.h>

void matrix_interpolate(
    const MATRIX *from,
    const MATRIX *to,
    MATRIX *output,
    s32 blend)
{
    const s16 *source = &from->m[0][0];
    const s16 *target = &to->m[0][0];
    s16 *destination = &output->m[0][0];
    s16 count = KF_MATRIX_ROTATION_ELEMENTS - 1;

    do {
        u16 from_value = *source++;
        u16 to_value = *target++;

        *destination++ = from_value + ((((s16)to_value - (s16)from_value) * blend) >> KF_FIXED12_BITS);
    } while (--count != -1);
}

void lighting_set_color_matrix(KfRenderState &view,
    const MATRIX *from,
    const MATRIX *to,
    s32 blend)
{
    matrix_interpolate(from, to, &view.lighting.color_matrix, blend);
}

void fog_interpolate_near(KfRenderState &view, s32 start, s32 end, s32 ratio)
{
    s32 distance = (((end - start) * ratio) >> KF_FIXED12_BITS) + start;

    view.fog_near_distance = distance;
    view.projection.fog_near = distance;
}

void fog_set_near(KfRenderState &view, s32 distance)
{
    view.fog_near_distance = distance;
    view.projection.fog_near = distance;
}

void color_lerp_cvector(
    const CVECTOR *from,
    const CVECTOR *to,
    CVECTOR *output,
    s32 blend)
{
    output->r = (((to->r - from->r) * blend) >> KF_FIXED12_BITS) + from->r;
    output->g = (((to->g - from->g) * blend) >> KF_FIXED12_BITS) + from->g;
    output->b = (((to->b - from->b) * blend) >> KF_FIXED12_BITS) + from->b;
}

u16 color_lerp_rgb555(u16 color0, u16 color1, s32 blend)
{
    s32 r0 = color0 & KF_RGB555_CHANNEL_MASK;
    s32 r1 = color1 & KF_RGB555_CHANNEL_MASK;
    s32 r = r0 + (((r1 - r0) * blend) >> KF_FIXED12_BITS);
    s32 g0 = (color0 >> KF_RGB555_GREEN_SHIFT) & KF_RGB555_CHANNEL_MASK;
    s32 g1 = (color1 >> KF_RGB555_GREEN_SHIFT) & KF_RGB555_CHANNEL_MASK;
    s32 g = g0 + (((g1 - g0) * blend) >> KF_FIXED12_BITS);
    s32 b0 = (color0 >> KF_RGB555_BLUE_SHIFT) & KF_RGB555_CHANNEL_MASK;
    s32 b1 = (color1 >> KF_RGB555_BLUE_SHIFT) & KF_RGB555_CHANNEL_MASK;
    s32 b = b0 + (((b1 - b0) * blend) >> KF_FIXED12_BITS);

    return r | ((color0 & KF_RGB555_STP) | (b << KF_RGB555_BLUE_SHIFT) | (g << KF_RGB555_GREEN_SHIFT));
}
