#include <kf/game_types.h>

extern s32 DAT_80095740;
extern void SetFogNear(s32 distance, s32 projection);

void func_80020400(s32 distance)
{
    DAT_80095740 = distance;
    SetFogNear(distance, 200);
}
