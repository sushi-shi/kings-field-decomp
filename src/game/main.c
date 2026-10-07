#include <kf/lib/address.h>
#include <kf/lib/overlay.h>
#include <kf/game/startup_layout.h>
#include <kf/lib/types.h>
#include <psyq/kernel.h>
#include <kf/game/game.h>

#include "../lib/repeat_store_word.inc"

/*
 * GCC inserts the `__main` hook call for a function named main; the SDK
 * start routine tail-calls here.
 */
ADDRESS(0x8001428c, 0x88)
void main(s32 entry_arg0, KfOverlayArguments *entry_args)
{
    repeat_store_word((int *)GAME_BSS_START,
        (OVERLAY_STACK_BOTTOM - GAME_BSS_START) / sizeof(int), 0);
    InitHeap((void *)GAME_HEAP_START, OVERLAY_STACK_BOTTOM - GAME_HEAP_START);
    CdInit();
    PadInit(0);
    InitCARD2(1);
    ExitCriticalSection();
    game_main_loop();
    entry_args->result = game_next_overlay_mode;
}
