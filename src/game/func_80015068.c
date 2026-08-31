#include <kf/game_types.h>

extern s32 SquareRoot0(s32 value);

s32 func_80015068(s32 x, s32 y)
{
    x >>= 3;
    y >>= 3;
    return SquareRoot0(x * x + y * y) << 3;
}
