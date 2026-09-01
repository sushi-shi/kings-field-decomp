#include <kf/semantic_types.h>

/* LIBGTE.H (Psy-Q Release 2.5): int rsin(int a); int rcos(int a); */
extern s32 rsin(s32 angle);
extern s32 rcos(s32 angle);

void func_80014b2c(s16 angle, struct KfVecXZs *direction)
{
    direction->x = -rsin(angle);
    direction->z = -rcos(angle);
}
