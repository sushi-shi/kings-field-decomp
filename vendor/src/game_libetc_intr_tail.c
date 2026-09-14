#include <kf/lib/address.h>
#include <kf/game/state.h>
#include <psyq/kernel.h>

/*
 * Version-skewed Sony LIBETC INTR.OBJ static helper. Its sole GAME caller is
 * the adjacent vendored intInit body, and the corresponding OPEN pair has the
 * same instruction shapes.
 */

ADDRESS(0x8005005c, 0x5c)
u32 critical_section_set(s32 enable)
{
    u32 previous = DAT_80057d24;

    if (enable) {
        EnterCriticalSection();
    } else {
        ExitCriticalSection();
    }
    DAT_80057d24 = enable;
    return previous;
}
