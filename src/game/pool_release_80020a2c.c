#include <kf/address.h>
#include <kf/game_types.h>

extern struct KfPoolRecord pool_records[];

extern void pool_record_release(struct KfPoolRecord *record);

/*
 * Releases every occupied pool record, freeing each owned allocation.  Called
 * on the hard resets (death restart, floor warp) that must flush all cached
 * instance data.
 */
ADDRESS(0x80020a2c, 0x6c)
void pool_release_all(void)
{
    struct KfPoolRecord *record = pool_records;
    s16 i;

    for (i = 11; i != -1; i--) {
        if (record->state != 0) {
            pool_record_release(record);
        }
        record++;
    }
}

/*
 * Per-frame sweep: releases the records still marked stale (state 1) by
 * pool_mark_allocated after render_entities had a chance to revalidate them
 * back to state 2.
 */
ADDRESS(0x80020a98, 0x6c)
void pool_release_stale(void)
{
    struct KfPoolRecord *record = pool_records;
    u16 count = 12;

    do {
        if (record->state == 1) {
            pool_record_release(record);
        }
        record++;
    } while (--count != 0);
}
