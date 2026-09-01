#include <kf/address.h>
#include <kf/game_types.h>

extern struct KfPoolRecord pool_records[];

extern void free(void *allocation);

ADDRESS(0x80020978, 0x30)
void pool_reset(void)
{
    struct KfPoolRecord *record = pool_records;
    u16 count = 12;

    do {
        record->state = 0;
        record->allocation = 0;
        record++;
    } while (--count != 0);
}

ADDRESS(0x800209a8, 0x3c)
void pool_mark_allocated(void)
{
    struct KfPoolRecord *record = pool_records;
    u16 count = 12;

    do {
        if (record->state != 0) {
            record->state = 1;
        }
        record++;
    } while (--count != 0);
}

ADDRESS(0x800209e4, 0x48)
void pool_record_release(struct KfPoolRecord *record)
{
    record->state = 0;
    *record->backlink = 0;
    if (record->allocation != 0) {
        free(record->allocation);
        record->allocation = 0;
    }
}
