#include <kf/address.h>
#include <kf/game_types.h>

/*
 * Retail loads every element unsigned (lhu) and sign-extends the operands of
 * the difference separately, and the element counter is a 16-bit value that
 * runs 8..-1.
 */

extern void matrix_interpolate(
    const struct KfMatrix *from,
    const struct KfMatrix *to,
    struct KfMatrix *matrix,
    s32 blend);
extern void SetColorMatrix(struct KfMatrix *matrix);
extern void SetLightMatrix(struct KfMatrix *matrix);

extern s32 fog_near_distance;
extern void SetFogNear(s32 distance, s32 projection);

ADDRESS(0x800202fc, 0x68)
void matrix_interpolate(
    const struct KfMatrix *from,
    const struct KfMatrix *to,
    struct KfMatrix *output,
    s32 blend)
{
    const u16 *source = (const u16 *)from;
    const u16 *target = (const u16 *)to;
    u16 *destination = (u16 *)output;
    s16 count = 8;

    do {
        u16 from_value = *source++;
        u16 to_value = *target++;

        *destination++ = from_value + ((((s16)to_value - (s16)from_value) * blend) >> 12);
    } while (--count != -1);
}

ADDRESS(0x80020364, 0x2c)
void lighting_set_color_matrix(
    const struct KfMatrix *from,
    const struct KfMatrix *to,
    s32 blend)
{
    struct KfMatrix matrix;

    matrix_interpolate(from, to, &matrix, blend);
    SetColorMatrix(&matrix);
}

ADDRESS(0x80020390, 0x2c)
void lighting_set_light_matrix(
    const struct KfMatrix *from,
    const struct KfMatrix *to,
    s32 blend)
{
    struct KfMatrix matrix;

    matrix_interpolate(from, to, &matrix, blend);
    SetLightMatrix(&matrix);
}

ADDRESS(0x800203bc, 0x44)
void fog_interpolate_near(s32 start, s32 end, s32 ratio)
{
    s32 distance = start;

    distance += ((end - distance) * ratio) >> 12;

    fog_near_distance = distance;
    SetFogNear(distance, 200);
}

ADDRESS(0x80020400, 0x28)
void fog_set_near(s32 distance)
{
    fog_near_distance = distance;
    SetFogNear(distance, 200);
}
