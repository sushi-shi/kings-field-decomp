#include <kf/address.h>
#include <kf/semantic_types.h>

extern u32 DAT_80057d34;
extern u32 DAT_80057d30;
extern u32 DAT_80057d3c;

ADDRESS(0x8005081c, 0x10)
u32 func_8005081c(void)
{
    return DAT_80057d34;
}

ADDRESS(0x8005082c, 0x10)
u32 func_8005082c(void)
{
    return DAT_80057d30;
}

ADDRESS(0x8005083c, 0x18)
u32 func_8005083c(u32 value)
{
    u32 previous = DAT_80057d3c;

    DAT_80057d3c = value;
    return previous;
}
