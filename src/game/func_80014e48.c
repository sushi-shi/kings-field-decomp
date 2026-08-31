#include <kf/game_types.h>

void func_80014e48(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;
    s32 z = vector[2] * scale;

    vector[0] = x >> 12;
    vector[1] = y >> 12;
    vector[2] = z >> 12;
}
