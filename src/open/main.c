#include <kf/address.h>
#include <kf/overlay.h>
#include <kf/game_types.h>
#include <kf/open_controller.h>
#include <kf/psyq_kernel.h>

ADDRESS(0x80013734, 0x24)
void func_80013734(int *destination, int count, int value)
{
    while (count-- != 0) {
        *destination = value;
    }
}

ADDRESS(0x80013758, 0x6c)
void main(s32 entry_arg0, u32 *entry_args)
{
    func_80013734((int *)0x800377a0, 0x70218, 0);
    CdInit();
    InitHeap((void *)0x80080100, 0x177f00);
    ExitCriticalSection();
    opening_run(entry_args[KF_OVERLAY_REQUEST_WORD]);
}
