#include <kf/address.h>
#include <kf/psyq_cd.h>
#include <kf/overlay.h>
#include <kf/game_types.h>
#include <kf/open_controller.h>
#include <kf/psyq_kernel.h>

#define STARTUP_STORE_ADDRESS 0x800377a0
#define INITIAL_HEAP_ADDRESS 0x80080100

enum {
    STARTUP_REPEAT_STORE_COUNT = 0x70218,
    INITIAL_HEAP_BYTES = 0x177f00
};

ADDRESS(0x80013734, 0x24)
void func_80013734(int *destination, int count, int value)
{
    /* Retail stores to the same word on every iteration. */
    while (count-- != 0) {
        *destination = value;
    }
}

ADDRESS(0x80013758, 0x6c)
void main(s32 entry_arg0, u32 *entry_args)
{
    func_80013734((int *)STARTUP_STORE_ADDRESS, STARTUP_REPEAT_STORE_COUNT, 0);
    CdInit();
    InitHeap((void *)INITIAL_HEAP_ADDRESS, INITIAL_HEAP_BYTES);
    ExitCriticalSection();
    opening_run(KF_ENUM_DECODE(KfOpenMode, entry_args[KF_OVERLAY_REQUEST_WORD]));
}
