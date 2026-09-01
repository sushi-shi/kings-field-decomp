#include <kf/game_types.h>

/* Psy-Q LIBGTE: int rsin(int a); int rcos(int a); */
extern s32 rsin(s32 angle);
extern s32 rcos(s32 angle);

void func_80014c5c(s16 angle, struct KfMatrix *matrix)
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
