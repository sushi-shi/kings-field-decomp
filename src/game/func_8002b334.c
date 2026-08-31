#include <kf/game_types.h>

extern void func_8002b36c(void);
extern s32 _card_info(s32 channel);
extern s32 func_8002b3c4(void);

s32 func_8002b334(void)
{
    func_8002b36c();
    if (_card_info(0) != 0) {
        return func_8002b3c4();
    }
    return 0;
}
