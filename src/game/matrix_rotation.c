#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game_types.h>

/* LIBGTE.H (Psy-Q Release 2.5): int rsin(int a); int rcos(int a); */
extern s32 rsin(s32 angle);
extern s32 rcos(s32 angle);

/* Psy-Q LIBGTE: int rsin(int a); int rcos(int a); */

/* Psy-Q LIBGTE MATRIX has the KfMatrix layout: MulMatrix(m0, m1), MulMatrix2(m0, m1). */
extern struct KfMatrix *MulMatrix(struct KfMatrix *m0, struct KfMatrix *m1);
extern struct KfMatrix *MulMatrix2(struct KfMatrix *m0, struct KfMatrix *m1);
extern void matrix_set_rotation_x(s16 angle, struct KfMatrix *matrix);
extern void matrix_set_rotation_y(s16 angle, struct KfMatrix *matrix);
extern void matrix_set_rotation_z(s16 angle, struct KfMatrix *matrix);

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
