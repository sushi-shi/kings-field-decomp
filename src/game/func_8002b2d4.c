#include <kf/game_types.h>

extern s32 DAT_80057e90;
extern s32 DAT_80057e98;
extern s32 DAT_80057ea0;
extern s32 DAT_80057ea8;
extern void StopCARD2(void);
extern s32 CloseEvent(s32 event);

void func_8002b2d4(void)
{
    StopCARD2();
    CloseEvent(DAT_80057e90);
    CloseEvent(DAT_80057e98);
    CloseEvent(DAT_80057ea0);
    CloseEvent(DAT_80057ea8);
}
