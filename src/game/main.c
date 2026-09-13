#include <kf/address.h>
#include <kf/overlay.h>
#include <kf/game_types.h>
#include <psyq/kernel.h>
#include <kf/game.h>

#include "../shared/repeat_store_word.inc"

/*
 * GCC inserts the `__main` hook call for a function named main; the SDK
 * start routine tail-calls here.
 */
ADDRESS(0x8001428c, 0x88)
void main(s32 entry_arg0, KfOverlayArguments *entry_args)
{
    repeat_store_word((int *)BSS_START,
        (OVERLAY_STACK_BOTTOM - (u32)BSS_START) / sizeof(int), 0);
    InitHeap((void *)BSS_END, OVERLAY_STACK_BOTTOM - (u32)BSS_END);
    CdInit();
    PadInit(0);
    InitCARD2(1);
    ExitCriticalSection();
    game_main_loop();
    entry_args->result = game_next_overlay_mode;
}
