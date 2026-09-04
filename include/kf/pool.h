#ifndef KF_POOL_H
#define KF_POOL_H

/*
 * Twelve-entry animation vertex cache and its per-frame lifecycle.
 */

#include <kf/game_types.h>
#include <kf/psyq.h>

struct KfMorphObject;

typedef struct KfPoolRecord {
    s16 state;
    u16 asset_index;
    u16 clip_index;
    u16 keyframe_index;
    struct KfMorphObject *rest_morph;
    SVECTOR *cached_vertices;
    struct KfPoolRecord **owner_slot;
} KfPoolRecord;

extern KfPoolRecord pool_records[12];

extern void pool_reset(void);
extern void pool_mark_allocated(void);
extern void pool_record_release(KfPoolRecord *record);
extern void pool_release_all(void);
extern void pool_release_stale(void);
extern KfPoolRecord *pool_allocate(void);

#endif
