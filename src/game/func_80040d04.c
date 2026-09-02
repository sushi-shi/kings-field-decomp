#include <kf/address.h>
#include <kf/semantic_types.h>

extern void DeliverEvent(u32 event, u32 spec);
extern void func_8004fbe4(u32 channel);

ADDRESS(0x80040d04, 0x30)
void func_80040d04(void)
{
    DeliverEvent(0xf0000009, 2);
    func_8004fbe4(0);
}
