#include <kf/game_types.h>

extern void func_800202fc(
    const struct KfMatrix *from,
    const struct KfMatrix *to,
    struct KfMatrix *matrix,
    s32 blend);
extern void SetLightMatrix(struct KfMatrix *matrix);

void func_80020390(
    const struct KfMatrix *from,
    const struct KfMatrix *to,
    s32 blend)
{
    struct KfMatrix matrix;

    func_800202fc(from, to, &matrix, blend);
    SetLightMatrix(&matrix);
}
