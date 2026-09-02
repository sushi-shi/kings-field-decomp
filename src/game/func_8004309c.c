#include <kf/address.h>
#include <kf/semantic_types.h>

extern u16 DAT_80057ff0;

ADDRESS(0x8004309c, 0x10)
void func_8004309c(void)
{
    DAT_80057ff0 = 0;
}
