#include <kf/address.h>
#include <kf/game_math.h>

ADDRESS(0x80015be0, 0x5c)
void vector3s_scale_shift12(s16 scale, SVECTOR *vector)
{
    s32 x = vector->vx * scale;
    s32 y = vector->vy * scale;
    s32 z = vector->vz * scale;

    setVector(vector, x >> KF_FIXED12_BITS, y >> KF_FIXED12_BITS, z >> KF_FIXED12_BITS);
}

ADDRESS(0x80015c3c, 0x5c)
void vector3s_scale_shift12_alt(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;
    s32 z = vector[2] * scale;

    vector[0] = x >> KF_FIXED12_BITS;
    vector[1] = y >> KF_FIXED12_BITS;
    vector[2] = z >> KF_FIXED12_BITS;
}

ADDRESS(0x80015c98, 0x3c)
int angle_within_tolerance(int lhs, int rhs, s16 range)
{
    int delta = (lhs - rhs) & KF_ANGLE_WRAP_MASK;

    return delta <= range || KF_ANGLE_FULL_TURN - range <= delta;
}
