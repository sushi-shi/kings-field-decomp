#include <kf/lib/null.h>
#include <kf/game/graphics.h>

#include <kf/lib/math.h>
#include <kf/game/asset.h>
#include <kf/game/render.h>
#include <kf/lib/memory.h>
#include <kf/game/pool.h>
#include <kf/lib/geometry_types.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

typedef struct KfAnimClip {
    u16 keyframe_count;
    u16 unknown_02;
    u32 keyframes[1];
} KfAnimClip;

typedef struct KfAnimKeyframe {
    KfAnimationBlendDirection reverse;
    u16 duration;
    u16 rest_index;
    u16 morph_count;
    u16 morph_indices[1];
} KfAnimKeyframe;

typedef struct KfMorphRange {
    u32 base_vertex;
    u32 vertex_count;
} KfMorphRange;

typedef struct KfMorphObject {
    u32 tmd_object_index;
    KfMorphRange range;
    SVECTOR deltas[1];
} KfMorphObject;

static_assert(sizeof(KfMorphRange) == 8 && offsetof(KfMorphObject, deltas) == 12);

static void morph_add_deltas(SVECTOR *vertices, u32 vertex_count,
    const KfMorphObject *morph, u16 blend)
{
    const auto &range = morph->range;
    if (range.base_vertex > vertex_count || range.vertex_count > vertex_count - range.base_vertex)
        kf::host_fail("Animation morph exceeds model vertex range.");
    const auto add = [blend](s16 value, s16 delta) {
        const s32 scaled = (s32(delta) * s16(blend)) >> KF_FIXED12_BITS;
        // Delta scaling saturates first; adding it to the base vertex wraps.
        return static_cast<s16>(value + std::clamp<s32>(scaled, std::numeric_limits<s16>::min(), std::numeric_limits<s16>::max()));
    };
    for (u32 i = 0; i < range.vertex_count; ++i) {
        SVECTOR &vertex = vertices[range.base_vertex + i];
        const SVECTOR &delta = morph->deltas[i];
        vertex.vx = add(vertex.vx, delta.vx);
        vertex.vy = add(vertex.vy, delta.vy);
        vertex.vz = add(vertex.vz, delta.vz);
    }
}

static inline void copy_vertices(
    SVECTOR *output, const SVECTOR *input, u32 count)
{
    memcpy(output, input, std::size_t(count) * sizeof *output);
}

KfPoolRecord *render_bind_animated_instance(
    KfPoolRecord **owner_slot, u16 asset_index, KfAnimationClip clip_index, u16 phase,
    u32 vertex_count)
{
    if (vertex_count > KF_PROJECTED_VERTEX_CAPACITY)
        kf::host_fail("Animated model exceeds vertex capacity.");
    KfPoolRecord *record = *owner_slot;
    KfAssetHeader *asset_header = game_graphics_runtime.asset_registry_entries[asset_index];
    KfAnimClip *clip;
    KfAnimKeyframe *keyframe;
    KfMorphObject *morph_object;
    u32 *clip_table;
    u32 *object_table;
    u16 morphs_left;
    u16 phase_end;
    u16 phase_start;

    u16 keyframe_index = 0;
    u16 blend_fraction;
    u16 keyframes_left;

    if (asset_header->animation_clip_count == 0) {
        if (record != NULL) {
            pool_record_release(record);
        }
        asset_registry_select(asset_index);
        tmd_select_object_vertices(0);
        return (KfPoolRecord *)KF_ANIMATION_BIND_STATIC;
    }
    if (vertex_count == 0)
        kf::host_fail("Animated model has no vertices.");

    if (record == NULL) {
        record = pool_allocate();
        if (record == NULL) {
            return NULL;
        }

reinitialize_record:
        record->asset_index = asset_index;
        record->owner_slot = owner_slot;
retry_allocation:
        record->cached_vertices = (SVECTOR *)memory_malloc_checked(
            vertex_count * sizeof(SVECTOR));
        if (record->cached_vertices == NULL) {
            pool_release_all();
            goto retry_allocation;
        }
        *owner_slot = record;
    } else if (record->asset_index != asset_index) {
        pool_record_release(record);
        record->clip_index = KF_ANIMATION_CLIP_NONE;
        goto reinitialize_record;
    }

    phase_end = 0;
    phase_start = 0;
    clip_table = (u32 *)((char *)asset_header + asset_header->clip_table_offset);
    clip = (KfAnimClip *)((char *)asset_header + clip_table[kf_enum_encode<u16>(clip_index)]);
    keyframes_left = clip->keyframe_count;
    {
        u32 *keyframe_offsets = clip->keyframes;

        while (keyframes_left-- != 0) {
            keyframe = (KfAnimKeyframe *)((char *)asset_header + *keyframe_offsets);
            keyframe_offsets++;
            phase_end += keyframe->duration;
            if (phase < phase_end) {
                u32 forward_fraction = ((u32)(u16)(phase - phase_start) << KF_FIXED12_BITS)
                    / keyframe->duration;

                blend_fraction = forward_fraction;
                if (keyframe->reverse != KF_ANIMATION_BLEND_FORWARD) {
                    blend_fraction = KF_FIXED12_ONE - forward_fraction;
                }
                goto update_vertex_cache;
            }
            phase_start = phase_end;
            keyframe_index++;
        }
    }
    keyframe_index--;
    blend_fraction = KF_FIXED12_ONE;

update_vertex_cache:
    if (record->clip_index != clip_index || record->keyframe_index != keyframe_index) {
        object_table = (u32 *)((char *)asset_header + asset_header->object_table_offset);
        asset_registry_select(asset_index);
        tmd_select_object_vertices(0);

        copy_vertices(record->cached_vertices, game_graphics_runtime.current_tmd_vertices, vertex_count);

        morphs_left = keyframe->morph_count;
        {
            u16 *morph_indices = keyframe->morph_indices;

            while (morphs_left-- != 0) {
                morph_object = (KfMorphObject *)(
                    (char *)asset_header + object_table[*morph_indices]);
                morph_indices++;
                morph_add_deltas(record->cached_vertices, vertex_count, morph_object, KF_FIXED12_ONE);
            }
        }

        record->rest_morph = (KfMorphObject *)(
            (char *)asset_header + object_table[keyframe->rest_index]);
    }

    record->clip_index = clip_index;
    record->keyframe_index = keyframe_index;

    copy_vertices(game_graphics_runtime.morph_scratch, record->cached_vertices, vertex_count);
    morph_add_deltas(game_graphics_runtime.morph_scratch, vertex_count, record->rest_morph, blend_fraction);
    tmd_set_current_vertices(game_graphics_runtime.morph_scratch);
    record->state = KF_ANIMATION_CACHE_LIVE;
    return record;
}

void pool_reset(void)
{
    KfPoolRecord *record = game_graphics_runtime.pool_records;
    u16 records_left = KF_ANIMATION_CACHE_CAPACITY;

    do {
        record->state = KF_ANIMATION_CACHE_FREE;
        record->cached_vertices = NULL;
        record++;
    } while (--records_left != 0);
}

void pool_mark_allocated(void)
{
    KfPoolRecord *record = game_graphics_runtime.pool_records;
    u16 records_left = KF_ANIMATION_CACHE_CAPACITY;

    do {
        if (record->state != KF_ANIMATION_CACHE_FREE) {
            record->state = KF_ANIMATION_CACHE_STALE;
        }
        record++;
    } while (--records_left != 0);
}

void pool_record_release(KfPoolRecord *record)
{
    record->state = KF_ANIMATION_CACHE_FREE;
    *record->owner_slot = NULL;
    if (record->cached_vertices != NULL) {
        free((void *)record->cached_vertices);
        record->cached_vertices = NULL;
    }
}

void pool_release_all(void)
{
    KfPoolRecord *record = game_graphics_runtime.pool_records;
    s16 records_left;

    for (records_left = KF_ANIMATION_CACHE_CAPACITY - 1; records_left != -1; records_left--) {
        if (record->state != KF_ANIMATION_CACHE_FREE) {
            pool_record_release(record);
        }
        record++;
    }
}

void pool_release_stale(void)
{
    KfPoolRecord *record = game_graphics_runtime.pool_records;
    u16 records_left = KF_ANIMATION_CACHE_CAPACITY;

    do {
        if (record->state == KF_ANIMATION_CACHE_STALE) {
            pool_record_release(record);
        }
        record++;
    } while (--records_left != 0);
}

KfPoolRecord *pool_allocate(void)
{
    KfPoolRecord *record = game_graphics_runtime.pool_records;
    u16 records_left = KF_ANIMATION_CACHE_CAPACITY;

    do {
        if (record->state == KF_ANIMATION_CACHE_FREE) {
            record->clip_index = KF_ANIMATION_CLIP_NONE;
            return record;
        }
        record++;
    } while (--records_left != 0);
    return NULL;
}
