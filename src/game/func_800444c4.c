#include <kf/address.h>
#include <kf/semantic_types.h>

extern s16 DAT_8005af1a[][4];

ADDRESS(0x800444c4, 0x34)
s32 func_800444c4(s16 index)
{
    if (index < 34) {
        return DAT_8005af1a[index][0];
    }
    return -1;
}
