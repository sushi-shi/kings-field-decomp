#include <kf/address.h>
#include <kf/game_math.h>


/*
 * Fixed-point matrix and vector math, one contiguous run
 * 0x80014a64..0x800150a8 (GAME.EXE): axis rotation-matrix builders and the
 * vector/scalar fixed-point helpers. Module boundary is WIP.
 */


/* Psy-Q LIBGTE: int rsin(int a); int rcos(int a). */

/*
 * Psy-Q LIBGTE MATRIX/SVECTOR/VECTOR have the MATRIX/SVECTOR/VECTOR layouts:
 * MulMatrix(m0, m1), MulMatrix2(m0, m1), ApplyMatrix(m, v0, v1).
 */

/*
 * Angles are 12-bit (0..0xfff). Within a half turn the step is applied
 * directly and clamped at the target; beyond it the angle wraps the other
 * way and the wrapped value is clamped only while it stays on the target's
 * side of the half-turn boundary.
 */
ADDRESS(0x80014a64, 0xc8)
s16 angle_approach(s16 current, s16 target, s32 step)
{
    s16 result;

    if (target == current) {
        return target;
    }
    if (current < target) {
        if (target - current <= KF_ANGLE_HALF_TURN) {
            result = current + step;
            if (target < result) {
                return target;
            }
        } else {
            result = (current - step) & KF_ANGLE_WRAP_MASK;
            if (result >= KF_ANGLE_HALF_TURN && result <= target) {
                return target;
            }
        }
    } else {
        if (current - target <= KF_ANGLE_HALF_TURN) {
            result = current - step;
            if (result < target) {
                return target;
            }
        } else {
            result = (current + step) & KF_ANGLE_WRAP_MASK;
            if (result <= KF_ANGLE_HALF_TURN && result >= target) {
                return target;
            }
        }
    }
    return result;
}

ADDRESS(0x80014b2c, 0x50)
void angle_to_forward_xz(s16 angle, struct KfVecXZs *direction)
{
    direction->x = -rsin(angle);
    direction->z = -rcos(angle);
}

ADDRESS(0x80014b7c, 0x70)
void matrix_set_rotation_x(s16 angle, MATRIX *matrix)
{
    s32 sin = rsin(angle);
    s32 cos = rcos(angle);

    matrix->m[0][0] = KF_FIXED12_ONE;
    matrix->m[0][1] = 0;
    matrix->m[0][2] = 0;
    matrix->m[1][0] = 0;
    matrix->m[1][1] = cos;
    matrix->m[1][2] = -sin;
    matrix->m[2][0] = 0;
    matrix->m[2][1] = sin;
    matrix->m[2][2] = cos;
}

ADDRESS(0x80014bec, 0x70)
void matrix_set_rotation_y(s16 angle, MATRIX *matrix)
{
    s32 sin = rsin(angle);
    s32 cos = rcos(angle);

    matrix->m[0][0] = cos;
    matrix->m[0][1] = 0;
    matrix->m[0][2] = -sin;
    matrix->m[1][0] = 0;
    matrix->m[1][1] = KF_FIXED12_ONE;
    matrix->m[1][2] = 0;
    matrix->m[2][0] = sin;
    matrix->m[2][1] = 0;
    matrix->m[2][2] = cos;
}

ADDRESS(0x80014c5c, 0x70)
void matrix_set_rotation_z(s16 angle, MATRIX *matrix)
{
    s32 sin = rsin(angle);
    s32 cos = rcos(angle);

    matrix->m[0][0] = cos;
    matrix->m[0][1] = -sin;
    matrix->m[0][2] = 0;
    matrix->m[1][0] = sin;
    matrix->m[1][1] = cos;
    matrix->m[1][2] = 0;
    matrix->m[2][0] = 0;
    matrix->m[2][1] = 0;
    matrix->m[2][2] = KF_FIXED12_ONE;
}

ADDRESS(0x80014ccc, 0x68)
void matrix_set_rotation_yxz(const struct KfEulerAngles *angles, MATRIX *matrix)
{
    MATRIX temporary;

    matrix_set_rotation_z(angles->z, &temporary);
    matrix_set_rotation_x(angles->x, matrix);
    MulMatrix(matrix, &temporary);
    matrix_set_rotation_y(angles->y, &temporary);
    MulMatrix2(&temporary, matrix);
}

/*
 * Rotates the unit forward vector (0, 0, 0x1000) by pitch then yaw. ApplyMatrix
 * writes a VECTOR of longs; only their low halves are carried on.
 */
ADDRESS(0x80014d34, 0xd4)
void pitch_yaw_to_forward_vector(const struct KfEulerAngles *angles, SVECTOR *direction)
{
    MATRIX pitch_matrix;
    MATRIX yaw_matrix;
    SVECTOR source;
    VECTOR result;

    source.vx = 0;
    source.vy = 0;
    source.vz = KF_FIXED12_ONE;
    matrix_set_rotation_x(-angles->x & KF_ANGLE_WRAP_MASK, &pitch_matrix);
    ApplyMatrix(&pitch_matrix, &source, &result);
    source.vx = result.vx;
    source.vy = result.vy;
    source.vz = result.vz;
    matrix_set_rotation_y(angles->y, &yaw_matrix);
    ApplyMatrix(&yaw_matrix, &source, &result);
    direction->vx = result.vx;
    direction->vy = result.vy;
    direction->vz = result.vz;
}


ADDRESS(0x80014e08, 0x40)
void vector2s_scale_shift11(s16 scale, struct KfVecXZs *vector)
{
    s32 x = vector->x * scale;
    s32 z = vector->z * scale;

    vector->x = x >> KF_FIXED11_BITS;
    vector->z = z >> KF_FIXED11_BITS;
}

ADDRESS(0x80014e48, 0x5c)
void vector3s_scale_shift12(s16 scale, SVECTOR *vector)
{
    s32 x = vector->vx * scale;
    s32 y = vector->vy * scale;
    s32 z = vector->vz * scale;

    vector->vx = x >> KF_FIXED12_BITS;
    vector->vy = y >> KF_FIXED12_BITS;
    vector->vz = z >> KF_FIXED12_BITS;
}

ADDRESS(0x80014ea4, 0x40)
void vector2s_scale_shift12(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;

    vector[0] = x >> KF_FIXED12_BITS;
    vector[1] = y >> KF_FIXED12_BITS;
}

ADDRESS(0x80014ee4, 0x5c)
void vector3s_scale_shift12_alt(s16 scale, s16 *vector)
{
    s32 x = vector[0] * scale;
    s32 y = vector[1] * scale;
    s32 z = vector[2] * scale;

    vector[0] = x >> KF_FIXED12_BITS;
    vector[1] = y >> KF_FIXED12_BITS;
    vector[2] = z >> KF_FIXED12_BITS;
}

ADDRESS(0x80014f40, 0x2c)
void vector3i_add_xz(
    VECTOR *destination, const struct KfVecXZs *delta)
{
    destination->vx += delta->x;
    destination->vz += delta->z;
}

ADDRESS(0x80014f6c, 0x3c)
int angle_within_tolerance(int lhs, int rhs, s16 range)
{
    int delta = (lhs - rhs) & KF_ANGLE_WRAP_MASK;

    return delta <= range || KF_ANGLE_FULL_TURN - range <= delta;
}

ADDRESS(0x80014fa8, 0x10)
int angle_mod_delta_le_half_turn(int lhs, int rhs)
{
    return ((lhs - rhs) & KF_ANGLE_WRAP_MASK) < (KF_ANGLE_HALF_TURN + 1);
}

/* Psy-Q LIBGTE: catan(long) returns a 12-bit angle for a 12-bit fixed ratio. */

/*
 * Heading of the (x, z) offset as a 12-bit angle; the division carries the
 * checked expansion retail keeps for both signs of z.
 */
ADDRESS(0x80014fb8, 0xb0)
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


ADDRESS(0x80015068, 0x40)
s32 fixed_vector2_length(s32 x, s32 y)
{
    x >>= KF_LENGTH_SQUARE_DOWNSHIFT;
    y >>= KF_LENGTH_SQUARE_DOWNSHIFT;
    return SquareRoot0(x * x + y * y) << KF_LENGTH_SQUARE_DOWNSHIFT;
}
