#include <kf/lib/null.h>
#include <kf/game/graphics.h>
#include <kf/lib/address.h>
#include <kf/lib/math.h>
#include <kf/game/asset.h>
#include <kf/game/render.h>
#include <kf/lib/memory.h>
#include <kf/game/pool.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>


/* Cache full-weight keyframe morphs in a pool record; blend into shared scratch. */

/* asset base + clip_table[KF_ENUM_ENCODE(u16, clip_index)]: one animation clip. */
typedef struct KfAnimClip {
    u16 keyframe_count; /* +0 */
    u16 unknown_02;     /* +2 */
    u32 keyframes[1];   /* +4: byte offsets to KfAnimKeyframe (from asset base) */
} KfAnimClip;

/* asset base + clip->keyframes[i]: one keyframe. */
typedef struct KfAnimKeyframe {
    KfAnimationBlendDirection reverse; /* +0: nonzero reverses the fraction */
    u16 duration;         /* +2 */
    u16 rest_index;       /* +4: object-table index of the rest pose */
    u16 morph_count;      /* +6 */
    u16 morph_indices[1]; /* +8: object-table indices of the morph targets */
} KfAnimKeyframe;

/* The rest blend also consumes these two VDF range words as one SDK vector. */
typedef struct KfMorphRange {
    u32 base_vertex;
    u32 vertex_count;
} KfMorphRange;

typedef union KfMorphPrefix {
    KfMorphRange range;
    SVECTOR vector;
} KfMorphPrefix;

/* asset base + object_table[index]: one VDF-format morph/rest object. */
typedef struct KfMorphObject {
    u32 tmd_object_index; /* +0: ignored; retail always selects TMD object zero */
    KfMorphPrefix prefix; /* +4: first vertex index and affected count */
    SVECTOR deltas[1];    /* +12: signed vertex deltas */
} KfMorphObject;

/*
 * KfPoolRecord lifecycle: the twelve-entry pool that caches per-instance
 * vertex allocations across frames. Records advance state 0 (free) -> 2 (live) and
 * are marked stale (1) each frame so the render pass can revalidate them
 * before pool_release_stale frees whatever was not touched.
 */

static inline void copy_vertices(
    SVECTOR *output, const SVECTOR *input, u16 count)
{
    /* These asset, heap and scratch buffers are word-aligned; include SVECTOR.pad. */
    const u32 *source = (const u32 *)input;
    u32 *destination = (u32 *)output;

    do {
        *destination++ = *source++;
        *destination++ = *source++;
    } while (--count != 0);
}

ADDRESS(0x800205d4, 0x3a4)
KfPoolRecord *render_bind_animated_instance(
    KfPoolRecord **owner_slot, u16 asset_index, KF_ENUM_PARAM(KfAnimationClip, u16) clip_index, u16 phase,
    u16 vertex_count)
{
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
    /* Retail uses the incoming s5 value without initializing it. */
    u16 keyframe_index;
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
    clip = (KfAnimClip *)((char *)asset_header + clip_table[KF_ENUM_ENCODE(u16, clip_index)]);
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
                gteMIMefunc(&record->cached_vertices[morph_object->prefix.range.base_vertex],
                            morph_object->deltas, morph_object->prefix.range.vertex_count, KF_FIXED12_ONE);
            }
        }

        record->rest_morph = (KfMorphObject *)(
            (char *)asset_header + object_table[keyframe->rest_index]);
    }

    record->clip_index = clip_index;
    record->keyframe_index = keyframe_index;

    copy_vertices(&game_graphics_runtime.morph_scratch[1], record->cached_vertices, vertex_count);

    morph_object = record->rest_morph;
    {
        SVECTOR *scratch_vertex = &game_graphics_runtime.morph_scratch[morph_object->prefix.range.base_vertex];
        u32 *scratch_words = (u32 *)scratch_vertex;
        u32 saved_xy_word = scratch_words[0];
        u32 saved_z_pad_word = scratch_words[1];

        /* Blend the header-sized extra vector too, then restore its scratch entry. */
        gteMIMefunc(scratch_vertex, &morph_object->prefix.vector,
                    morph_object->prefix.range.vertex_count + 1, blend_fraction);
        scratch_words[0] = saved_xy_word;
        scratch_words[1] = saved_z_pad_word;
    }
    tmd_set_current_vertices(&game_graphics_runtime.morph_scratch[1]);
    record->state = KF_ANIMATION_CACHE_LIVE;
    return record;
}

ADDRESS(0x80020978, 0x30)
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

ADDRESS(0x800209a8, 0x3c)
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

ADDRESS(0x800209e4, 0x48)
void pool_record_release(KfPoolRecord *record)
{
    record->state = KF_ANIMATION_CACHE_FREE;
    *record->owner_slot = NULL;
    if (record->cached_vertices != NULL) {
        free((void *)record->cached_vertices);
        record->cached_vertices = NULL;
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
    KfPoolRecord *record = game_graphics_runtime.pool_records;
    s16 records_left;

    for (records_left = KF_ANIMATION_CACHE_CAPACITY - 1; records_left != -1; records_left--) {
        if (record->state != KF_ANIMATION_CACHE_FREE) {
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
    KfPoolRecord *record = game_graphics_runtime.pool_records;
    u16 records_left = KF_ANIMATION_CACHE_CAPACITY;

    do {
        if (record->state == KF_ANIMATION_CACHE_STALE) {
            pool_record_release(record);
        }
        record++;
    } while (--records_left != 0);
}

ADDRESS(0x80020b04, 0x48)
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
