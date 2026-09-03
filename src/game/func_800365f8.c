#include <kf/address.h>
#include <kf/semantic_types.h>

extern void map_load_floor(void);

ADDRESS(0x800365f8, 0x20)
void func_800365f8(void)
{
    map_load_floor();
}
