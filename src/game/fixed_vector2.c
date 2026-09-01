#include <kf/address.h>
#include <kf/game_types.h>

extern s32 SquareRoot0(s32 value);

ADDRESS(0x80015068, 0x40)
s32 fixed_vector2_length(s32 x, s32 y)
{
    x >>= 3;
    y >>= 3;
    return SquareRoot0(x * x + y * y) << 3;
}
