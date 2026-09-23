#include <kf/lib/bool.h>

#include <kf/lib/math.h>

#ifndef KF_OPEN

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

void angle_to_forward_xz(s16 angle, struct KfVecXZs *direction)
{
    direction->x = -rsin(angle);
    direction->z = -rcos(angle);
}
#endif

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

void matrix_set_rotation_yxz(const struct KfEulerAngles *angles, MATRIX *matrix)
{
    MATRIX temporary;

    matrix_set_rotation_z(angles->z, &temporary);
    matrix_set_rotation_x(angles->x, matrix);
    MulMatrix(matrix, &temporary);
    matrix_set_rotation_y(angles->y, &temporary);
    MulMatrix2(&temporary, matrix);
}
