#ifndef KF_POOL_H
#define KF_POOL_H

/*
 * game.pool (0x80020978-0x80020b4c): the twelve-entry KfPoolRecord cache and
 * its lifecycle. Exports the record management routines other units call.
 */

#include <kf/game_types.h>

extern void pool_reset(void);
extern void pool_mark_allocated(void);
extern void pool_record_release(struct KfPoolRecord *record);
extern void pool_release_all(void);
extern void pool_release_stale(void);
extern struct KfPoolRecord *pool_allocate(void);

#endif
