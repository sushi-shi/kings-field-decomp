#include <kf/game_types.h>

extern s16 DAT_80095874;
extern void func_80032cb0(void);
extern void func_8004b6e0(s32 value);
extern void SsEnd(void);

void func_80032c78(void)
{
    func_80032cb0();
    func_8004b6e0(DAT_80095874);
    SsEnd();
}
