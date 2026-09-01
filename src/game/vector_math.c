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

ADDRESS(0x80014e08, 0x40)
void vector2s_scale_shift11(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;

    vector[0] = x >> 11;
    vector[1] = y >> 11;
}

ADDRESS(0x80014e48, 0x5c)
void vector3s_scale_shift12(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;
    s32 z = vector[2] * scale;

    vector[0] = x >> 12;
    vector[1] = y >> 12;
    vector[2] = z >> 12;
}

ADDRESS(0x80014ea4, 0x40)
void vector2s_scale_shift12(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;

    vector[0] = x >> 12;
    vector[1] = y >> 12;
}

ADDRESS(0x80014ee4, 0x5c)
void vector3s_scale_shift12_alt(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;
    s32 z = vector[2] * scale;

    vector[0] = x >> 12;
    vector[1] = y >> 12;
    vector[2] = z >> 12;
}

ADDRESS(0x80014f40, 0x2c)
void vector3i_add_xz(struct VecXZ32 *destination, const struct VecXZ16 *delta)
{
    destination->x += delta->x;
    destination->z += delta->z;
}

ADDRESS(0x80014f6c, 0x3c)
int angle_within_tolerance(int lhs, int rhs, s16 range)
{
    int delta = (lhs - rhs) & 0xfff;

    return range < delta && 0x1000 - range <= delta;
}

ADDRESS(0x80014fa8, 0x10)
int angle_mod_delta_le_half_turn(int lhs, int rhs)
{
    return ((lhs - rhs) & 0xfff) < 0x801;
}

/* Psy-Q LIBGTE: catan(long) returns a 12-bit angle for a 12-bit fixed ratio. */
extern s32 catan(s32 ratio);

/*
 * Heading of the (x, z) offset as a 12-bit angle; the division carries the
 * checked expansion retail keeps for both signs of z.
 */
ADDRESS(0x80014fb8, 0xb0)
u16 vector_xz_to_angle(s32 x, s32 z)
{
    if (z > 0) {
        return catan((x << 12) / z) + 0x800;
    }
    if (z < 0) {
        return catan((x << 12) / z) & 0xfff;
    }
    if (x > 0) {
        return 0xc00;
    }
    return 0x400;
}
