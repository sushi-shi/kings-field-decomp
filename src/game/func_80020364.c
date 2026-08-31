#include <kf/game_types.h>

extern void func_800202fc(s32 first, s32 second, struct KfMatrix *matrix, s32 third);
extern void SetColorMatrix(struct KfMatrix *matrix);

void func_80020364(s32 first, s32 second, s32 third)
{
    struct KfMatrix matrix;

    func_800202fc(first, second, &matrix, third);
    SetColorMatrix(&matrix);
}
