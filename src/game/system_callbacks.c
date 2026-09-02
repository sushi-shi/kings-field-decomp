#include <kf/address.h>
#include <kf/semantic_types.h>

extern u32 DAT_800642d0[];
extern u32 DAT_800642f0[];
extern u32 *DAT_80057d1c;
extern u16 *DAT_80057d14;

void func_8004fc2c(s32 channel, s32 handler);
u32 func_8004fcac(u32 event, s32 handler);

ADDRESS(0x8004fac4, 0x24)
void func_8004fac4(u32 arg0)
{
    func_8004fc2c(0, arg0);
}

ADDRESS(0x8004fae8, 0x24)
void func_8004fae8(u32 arg0)
{
    func_8004fc2c(1, arg0);
}

ADDRESS(0x8004fb0c, 0x24)
void func_8004fb0c(u32 arg0)
{
    func_8004fc2c(2, arg0);
}

ADDRESS(0x8004fb30, 0x24)
void func_8004fb30(u32 arg0)
{
    func_8004fc2c(3, arg0);
}

ADDRESS(0x8004fb54, 0x24)
void func_8004fb54(u32 arg0)
{
    func_8004fc2c(4, arg0);
}

ADDRESS(0x8004fb78, 0x24)
void func_8004fb78(u32 arg0)
{
    func_8004fc2c(5, arg0);
}

ADDRESS(0x8004fb9c, 0x24)
void func_8004fb9c(u32 arg0)
{
    func_8004fcac(0, arg0);
}

ADDRESS(0x8004fbc0, 0x24)
void func_8004fbc0(u32 arg0)
{
    func_8004fcac(2, arg0);
}

ADDRESS(0x8004fbe4, 0x24)
void func_8004fbe4(u32 arg0)
{
    func_8004fcac(9, arg0);
}

ADDRESS(0x8004fc08, 0x24)
void func_8004fc08(u32 arg0)
{
    func_8004fcac(10, arg0);
}

ADDRESS(0x8004fc2c, 0x80)
void func_8004fc2c(s32 channel, s32 handler)
{
    u32 *slot = &DAT_800642d0[channel];

    *slot = 0;
    if (handler != 0) {
        *DAT_80057d1c |= 0x800000 | (0x101 << (channel + 16));
        *slot = handler;
    } else {
        *DAT_80057d1c &= ~(0xff000000 | (1 << (channel + 16)));
    }
}

ADDRESS(0x8004fcac, 0x84)
u32 func_8004fcac(u32 event, s32 handler)
{
    u32 *slot;

    if (event == 3) {
        return -1;
    }
    slot = &DAT_800642f0[event];
    *slot = 0;
    if (handler != 0) {
        *slot = handler;
        *DAT_80057d14 |= 1 << event;
    } else {
        *DAT_80057d14 &= ~(1 << event);
    }
    return event;
}
