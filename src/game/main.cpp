#include <kf/lib/overlay.h>
#include <kf/lib/types.h>
#include <psyq/kernel.h>
#include <kf/game/game.h>

#include "../lib/repeat_store_word.inc"

extern "C" void main(s32 entry_arg0, KfOverlayArguments *entry_args)
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
