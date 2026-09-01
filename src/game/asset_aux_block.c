#include <kf/address.h>
#include <kf/game_types.h>

extern u32 asset_aux_block[];

ADDRESS(0x800150fc, 0x2c)
void asset_aux_block_load(const u32 *source)
{
    u32 *destination = asset_aux_block;
    s32 count = 294;

    do {
        *destination++ = *source++;
    } while (--count != 0);
}

ADDRESS(0x80015128, 0x3c)
int fixed6_ratio_step(int value, int divisor)
{
    return (value << 6) / (divisor + 1) + 1;
}
