#include <kf/game_types.h>

/*
 * Retail loads every element unsigned (lhu) and sign-extends the operands of
 * the difference separately, and the element counter is a 16-bit value that
 * runs 8..-1.
 */
void func_800202fc(
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
