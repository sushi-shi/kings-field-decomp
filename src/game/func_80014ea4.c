#include <kf/game_types.h>

void func_80014ea4(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;

    vector[0] = x >> 12;
    vector[1] = y >> 12;
}
