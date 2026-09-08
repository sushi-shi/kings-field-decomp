#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_math.h>
#include <kf/game_render.h>
#include <kf/game.h>

/*
 * Retail loads every element unsigned (lhu) and sign-extends the operands of
 * the difference separately, and the element counter is a 16-bit value that
 * runs 8..-1.
 */

ADDRESS(0x800202fc, 0x68)
void matrix_interpolate(
    const MATRIX *from,
    const MATRIX *to,
    MATRIX *output,
    s32 blend)
{
    const u16 *source = (const u16 *)from;
    const u16 *target = (const u16 *)to;
    u16 *destination = (u16 *)output;
    s16 count = KF_MATRIX_ROTATION_ELEMENTS - 1;

    do {
        u16 from_value = *source++;
        u16 to_value = *target++;

        *destination++ = from_value + ((((s16)to_value - (s16)from_value) * blend) >> KF_FIXED12_BITS);
    } while (--count != -1);
}

ADDRESS(0x80020364, 0x2c)
void lighting_set_color_matrix(
    const MATRIX *from,
    const MATRIX *to,
    s32 blend)
{
    MATRIX matrix;

    matrix_interpolate(from, to, &matrix, blend);
    SetColorMatrix(&matrix);
}

ADDRESS(0x80020390, 0x2c)
void lighting_set_light_matrix(
    const MATRIX *from,
    const MATRIX *to,
    s32 blend)
{
    MATRIX matrix;

    matrix_interpolate(from, to, &matrix, blend);
    SetLightMatrix(&matrix);
}

ADDRESS(0x800203bc, 0x44)
void fog_interpolate_near(s32 start, s32 end, s32 ratio)
{
    s32 distance = (((end - start) * ratio) >> KF_FIXED12_BITS) + start;

    game_graphics_runtime.render_state.fog_near_distance = distance;
    SetFogNear(distance, KF_DEFAULT_PROJECTION_DISTANCE);
}

ADDRESS(0x80020400, 0x28)
void fog_set_near(s32 distance)
{
    game_graphics_runtime.render_state.fog_near_distance = distance;
    SetFogNear(distance, KF_DEFAULT_PROJECTION_DISTANCE);
}
