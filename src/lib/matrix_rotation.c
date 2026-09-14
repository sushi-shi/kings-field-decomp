#include <kf/lib/bool.h>
#include <kf/lib/address.h>
#include <kf/lib/math.h>


/*
 * Fixed-point angle and rotation-matrix builders. The following direction and
 * angle helpers continue in vector_math.c; the module boundary remains WIP.
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
#ifndef KF_OPEN
ADDRESS_AT("GAME", 0x80014a64, 0xc8)
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

ADDRESS_AT("GAME", 0x80014b2c, 0x50)
void angle_to_forward_xz(s16 angle, struct KfVecXZs *direction)
{
    direction->x = -rsin(angle);
    direction->z = -rcos(angle);
}
#endif

ADDRESS_AT("GAME", 0x80014b7c, 0x70)
ADDRESS_AT("OPEN", 0x800158d0, 0x70)
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

ADDRESS_AT("GAME", 0x80014bec, 0x70)
ADDRESS_AT("OPEN", 0x80015940, 0x70)
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

ADDRESS_AT("GAME", 0x80014c5c, 0x70)
ADDRESS_AT("OPEN", 0x800159b0, 0x70)
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

ADDRESS_AT("GAME", 0x80014ccc, 0x68)
ADDRESS_AT("OPEN", 0x80015a20, 0x68)
void matrix_set_rotation_yxz(const struct KfEulerAngles *angles, MATRIX *matrix)
{
    MATRIX temporary;

    matrix_set_rotation_z(angles->z, &temporary);
    matrix_set_rotation_x(angles->x, matrix);
    MulMatrix(matrix, &temporary);
    matrix_set_rotation_y(angles->y, &temporary);
    MulMatrix2(&temporary, matrix);
}
