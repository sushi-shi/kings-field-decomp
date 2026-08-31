#include <kf/game_types.h>

extern s32 DAT_80057e90;
extern s32 DAT_80057e98;
extern s32 DAT_80057ea0;
extern s32 DAT_80057ea8;
extern s32 TestEvent(s32 event);

void func_8002b36c(void)
{
    TestEvent(DAT_80057e90);
    TestEvent(DAT_80057e98);
    TestEvent(DAT_80057ea0);
    TestEvent(DAT_80057ea8);
}
