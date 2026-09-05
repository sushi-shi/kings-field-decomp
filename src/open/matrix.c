#include <kf/address.h>
#include <kf/open_render.h>

/*
 * Retail loads each matrix element unsigned, sign-extends both operands of
 * the difference separately, and uses a 16-bit counter running from 8 to -1.
 */
ADDRESS(0x80019598, 0x68)
void matrix_interpolate(
    const MATRIX *from,
    const MATRIX *to,
    MATRIX *output,
    s32 blend)
{
    const s16 *source = &from->m[0][0];
    const s16 *target = &to->m[0][0];
    s16 *destination = &output->m[0][0];
    s16 count = 8;

    do {
        u16 from_value = (u16)*source++;
        u16 to_value = (u16)*target++;

        *destination++ = from_value + ((((s16)to_value - (s16)from_value) * blend) >> 12);
    } while (--count != -1);
}

ADDRESS(0x80019600, 0x2c)
void lighting_set_color_matrix(
    const MATRIX *from,
    const MATRIX *to,
    s32 blend)
{
    MATRIX matrix;

    matrix_interpolate(from, to, &matrix, blend);
    SetColorMatrix(&matrix);
}

ADDRESS(0x8001962c, 0x2c)
void lighting_set_light_matrix(
    const MATRIX *from,
    const MATRIX *to,
    s32 blend)
{
    MATRIX matrix;

    matrix_interpolate(from, to, &matrix, blend);
    SetLightMatrix(&matrix);
}

ADDRESS(0x80019658, 0x44)
void fog_interpolate_near(s32 start, s32 end, s32 ratio)
{
    s32 distance = (((end - start) * ratio) >> 12) + start;

    open_graphics_runtime.render_state.fog_near_distance = distance;
    SetFogNear(distance, 200);
}

ADDRESS(0x8001969c, 0x28)
void fog_set_near(s32 distance)
{
    open_graphics_runtime.render_state.fog_near_distance = distance;
    SetFogNear(distance, 200);
}

ADDRESS(0x800196c4, 0x88)
void color_lerp_cvector(
    const CVECTOR *from,
    const CVECTOR *to,
    CVECTOR *output,
    s32 blend)
{
    output->r = (((to->r - from->r) * blend) >> 12) + from->r;
    output->g = (((to->g - from->g) * blend) >> 12) + from->g;
    output->b = (((to->b - from->b) * blend) >> 12) + from->b;
}

ADDRESS(0x8001974c, 0x98)
u16 color_lerp_rgb555(u16 color0, u16 color1, s32 blend)
{
    s32 r0 = color0 & 0x1f;
    s32 r1 = color1 & 0x1f;
    s32 r = r0 + (((r1 - r0) * blend) >> 12);
    s32 g0 = (color0 >> 5) & 0x1f;
    s32 g1 = (color1 >> 5) & 0x1f;
    s32 g = g0 + (((g1 - g0) * blend) >> 12);
    s32 b0 = (color0 >> 10) & 0x1f;
    s32 b1 = (color1 >> 10) & 0x1f;
    s32 b = b0 + (((b1 - b0) * blend) >> 12);

    return r | ((color0 & 0x8000) | (b << 10) | (g << 5));
}
