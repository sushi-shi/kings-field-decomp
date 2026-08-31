#include <kf/game_types.h>

extern s32 DAT_80057b6c;
extern void func_8001c5ec(s32 value);

void func_8002af0c(void)
{
    if (DAT_80057b6c == 1) {
        func_8001c5ec(4);
        DAT_80057b6c = 0;
    }
}
