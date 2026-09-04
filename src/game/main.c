#include <kf/address.h>
#include <kf/game_types.h>
#include <kf/psyq_kernel.h>
#include <kf/game.h>

ADDRESS(0x80014268, 0x24)
void repeat_store_word(int *destination, int count, int value)
{
    /* Retail deliberately stores to the same word on every iteration. */
    while (count-- != 0) {
        *destination = value;
    }
}

/*
 * GCC inserts the `__main` hook call for a function named main; the SNMAIN
 * start routine tail-calls here. The BSS start and heap base are numeric
 * constants in retail (lui/ori forms carry no relocation).
 */
ADDRESS(0x8001428c, 0x88)
void main(s32 entry_arg0, u32 *entry_args)
{
    repeat_store_word((int *)0x80058060, 0x67fe8, 0);
    InitHeap((u32 *)0x800a0980, 0x157680);
    CdInit();
    PadInit(0);
    InitCARD2(1);
    ExitCriticalSection();
    game_main_loop();
    entry_args[1] = game_exit_code;
}
