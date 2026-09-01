#include <kf/game_types.h>

void func_80033de8(s16 red, s16 green, s16 blue, struct KfMatrix *matrix)
{
    matrix->m[0][2] = red;
    matrix->m[0][1] = red;
    matrix->m[0][0] = red;
    matrix->m[1][2] = green;
    matrix->m[1][1] = green;
    matrix->m[1][0] = green;
    matrix->m[2][2] = blue;
    matrix->m[2][1] = blue;
    matrix->m[2][0] = blue;
}
