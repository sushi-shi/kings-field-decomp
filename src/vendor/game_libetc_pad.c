#include <kf/address.h>
#include <kf/psyq_pad.h>
#include <kf/psyq_kernel.h>
#include <kf/psyq_libc.h>

static u32 pad_init_bad_identifier();
static void pad_read_bad_identifier(void);
static void pad_stop_bad_identifier(void);

DATA(0x80058020, 0x4)
static u32 pad_buf;

DATA(0x80058028, 0x4)
static u32 pad_status;

DATA(0x8006bd88, 0x4)
int PadIdentifier;

RODATA(0x80013e8c, 0x60)

/*
 * Version-skewed Sony LIBETC PAD.OBJ controller front end. The three PAD entry
 * points dispatch on the stored pad identifier: identifier 0 uses the Sony trampolines
 * (PAD_init2/PAD_dr/StopPAD2); any other identifier falls to a "Bad
 * PadIdentifier" reporting stub. The stubs keep the original K&R shape - they
 * declare no parameters and fall off the end, so PadInit consumes the
 * incidental v0 the same way the retail program does.
 *
 * The init reporting stub retains its K&R parameter list because PadInit
 * passes the low-level call arguments even though the body ignores them.
 */

ADDRESS(0x800500b8, 0x74)
u32 PadInit(s32 identifier)
{
    u32 result;

    PadIdentifier = identifier;
    pad_status = 0;
    pad_buf = -1;
    if (identifier == 0) {
        result = PAD_init2(0x20000001, &pad_buf);
    } else {
        result = pad_init_bad_identifier(0x20000001, &pad_buf);
    }
    ResetCallback();
    return result;
}

ADDRESS(0x8005012c, 0x44)
u32 PadRead(void)
{
    if (PadIdentifier == 0) {
        PAD_dr();
    } else {
        pad_read_bad_identifier();
    }
    return ~pad_buf;
}

ADDRESS(0x80050170, 0x3c)
void PadStop(void)
{
    if (PadIdentifier == 0) {
        StopPAD2();
    } else {
        pad_stop_bad_identifier();
    }
}

ADDRESS(0x800501ac, 0x30)
static u32 pad_init_bad_identifier()
{
    printf("PAD_init: Bad PadIdentifier %d\n", PadIdentifier);
}

ADDRESS(0x800501dc, 0x30)
static void pad_read_bad_identifier(void)
{
    printf("PAD_dr  : Bad PadIdentifier %d\n", PadIdentifier);
}

ADDRESS(0x8005020c, 0x30)
static void pad_stop_bad_identifier(void)
{
    printf("StopPAD : Bad PadIdentifier %d\n", PadIdentifier);
}
