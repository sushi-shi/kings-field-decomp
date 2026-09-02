#include <kf/address.h>
#include <kf/semantic_types.h>

extern u8 DAT_80057fe0;

ADDRESS(0x80046324, 0x28)
u8 func_80046324(u8 value)
{
    if (value < 25) {
        DAT_80057fe0 = value;
        return DAT_80057fe0;
    }
    return 0xff;
}
