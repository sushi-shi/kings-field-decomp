#include <kf/game_types.h>

extern struct KfPoolRecord DAT_800910c0[];

void func_80020978(void)
{
    struct KfPoolRecord *record = DAT_800910c0;
    u16 count = 12;

    do {
        record->state = 0;
        record->allocation = 0;
        record++;
    } while (--count != 0);
}
