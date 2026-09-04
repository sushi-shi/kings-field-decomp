#include <kf/address.h>
#include <kf/game_math.h>

ADDRESS(0x80015d94, 0x40)
s32 fixed_vector2_length(s32 x, s32 y)
{
    x >>= 3;
    y >>= 3;
    return SquareRoot0(x * x + y * y) << 3;
}
