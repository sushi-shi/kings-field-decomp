#include <kf/address.h>
#include <psyq/cd.h>
#include <kf/overlay.h>
#include <kf/game_types.h>
#include <kf/open_controller.h>
#include <psyq/kernel.h>

ADDRESS(0x80013734, 0x24)
void repeat_store_word(int *destination, int count, int value)
{
    /* Retail stores to the same word on every iteration. */
    while (count-- != 0) {
        *destination = value;
    }
}

ADDRESS(0x80013758, 0x6c)
void main(s32 entry_arg0, KfOverlayArguments *entry_args)
{
    repeat_store_word((int *)BSS_START,
        (OVERLAY_STACK_BOTTOM - (u32)BSS_START) / sizeof(int), 0);
    CdInit();
    InitHeap((void *)BSS_END, OVERLAY_STACK_BOTTOM - (u32)BSS_END);
    ExitCriticalSection();
    opening_run(entry_args->request);
}
