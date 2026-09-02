#include <kf/address.h>
#include <kf/semantic_types.h>

extern u32 DAT_80056504;

ADDRESS(0x8003c560, 0x10)
void func_8003c560(u32 value)
{
    DAT_80056504 = value;
}
