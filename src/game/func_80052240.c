#include <kf/address.h>
#include <kf/semantic_types.h>

extern u32 *DAT_80057d50;

ADDRESS(0x80052240, 0x18)
u32 func_80052240(void)
{
    return *DAT_80057d50;
}
