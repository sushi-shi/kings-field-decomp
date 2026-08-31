#include <kf/game_types.h>

extern struct KfPoolRecord DAT_800910c0[];

void func_800209a8(void)
{
    struct KfPoolRecord *record = DAT_800910c0;
    u16 count = 12;

    do {
        if (record->state != 0) {
            record->state = 1;
        }
        record++;
    } while (--count != 0);
}
