#include <kf/lib/address.h>
#include <psyq/cd.h>
#include <kf/lib/overlay.h>
#include <kf/open/startup_layout.h>
#include <kf/lib/types.h>
#include <kf/open/controller.h>
#include <psyq/kernel.h>

#include "../lib/repeat_store_word.inc"

ADDRESS(0x80013758, 0x6c)
void main(s32 entry_arg0, KfOverlayArguments *entry_args)
{
    repeat_store_word((int *)OPEN_BSS_START,
        (OVERLAY_STACK_BOTTOM - OPEN_BSS_START) / sizeof(int), 0);
    CdInit();
    InitHeap((void *)OPEN_HEAP_START, OVERLAY_STACK_BOTTOM - OPEN_HEAP_START);
    ExitCriticalSection();
    opening_run(entry_args->request);
}
