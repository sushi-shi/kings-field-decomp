#ifndef KF_POOL_H
#define KF_POOL_H

/*
 * game.pool (0x80020978-0x80020b4c): the twelve-entry KfPoolRecord cache and
 * its lifecycle. Exports the record management routines other units call.
 */

#include <kf/game_types.h>

typedef struct KfPoolRecord {
    s16 state;
    u16 unknown_02;
    u16 value_04;
    u16 unknown_06;
    u32 unknown_08;
    void *allocation;
    u32 *backlink;
} KfPoolRecord;

extern KfPoolRecord pool_records[12];

extern void pool_reset(void);
extern void pool_mark_allocated(void);
extern void pool_record_release(KfPoolRecord *record);
extern void pool_release_all(void);
extern void pool_release_stale(void);
extern KfPoolRecord *pool_allocate(void);

#endif
