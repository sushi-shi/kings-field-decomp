#include <kf/game_types.h>

extern s32 DAT_80095868;
extern s16 DAT_8009586c;
extern void SsVabClose(s16 vab_id);

void func_80032cb0(void)
{
    s16 *vab_id = &DAT_8009586c;

    SsVabClose(*vab_id);
    *vab_id = -1;
    DAT_80095868 = 0;
}
