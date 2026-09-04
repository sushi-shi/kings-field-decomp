#include <kf/address.h>
#include <kf/game_math.h>

ADDRESS(0x80015be0, 0x5c)
void vector3s_scale_shift12(s16 scale, struct KfVec3s *vector)
{
    s32 x = vector->x * scale;
    s32 y = vector->y * scale;
    s32 z = vector->z * scale;

    vector->x = x >> 12;
    vector->y = y >> 12;
    vector->z = z >> 12;
}

ADDRESS(0x80015c3c, 0x5c)
void vector3s_scale_shift12_alt(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;
    s32 z = vector[2] * scale;

    vector[0] = x >> 12;
    vector[1] = y >> 12;
    vector[2] = z >> 12;
}

ADDRESS(0x80015c98, 0x3c)
int angle_within_tolerance(int lhs, int rhs, s16 range)
{
    int delta = (lhs - rhs) & 0xfff;

    return delta <= range || 0x1000 - range <= delta;
}
