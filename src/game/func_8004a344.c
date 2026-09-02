#include <kf/address.h>
#include <kf/semantic_types.h>

extern u32 *DAT_800a06e0[];

ADDRESS(0x8004a344, 0x30)
void func_8004a344(s16 index)
{
    *DAT_800a06e0[index] |= 4;
}
