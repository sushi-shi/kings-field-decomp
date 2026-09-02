#include <kf/address.h>
#include <kf/semantic_types.h>

extern u32 DAT_80057c04;
extern void func_8003b5ac(s32 arg0);

ADDRESS(0x8003b8c8, 0x34)
void func_8003b8c8(void)
{
    DAT_80057c04 = 1;
    func_8003b5ac(1);
    DAT_80057c04 = 0;
}
