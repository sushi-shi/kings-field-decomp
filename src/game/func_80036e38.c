#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * func_80036e38 (GAME.EXE): tear down and re-establish the system heap around a
 * mode-selected reload. It drains the GPU (DrawSync), runs func_80020a2c, resets
 * the system heap, dispatches on the mode argument, resets the heap again, and
 * clears the player motion state. Callers pass a variable number of arguments,
 * so the incoming register arguments are spilled to their home slots.
 */

extern void func_80020a2c(void);
extern void memory_reset_system_heap(void);
extern s32 func_80022348(void);
extern s32 func_80021ffc(s32 arg0);
extern void func_800212d8(u32 arg0);
extern void player_clear_motion(void);

ADDRESS(0x80036e38, 0xc8)
u32 func_80036e38(s32 mode, ...)
{
    u32 result;

    DrawSync(0);
    func_80020a2c();
    memory_reset_system_heap();
    switch (mode) {
    case 0:
        result = func_80022348();
        break;
    case 1:
        result = func_80021ffc(*((u8 *)&mode + 4));
        break;
    case 2:
        func_800212d8(*((u8 *)&mode + 4));
        result = 0;
        break;
    }
    memory_reset_system_heap();
    player_clear_motion();
    return result;
}
