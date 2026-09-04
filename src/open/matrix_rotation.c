#include <kf/address.h>
#include <kf/game_math.h>
#include <kf/open_render.h>

RODATA(0x80012038, 0x24)

static const char debug_matrix_label[16] = "Dump Matrix\n";

ADDRESS(0x800158d0, 0x70)
void matrix_set_rotation_x(s16 angle, MATRIX *matrix)
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

ADDRESS(0x80015940, 0x70)
void matrix_set_rotation_y(s16 angle, MATRIX *matrix)
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

ADDRESS(0x800159b0, 0x70)
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
    matrix->m[2][2] = 0x1000;
}

ADDRESS(0x80015a20, 0x68)
void matrix_set_rotation_yxz(const struct KfEulerAngles *angles, MATRIX *matrix)
{
    MATRIX temporary;

    matrix_set_rotation_z(angles->z, &temporary);
    matrix_set_rotation_x(angles->x, matrix);
    MulMatrix(matrix, &temporary);
    matrix_set_rotation_y(angles->y, &temporary);
    MulMatrix2(&temporary, matrix);
}

ADDRESS(0x80015a88, 0x84)
void debug_dump_matrix(const MATRIX *matrix)
{
    debug_printf(debug_matrix_label);
    debug_printf("  [%05d,%05d,%05d]\n",
        matrix->m[0][0], matrix->m[0][1], matrix->m[0][2]);
    debug_printf("  [%05d,%05d,%05d]\n",
        matrix->m[1][0], matrix->m[1][1], matrix->m[1][2]);
    debug_printf("  [%05d,%05d,%05d]\n",
        matrix->m[2][0], matrix->m[2][1], matrix->m[2][2]);
}

ADDRESS(0x80015b0c, 0xd4)
void pitch_yaw_to_forward_vector(
    const struct KfPitchYaw *angles, struct KfVec3s *direction)
{
    MATRIX pitch_matrix;
    MATRIX yaw_matrix;
    SVECTOR source;
    VECTOR result;

    source.vx = 0;
    source.vy = 0;
    source.vz = 0x1000;
    matrix_set_rotation_x(-angles->pitch & 0xfff, &pitch_matrix);
    ApplyMatrix(&pitch_matrix, &source, &result);
    source.vx = result.vx;
    source.vy = result.vy;
    source.vz = result.vz;
    matrix_set_rotation_y(angles->yaw, &yaw_matrix);
    ApplyMatrix(&yaw_matrix, &source, &result);
    direction->x = result.vx;
    direction->y = result.vy;
    direction->z = result.vz;
}
