#include <kf/address.h>
#include <kf/game_types.h>

struct VecXZ32 {
    int x;
    int unused;
    int z;
};

struct VecXZ16 {
    s16 x;
    s16 z;
};

ADDRESS(0x80014e08)
void vector2s_scale_shift11(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;

    vector[0] = x >> 11;
    vector[1] = y >> 11;
}

ADDRESS(0x80014e48)
void vector3s_scale_shift12(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;
    s32 z = vector[2] * scale;

    vector[0] = x >> 12;
    vector[1] = y >> 12;
    vector[2] = z >> 12;
}

ADDRESS(0x80014ea4)
void vector2s_scale_shift12(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;

    vector[0] = x >> 12;
    vector[1] = y >> 12;
}

ADDRESS(0x80014ee4)
void vector3s_scale_shift12_alt(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;
    s32 z = vector[2] * scale;

    vector[0] = x >> 12;
    vector[1] = y >> 12;
    vector[2] = z >> 12;
}

ADDRESS(0x80014f40)
void vector3i_add_xz(struct VecXZ32 *destination, const struct VecXZ16 *delta)
{
    destination->x += delta->x;
    destination->z += delta->z;
}

ADDRESS(0x80014f6c)
int angle_within_tolerance(int lhs, int rhs, s16 range)
{
    int delta = (lhs - rhs) & 0xfff;

    return range < delta && 0x1000 - range <= delta;
}

ADDRESS(0x80014fa8)
int angle_mod_delta_le_half_turn(int lhs, int rhs)
{
    return ((lhs - rhs) & 0xfff) < 0x801;
}
