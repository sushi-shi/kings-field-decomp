#include <kf/address.h>
#include <kf/game_math.h>

/*
 * Shared across GAME.EXE and OPEN.EXE: both linked programs contain these three
 * functions with identical code (verified byte-identical modulo relocations).
 * Retail loads each matrix element unsigned, sign-extends both operands of the
 * difference separately, and uses a 16-bit counter running from 8 to -1.
 */

ADDRESS_AT("GAME", 0x800202fc, 0x68)
ADDRESS_AT("OPEN", 0x80019598, 0x68)
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

ADDRESS_AT("GAME", 0x80020364, 0x2c)
ADDRESS_AT("OPEN", 0x80019600, 0x2c)
void lighting_set_color_matrix(
    const MATRIX *from,
    const MATRIX *to,
    s32 blend)
{
    MATRIX matrix;

    matrix_interpolate(from, to, &matrix, blend);
    SetColorMatrix(&matrix);
}

ADDRESS_AT("GAME", 0x80020390, 0x2c)
ADDRESS_AT("OPEN", 0x8001962c, 0x2c)
void lighting_set_light_matrix(
    const MATRIX *from,
    const MATRIX *to,
    s32 blend)
{
    MATRIX matrix;

    matrix_interpolate(from, to, &matrix, blend);
    SetLightMatrix(&matrix);
}
