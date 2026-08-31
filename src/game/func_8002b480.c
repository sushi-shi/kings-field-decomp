#include <kf/game_types.h>

extern s32 DAT_80057e90;
extern s32 DAT_80057e98;
extern s32 DAT_80057ea0;
extern s32 DAT_80057ea8;
extern s32 UnDeliverEvent(s32 event);

void func_8002b480(void)
{
    UnDeliverEvent(DAT_80057e90);
    UnDeliverEvent(DAT_80057e98);
    UnDeliverEvent(DAT_80057ea0);
    UnDeliverEvent(DAT_80057ea8);
}
