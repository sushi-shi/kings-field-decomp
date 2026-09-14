#include <kf/bool.h>
#include <kf/address.h>
#include <kf/game_math.h>

/*
 * Fixed-point direction and angle helpers adjacent to the rotation builders
 * in GAME.EXE and OPEN.EXE. Image-only helpers remain in their linked order.
 */

ADDRESS_AT("GAME", 0x80014d34, 0xd4)
ADDRESS_AT("OPEN", 0x80015b0c, 0xd4)
void pitch_yaw_to_forward_vector(const struct KfEulerAngles *angles, SVECTOR *direction)
{
    MATRIX pitch_matrix;
    MATRIX yaw_matrix;
    SVECTOR source;
    VECTOR result;

    setVector(&source, 0, 0, KF_FIXED12_ONE);
    matrix_set_rotation_x(-angles->x & KF_ANGLE_WRAP_MASK, &pitch_matrix);
    ApplyMatrix(&pitch_matrix, &source, &result);
    copyVector(&source, &result);
    matrix_set_rotation_y(angles->y, &yaw_matrix);
    ApplyMatrix(&yaw_matrix, &source, &result);
    copyVector(direction, &result);
}

#ifndef KF_OPEN
ADDRESS_AT("GAME", 0x80014e08, 0x40)
void vector2s_scale_shift11(s16 scale, struct KfVecXZs *vector)
{
    s32 x = vector->x * scale;
    s32 z = vector->z * scale;

    vector->x = x >> KF_FIXED11_BITS;
    vector->z = z >> KF_FIXED11_BITS;
}
#endif

ADDRESS_AT("GAME", 0x80014e48, 0x5c)
ADDRESS_AT("OPEN", 0x80015be0, 0x5c)
void vector3s_scale_shift12(s16 scale, SVECTOR *vector)
{
    s32 x = vector->vx * scale;
    s32 y = vector->vy * scale;
    s32 z = vector->vz * scale;

    setVector(vector, x >> KF_FIXED12_BITS, y >> KF_FIXED12_BITS, z >> KF_FIXED12_BITS);
}

#ifndef KF_OPEN
ADDRESS_AT("GAME", 0x80014ea4, 0x40)
void vector2s_scale_shift12(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;

    vector[0] = x >> KF_FIXED12_BITS;
    vector[1] = y >> KF_FIXED12_BITS;
}
#endif

ADDRESS_AT("GAME", 0x80014ee4, 0x5c)
ADDRESS_AT("OPEN", 0x80015c3c, 0x5c)
void vector3s_scale_shift12_alt(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;
    s32 z = vector[2] * scale;

    vector[0] = x >> KF_FIXED12_BITS;
    vector[1] = y >> KF_FIXED12_BITS;
    vector[2] = z >> KF_FIXED12_BITS;
}

#ifndef KF_OPEN
ADDRESS_AT("GAME", 0x80014f40, 0x2c)
void vector3i_add_xz(VECTOR *destination, const struct KfVecXZs *delta)
{
    destination->vx += delta->x;
    destination->vz += delta->z;
}
#endif

ADDRESS_AT("GAME", 0x80014f6c, 0x3c)
ADDRESS_AT("OPEN", 0x80015c98, 0x3c)
KfBool angle_within_tolerance(int lhs, int rhs, s16 range)
{
    int delta = (lhs - rhs) & KF_ANGLE_WRAP_MASK;

    return delta <= range || KF_ANGLE_FULL_TURN - range <= delta;
}

ADDRESS_AT("GAME", 0x80014fa8, 0x10)
ADDRESS_AT("OPEN", 0x80015cd4, 0x10)
KfBool angle_mod_delta_le_half_turn(int lhs, int rhs)
{
    return ((lhs - rhs) & KF_ANGLE_WRAP_MASK) < (KF_ANGLE_HALF_TURN + 1);
}

/* Psy-Q LIBGTE: catan(long) returns a 12-bit angle for a 12-bit fixed ratio. */
ADDRESS_AT("GAME", 0x80014fb8, 0xb0)
ADDRESS_AT("OPEN", 0x80015ce4, 0xb0)
s32 vector_xz_to_angle(s32 x, s32 z)
{
    if (z > 0) {
        return catan((x << KF_FIXED12_BITS) / z) + KF_ANGLE_HALF_TURN;
    }
    if (z < 0) {
        return catan((x << KF_FIXED12_BITS) / z) & KF_ANGLE_WRAP_MASK;
    }
    if (x > 0) {
        return KF_ANGLE_THREE_QUARTER_TURN;
    }
    return KF_ANGLE_QUARTER_TURN;
}

ADDRESS_AT("GAME", 0x80015068, 0x40)
ADDRESS_AT("OPEN", 0x80015d94, 0x40)
s32 fixed_vector2_length(s32 x, s32 y)
{
    x >>= KF_LENGTH_SQUARE_DOWNSHIFT;
    y >>= KF_LENGTH_SQUARE_DOWNSHIFT;
    return SquareRoot0(x * x + y * y) << KF_LENGTH_SQUARE_DOWNSHIFT;
}
