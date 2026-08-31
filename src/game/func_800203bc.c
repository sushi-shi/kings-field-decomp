#include <kf/game_types.h>

extern s32 DAT_80095740;
extern void SetFogNear(s32 distance, s32 projection);

void func_800203bc(s32 start, s32 end, s32 ratio)
{
    s32 distance = start;

    distance += ((end - distance) * ratio) >> 12;

    DAT_80095740 = distance;
    SetFogNear(distance, 200);
}
