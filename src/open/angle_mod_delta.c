#include <kf/address.h>
#include <kf/game_math.h>

ADDRESS(0x80015cd4, 0x10)
int angle_mod_delta_le_half_turn(int lhs, int rhs)
{
    return ((lhs - rhs) & KF_ANGLE_WRAP_MASK) < (KF_ANGLE_HALF_TURN + 1);
}
