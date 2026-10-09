#ifndef KF_ANIMATION_CACHE_H
#define KF_ANIMATION_CACHE_H

/*
 * Twelve-entry animation vertex cache and its per-frame lifecycle.
 */

#include <kf/lib/animation.h>
#include <kf/lib/types.h>
#include <kf/lib/enum.h>
#include <psyq/sdk.h>
#include <kf/lib/tmd.h>

struct KfMorphObject;

KF_ENUM_BEGIN(KfAnimationCacheState, s16)
    KF_ANIMATION_CACHE_FREE = 0,
    KF_ANIMATION_CACHE_STALE = 1,
    KF_ANIMATION_CACHE_LIVE = 2
KF_ENUM_END(KfAnimationCacheState)

enum {
    KF_ANIMATION_CACHE_CAPACITY = 12,
    KF_ANIMATION_BIND_STATIC = 1
};

typedef struct KfAnimationCacheRecord {
    KfAnimationCacheState state;
    u16 asset_index;
    KF_ENUM_STORAGE(KfAnimationClip, u16) clip_index;
    u16 keyframe_index;
    struct KfMorphObject *rest_morph;
    SVECTOR *cached_vertices;
    struct KfAnimationCacheRecord **owner_slot;
} KfAnimationCacheRecord;

/*
 * Returns NULL on pool exhaustion, KF_ANIMATION_BIND_STATIC cast to a pointer
 * for a static asset, or the live record. The static sentinel is not a record
 * and is never installed in *owner_slot; that slot owns the cached record.
 */
extern KfAnimationCacheRecord *render_bind_animated_instance(
    KfAnimationCacheRecord **owner_slot, u16 asset_index, KF_ENUM_PARAM(KfAnimationClip, u16) clip_index, u16 phase,
    u16 vertex_count);
extern void animation_cache_reset(void);
extern void animation_cache_mark_stale(void);
extern void animation_cache_release(KfAnimationCacheRecord *record);
extern void animation_cache_release_all(void);
extern void animation_cache_release_stale(void);
extern KfAnimationCacheRecord *animation_cache_allocate(void);

#endif
