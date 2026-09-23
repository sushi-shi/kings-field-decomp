#ifndef KF_POOL_H
#define KF_POOL_H

#include <kf/lib/animation.h>
#include <kf/lib/types.h>
#include <kf/lib/enum.h>
#include <psyq/sdk.h>
#include <kf/lib/tmd.h>

struct KfMorphObject;

enum class KfAnimationCacheState : s16 {
    KF_ANIMATION_CACHE_FREE = 0,
    KF_ANIMATION_CACHE_STALE = 1,
    KF_ANIMATION_CACHE_LIVE = 2
}; using enum KfAnimationCacheState;

enum {
    KF_ANIMATION_CACHE_CAPACITY = 12,
    KF_ANIMATION_BIND_STATIC = 1
};

typedef struct KfPoolRecord {
    KfAnimationCacheState state;
    u16 asset_index;
    KfEnumStorage<KfAnimationClip, u16> clip_index;
    u16 keyframe_index;
    struct KfMorphObject *rest_morph;
    SVECTOR *cached_vertices;
    struct KfPoolRecord **owner_slot;
} KfPoolRecord;

extern KfPoolRecord *render_bind_animated_instance(
    KfPoolRecord **owner_slot, u16 asset_index, KfAnimationClip clip_index, u16 phase,
    u16 vertex_count);
extern void pool_reset(void);
extern void pool_mark_allocated(void);
extern void pool_record_release(KfPoolRecord *record);
extern void pool_release_all(void);
extern void pool_release_stale(void);
extern KfPoolRecord *pool_allocate(void);

#endif
