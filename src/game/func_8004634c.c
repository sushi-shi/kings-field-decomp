#include <kf/address.h>
#include <kf/semantic_types.h>

extern void SpuVmFlush(void);

ADDRESS(0x8004634c, 0x20)
void func_8004634c(void)
{
    SpuVmFlush();
}
