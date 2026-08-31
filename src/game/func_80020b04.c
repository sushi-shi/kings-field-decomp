#include <kf/game_types.h>

extern struct KfPoolRecord DAT_800910c0[];

struct KfPoolRecord *func_80020b04(void)
{
    struct KfPoolRecord *record = DAT_800910c0;
    u16 count = 12;

    do {
        if (record->state == 0) {
            record->value_04 = 255;
            return record;
        }
        record++;
    } while (--count != 0);
    return 0;
}
