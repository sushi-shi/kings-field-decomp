#include <kf/address.h>
#include <kf/semantic_types.h>

extern u16 DAT_8009ddb0;
extern u16 DAT_8009ddb2;

ADDRESS(0x800356e8, 0x20)
void func_800356e8(void)
{
    DAT_8009ddb0 = 3;
    DAT_8009ddb2 = 10;
}
