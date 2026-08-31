#include <kf/game_types.h>

extern u32 DAT_800a0248[];

void func_800150fc(const u32 *source)
{
    u32 *destination = DAT_800a0248;
    s32 count = 294;

    do {
        *destination++ = *source++;
    } while (--count != 0);
}
