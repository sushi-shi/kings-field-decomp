#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game_types.h>

/* Psy-Q LIBGTE: int rsin(int a); int rcos(int a). */

/*
 * Psy-Q LIBGTE MATRIX/SVECTOR/VECTOR have the KfMatrix/KfVec4s/KfVec4i layouts:
 * MulMatrix(m0, m1), MulMatrix2(m0, m1), ApplyMatrix(m, v0, v1).
 */
extern void matrix_set_rotation_x(s16 angle, struct KfMatrix *matrix);
extern void matrix_set_rotation_y(s16 angle, struct KfMatrix *matrix);
extern void matrix_set_rotation_z(s16 angle, struct KfMatrix *matrix);

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
        if (target - current <= 0x800) {
            result = current + step;
            if (target < result) {
                return target;
            }
        } else {
            result = (current - step) & 0xfff;
            if (result >= 0x800 && result <= target) {
                return target;
            }
        }
    } else {
        if (current - target <= 0x800) {
            result = current - step;
            if (result < target) {
                return target;
            }
        } else {
            result = (current + step) & 0xfff;
            if (result <= 0x800 && result >= target) {
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
void matrix_set_rotation_x(s16 angle, struct KfMatrix *matrix)
{
    s32 sin = rsin(angle);
    s32 cos = rcos(angle);

    matrix->m[0][0] = 0x1000;
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
void matrix_set_rotation_y(s16 angle, struct KfMatrix *matrix)
{
    s32 sin = rsin(angle);
    s32 cos = rcos(angle);

    matrix->m[0][0] = cos;
    matrix->m[0][1] = 0;
    matrix->m[0][2] = -sin;
    matrix->m[1][0] = 0;
    matrix->m[1][1] = 0x1000;
    matrix->m[1][2] = 0;
    matrix->m[2][0] = sin;
    matrix->m[2][1] = 0;
    matrix->m[2][2] = cos;
}

ADDRESS(0x80014c5c, 0x70)
void matrix_set_rotation_z(s16 angle, struct KfMatrix *matrix)
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
    matrix->m[2][2] = 0x1000;
}

ADDRESS(0x80014ccc, 0x68)
void matrix_set_rotation_yxz(const struct KfEulerAngles *angles, struct KfMatrix *matrix)
{
    struct KfMatrix temporary;

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
void pitch_yaw_to_forward_vector(const struct KfPitchYaw *angles, struct KfVec3s *direction)
{
    struct KfMatrix pitch_matrix;
    struct KfMatrix yaw_matrix;
    struct KfVec4s source;
    struct KfVec4i result;

    source.x = 0;
    source.y = 0;
    source.z = 0x1000;
    matrix_set_rotation_x(-angles->pitch & 0xfff, &pitch_matrix);
    ApplyMatrix(&pitch_matrix, &source, &result);
    source.x = result.x;
    source.y = result.y;
    source.z = result.z;
    matrix_set_rotation_y(angles->yaw, &yaw_matrix);
    ApplyMatrix(&yaw_matrix, &source, &result);
    direction->x = result.x;
    direction->y = result.y;
    direction->z = result.z;
}
