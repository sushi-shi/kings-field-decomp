#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Controller (PAD) front end plus the interrupt critical-section guard that
 * sits at the head of this contiguous run. The three PAD entry points dispatch
 * on the stored pad identifier: identifier 0 uses the Sony trampolines
 * (PAD_init2/PAD_dr/StopPAD2); any other identifier falls to a "Bad
 * PadIdentifier" reporting stub. The stubs keep the original K&R shape - they
 * declare no parameters and fall off the end, so pad_initialize consumes the
 * incidental v0 the same way the retail program does.
 *
 * pad_initialize/pad_read/pad_stop stay address-derived here because
 * they are the public PAD API referenced from many other translation units;
 * proposed names (pad_initialize/pad_read/pad_stop) are in the reconstruction
 * report. DAT_80057d24/DAT_8006bd88/DAT_80058020/DAT_80058028 likewise remain
 * address-derived WIP identities (critical-section state, pad identifier, and
 * the two pad data words).
 *
 * pad_initialize/pad_read/pad_stop are exact. critical_section_set and
 * the three stubs are structurally exact but one prologue reorder short: retail
 * hoists the first data load above the frame allocation when it feeds a call
 * argument or a callee-saved register, which cc1psx-257 does not reproduce (see
 * docs/patterns/source-shapes-gcc257.md, pad campaign). Not steered with dead
 * code.
 */

extern u32 DAT_80057d24;
extern u32 DAT_8006bd88;
extern u32 DAT_80058020;
extern u32 DAT_80058028;

extern char DAT_80013e8c[];
extern char DAT_80013eac[];
extern char DAT_80013ecc[];

extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern u32 PAD_init2();
extern void PAD_dr(void);
extern void StopPAD2(void);
extern int printf(const char *format, ...);
extern void ResetCallback(void);

extern u32 pad_init_bad_identifier();
extern void pad_read_bad_identifier(void);
extern void pad_stop_bad_identifier(void);

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
    printf(DAT_80013e8c, DAT_8006bd88);
}

ADDRESS(0x800501dc, 0x30)
void pad_read_bad_identifier(void)
{
    printf(DAT_80013eac, DAT_8006bd88);
}

ADDRESS(0x8005020c, 0x30)
void pad_stop_bad_identifier(void)
{
    printf(DAT_80013ecc, DAT_8006bd88);
}
