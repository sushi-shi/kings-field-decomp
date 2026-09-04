#include <kf/address.h>
#include <kf/game_types.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>
#include <kf/pool.h>

/*
 * KfPoolRecord lifecycle: the twelve-entry pool that caches per-instance
 * allocations across frames. Records advance state 0 (free) -> 2 (live) and
 * are marked stale (1) each frame so the render pass can revalidate them
 * before pool_release_stale frees whatever was not touched.
 */

ADDRESS(0x80020978, 0x30)
void pool_reset(void)
{
    KfPoolRecord *record = pool_records;
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
    KfPoolRecord *record = pool_records;
    u16 count = 12;

    do {
        if (record->state != 0) {
            record->state = 1;
        }
        record++;
    } while (--count != 0);
}

ADDRESS(0x800209e4, 0x48)
void pool_record_release(KfPoolRecord *record)
{
    record->state = 0;
    *record->backlink = 0;
    if (record->allocation != 0) {
        free(record->allocation);
        record->allocation = 0;
    }
}

/*
 * Releases every occupied pool record, freeing each owned allocation.  Called
 * on the hard resets (death restart, floor warp) that must flush all cached
 * instance data.
 */
ADDRESS(0x80020a2c, 0x6c)
void pool_release_all(void)
{
    KfPoolRecord *record = pool_records;
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
    KfPoolRecord *record = pool_records;
    u16 count = 12;

    do {
        if (record->state == 1) {
            pool_record_release(record);
        }
        record++;
    } while (--count != 0);
}

ADDRESS(0x80020b04, 0x48)
KfPoolRecord *pool_allocate(void)
{
    KfPoolRecord *record = pool_records;
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
