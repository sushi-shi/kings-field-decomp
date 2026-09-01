#include <kf/game_types.h>

extern void func_800202fc(
    const struct KfMatrix *from,
    const struct KfMatrix *to,
    struct KfMatrix *matrix,
    s32 blend);
extern void SetColorMatrix(struct KfMatrix *matrix);

void func_80020364(
    const struct KfMatrix *from,
    const struct KfMatrix *to,
    s32 blend)
{
    struct KfMatrix matrix;

    func_800202fc(from, to, &matrix, blend);
    SetColorMatrix(&matrix);
}
