#include <kf/game_math.h>
#include <kf/open_render.h>

void fog_interpolate_near(s32 start, s32 end, s32 ratio)
{
    s32 distance = (((end - start) * ratio) >> KF_FIXED12_BITS) + start;

    open_graphics_runtime.render_state.fog_near_distance = distance;
    SetFogNear(distance, KF_DEFAULT_PROJECTION_DISTANCE);
}

void fog_set_near(s32 distance)
{
    open_graphics_runtime.render_state.fog_near_distance = distance;
    SetFogNear(distance, KF_DEFAULT_PROJECTION_DISTANCE);
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
