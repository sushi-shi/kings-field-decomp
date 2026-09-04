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
    s32 distance = start;

    distance += ((end - distance) * ratio) >> 12;

    render_state.fog_near_distance = distance;
    SetFogNear(distance, 200);
}

ADDRESS(0x8001969c, 0x28)
void fog_set_near(s32 distance)
{
    render_state.fog_near_distance = distance;
    SetFogNear(distance, 200);
}
