#include <kf/address.h>
#include <kf/game_types.h>

/* Psy-Q LIBAPI: InitHeap(unsigned long *head, unsigned long size); InitCARD(long). */
extern void InitHeap(u32 *head, u32 size);
extern u32 pad_initialize(s32 arg0);
extern void InitCARD2(s32 pad_enable);
extern void ExitCriticalSection(void);
extern void game_main_loop(void);
extern u32 game_exit_code;

ADDRESS(0x80014268, 0x24)
void func_80014268(int *destination, int count, int value)
{
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
    func_80014268((int *)0x80058060, 0x67fe8, 0);
    InitHeap((u32 *)0x800a0980, 0x157680);
    CdInit();
    pad_initialize(0);
    InitCARD2(1);
    ExitCriticalSection();
    game_main_loop();
    entry_args[1] = game_exit_code;
}
