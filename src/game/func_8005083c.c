#include <kf/address.h>
#include <kf/semantic_types.h>

extern u32 DAT_80057d3c;

ADDRESS(0x8005083c, 0x18)
u32 func_8005083c(u32 value)
{
    u32 previous = DAT_80057d3c;

    DAT_80057d3c = value;
    return previous;
}
