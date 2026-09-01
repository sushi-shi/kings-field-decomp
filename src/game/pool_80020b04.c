#include <kf/address.h>
#include <kf/game_types.h>

extern struct KfPoolRecord pool_records[];

ADDRESS(0x80020b04, 0x48)
struct KfPoolRecord *pool_allocate(void)
{
    struct KfPoolRecord *record = pool_records;
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
