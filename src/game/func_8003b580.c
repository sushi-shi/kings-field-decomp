#include <kf/address.h>
#include <kf/semantic_types.h>

extern u32 DAT_80057c04;
extern void func_8003b5ac(s32 arg0);

ADDRESS(0x8003b580, 0x2c)
void func_8003b580(void)
{
    if (DAT_80057c04) {
        func_8003b5ac(0);
    }
}
