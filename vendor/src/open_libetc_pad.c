#include <kf/lib/address.h>
#include <psyq/pad.h>
#include <psyq/kernel.h>
#include <psyq/libc.h>

/* The modern view spells the two ignored K&R arguments. */
#if defined(__cplusplus)
#define PAD_INIT_IGNORED_ARGS s32, u32 *
#else
#define PAD_INIT_IGNORED_ARGS
#endif
static u32 pad_init_bad_identifier(PAD_INIT_IGNORED_ARGS);
static void pad_read_bad_identifier(void);
static void pad_stop_bad_identifier(void);

DATA(0x80037760, 0x4)
static u32 pad_buf;

DATA(0x80037768, 0x4)
static u32 pad_status;

DATA(0x80049528, 0x4)
int PadIdentifier;

RODATA(0x80013358, 0x60)

/*
 * OPEN links the same version-skewed Sony LIBETC PAD.OBJ source shape as
 * GAME. Its state and private literals occupy overlay-specific addresses.
 * The init diagnostic keeps its K&R form because PadInit passes ignored
 * low-level call arguments and consumes the incidental printf result.
 */

ADDRESS(0x8002fe8c, 0x74)
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

ADDRESS(0x8002ff00, 0x44)
u32 PadRead(void)
{
    if (PadIdentifier == 0) {
        PAD_dr();
    } else {
        pad_read_bad_identifier();
    }
    return ~pad_buf;
}

ADDRESS(0x8002ff44, 0x3c)
void PadStop(void)
{
    if (PadIdentifier == 0) {
        StopPAD2();
    } else {
        pad_stop_bad_identifier();
    }
}

ADDRESS(0x8002ff80, 0x30)
static u32 pad_init_bad_identifier(PAD_INIT_IGNORED_ARGS)
{
    printf("PAD_init: Bad PadIdentifier %d\n", PadIdentifier);
}

ADDRESS(0x8002ffb0, 0x30)
static void pad_read_bad_identifier(void)
{
    printf("PAD_dr  : Bad PadIdentifier %d\n", PadIdentifier);
}

ADDRESS(0x8002ffe0, 0x30)
static void pad_stop_bad_identifier(void)
{
    printf("StopPAD : Bad PadIdentifier %d\n", PadIdentifier);
}
