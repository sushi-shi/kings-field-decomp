#include <kf/address.h>
#include <kf/semantic_types.h>

/* 34-entry, 8-byte lookup table indexed by a signed 16-bit key. Offsets 0
   and 2 are read signed; offset 4 is read unsigned into an out-parameter. */
typedef struct {
    s16 field_0;
    s16 field_2;
    u16 field_4;
    s16 field_6;
} DAT_8005af1a_record;

extern DAT_8005af1a_record DAT_8005af1a[];

ADDRESS(0x80044488, 0x3c)
s32 func_80044488(s16 index, u16 *out)
{
    if (index < 34) {
        *out = DAT_8005af1a[index].field_4;
        return index;
    }
    return -1;
}

ADDRESS(0x800444c4, 0x34)
s32 func_800444c4(s16 index)
{
    if (index < 34) {
        return DAT_8005af1a[index].field_0;
    }
    return -1;
}

ADDRESS(0x800444f8, 0x34)
s32 func_800444f8(s16 index)
{
    if (index < 34) {
        return DAT_8005af1a[index].field_2;
    }
    return -1;
}
