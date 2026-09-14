#include <kf/lib/bool.h>
#include <stdarg.h>
#include <kf/lib/address.h>
#include <kf/lib/debug.h>
#include <kf/lib/types.h>

/*
 * The formatter walks its va_list argument words and returns a byte count
 * including the terminating NUL. The diagnostic sink currently emits nothing.
 */

/* "DEBUG STOP !!!" literal owned by this unit in the shared rodata pool. */
RODATA(0x80012dd4, 0x10)

DATA(0x80057b98, 0x4)
KfBool32 debug_stop_flag = KF_FALSE;

/* Numeric scratch: eight leading bytes receive left padding (at most seven
 * are written), then a sign, ten decimal digits and NUL from the digit anchor
 * at 0x800598a8. The claim is the smallest eight-byte-rounded reservation that
 * covers that span; the original allocation's outer bounds remain unresolved. */
DATA(0x800598a0, 0x18)
static char format_number_storage[24];

ADDRESS(0x8003a7dc, 0x40)
void debug_stop(void)
{
    debug_printf_sink("DEBUG STOP !!!\n");
    debug_stop_flag = debug_stop_flag == KF_FALSE;
}

#include "../lib/format.inc"

#include "../lib/debug_sink.inc"

ADDRESS(0x8003ac4c, 0x8)
void func_8003ac4c(void)
{
    /* Unreferenced return stub at the game/runtime boundary. */
}
