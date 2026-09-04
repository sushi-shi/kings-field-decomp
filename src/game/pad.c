#include <kf/address.h>
#include <kf/game_pad.h>
#include <kf/game_state.h>
#include <kf/psyq_kernel.h>
#include <kf/psyq_libc.h>

u32 pad_init_bad_identifier();
void pad_read_bad_identifier(void);
void pad_stop_bad_identifier(void);

RODATA(0x80013e8c, 0x60)

/*
 * Controller (PAD) front end plus the interrupt critical-section guard that
 * sits at the head of this contiguous run. The three PAD entry points dispatch
 * on the stored pad identifier: identifier 0 uses the Sony trampolines
 * (PAD_init2/PAD_dr/StopPAD2); any other identifier falls to a "Bad
 * PadIdentifier" reporting stub. The stubs keep the original K&R shape - they
 * declare no parameters and fall off the end, so pad_initialize consumes the
 * incidental v0 the same way the retail program does.
 *
 * The three reporting stubs retain their K&R parameter lists because callers
 * pass arguments that the bodies ignore.
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

ADDRESS(0x800500b8, 0x74)
u32 pad_initialize(s32 identifier)
{
    u32 result;

    DAT_8006bd88 = identifier;
    DAT_80058028 = 0;
    DAT_80058020 = -1;
    if (identifier == 0) {
        result = PAD_init2(0x20000001, &DAT_80058020);
    } else {
        result = pad_init_bad_identifier(0x20000001, &DAT_80058020);
    }
    ResetCallback();
    return result;
}

ADDRESS(0x8005012c, 0x44)
u32 pad_read(void)
{
    if (DAT_8006bd88 == 0) {
        PAD_dr();
    } else {
        pad_read_bad_identifier();
    }
    return ~DAT_80058020;
}

ADDRESS(0x80050170, 0x3c)
void pad_stop(void)
{
    if (DAT_8006bd88 == 0) {
        StopPAD2();
    } else {
        pad_stop_bad_identifier();
    }
}

ADDRESS(0x800501ac, 0x30)
u32 pad_init_bad_identifier()
{
    printf("PAD_init: Bad PadIdentifier %d\n", DAT_8006bd88);
}

ADDRESS(0x800501dc, 0x30)
void pad_read_bad_identifier(void)
{
    printf("PAD_dr  : Bad PadIdentifier %d\n", DAT_8006bd88);
}

ADDRESS(0x8005020c, 0x30)
void pad_stop_bad_identifier(void)
{
    printf("StopPAD : Bad PadIdentifier %d\n", DAT_8006bd88);
}
