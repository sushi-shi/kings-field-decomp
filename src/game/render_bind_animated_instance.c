#include <kf/address.h>
#include <kf/game_asset.h>
#include <kf/game_render.h>
#include <kf/memory.h>
#include <kf/pool.h>
#include <kf/psyq.h>

/* Cache full-weight keyframe morphs in a pool record; blend into shared scratch. */

/* asset base + clip_table[clip_index]: one animation clip. */
typedef struct KfAnimClip {
    u16 keyframe_count; /* +0 */
    u16 unknown_02;     /* +2 */
    u32 keyframes[1];   /* +4: byte offsets to KfAnimKeyframe (from asset base) */
} KfAnimClip;

/* asset base + clip->keyframes[i]: one keyframe. */
typedef struct KfAnimKeyframe {
    u16 reverse;          /* +0: 0 => forward blend, else 0x1000 - fraction */
    u16 duration;         /* +2 */
    u16 rest_index;       /* +4: object-table index of the rest pose */
    u16 morph_count;      /* +6 */
    u16 morph_indices[1]; /* +8: object-table indices of the morph targets */
} KfAnimKeyframe;

/* asset base + object_table[index]: one morph/rest vertex-delta object. */
typedef struct KfMorphObject {
    u32 unknown_00;    /* +0 */
    u32 base_vertex;   /* +4: first vertex index (byte offset == index << 3) */
    u32 vertex_count;  /* +8 */
    SVECTOR deltas[1]; /* +12: signed vertex deltas */
} KfMorphObject;

ADDRESS(0x800205d4, 0x3a4)
u16 *render_bind_animated_instance(
    void *anchor, u16 asset_index, u16 clip_index, u16 phase, u16 vertex_count)
{
    KfPoolRecord *record = *(KfPoolRecord **)anchor;
    KfAssetHeader *asset_header = asset_registry_entries[asset_index];
    KfAnimClip *clip;
    KfAnimKeyframe *keyframe;
    KfMorphObject *morph_object;
    u32 *object_table;
    u32 *source_words;
    u32 *destination_words;
    void *allocation;
    u16 vertices_left;
    u16 morphs_left;
    u16 phase_end;
    u16 phase_start;
    /* Retail uses the incoming s5 value without initializing it. */
    u16 keyframe_index;
    u16 blend_fraction;
    u16 keyframes_left;

    if (asset_header->animation_data == 0) {
        if (record != 0) {
            pool_record_release(record);
        }
        asset_registry_select(asset_index);
        tmd_select_object_vertices(0);
        return (u16 *)1;
    }

    if (record != 0) {
        goto check_record;
    }
    record = pool_allocate();
    if (record == 0) {
        return (u16 *)0;
    }

reinitialize_record:
    record->unknown_02 = asset_index;
    record->backlink = (u32 *)anchor;
    do {
        allocation = memory_malloc_checked(vertex_count << 3);
        record->allocation = allocation;
        if (allocation == 0) {
            pool_release_all();
        }
    } while (allocation == 0);
    *(KfPoolRecord **)anchor = record;
    goto find_keyframe;

check_record:
    if (record->unknown_02 == asset_index) {
        goto find_keyframe;
    }
    pool_record_release(record);
    record->value_04 = 0xff;
    goto reinitialize_record;

find_keyframe:
    phase_end = 0;
    phase_start = 0;
    clip = (KfAnimClip *)((char *)asset_header
        + ((u32 *)((char *)asset_header
                  + asset_header->clip_table_offset))[clip_index]);
    keyframes_left = clip->keyframe_count;
    if (keyframes_left != 0) {
        u32 *keyframe_offsets = clip->keyframes;

        keyframes_left = keyframes_left - 1;
        do {
            keyframe = (KfAnimKeyframe *)((char *)asset_header + *keyframe_offsets);
            keyframe_offsets++;
            phase_end += keyframe->duration;
            if (phase < phase_end) {
                u32 forward_fraction = ((u32)(u16)(phase - phase_start) << 12)
                    / keyframe->duration;

                blend_fraction = keyframe->reverse == 0
                    ? forward_fraction : 0x1000 - forward_fraction;
                goto update_vertex_cache;
            }
            phase_start = phase_end;
            keyframe_index++;
        } while (keyframes_left-- != 0);
    }
    keyframe_index--;
    blend_fraction = 0x1000;

update_vertex_cache:
    if (record->value_04 == clip_index && record->unknown_06 == keyframe_index) {
        goto blend_scratch;
    }

    object_table = (u32 *)((char *)asset_header + asset_header->object_table_offset);
    asset_registry_select(asset_index);
    tmd_select_object_vertices(0);

    source_words = (u32 *)current_tmd_vertices;
    destination_words = (u32 *)record->allocation;
    vertices_left = vertex_count;
    do {
        *destination_words++ = *source_words++;
        *destination_words++ = *source_words++;
    } while (--vertices_left != 0);

    morphs_left = keyframe->morph_count;
    if (morphs_left != 0) {
        u16 *morph_indices = keyframe->morph_indices;

        morphs_left = morphs_left - 1;
        do {
            morph_object = (KfMorphObject *)(
                (char *)asset_header + object_table[*morph_indices]);
            morph_indices++;
            gteMIMefunc(&((SVECTOR *)record->allocation)[morph_object->base_vertex],
                        morph_object->deltas, morph_object->vertex_count, 0x1000);
        } while (morphs_left-- != 0);
    }

    record->unknown_08 = (u32)(KfMorphObject *)(
        (char *)asset_header + object_table[keyframe->rest_index]);

blend_scratch:
    record->unknown_06 = keyframe_index;
    record->value_04 = clip_index;

    source_words = (u32 *)record->allocation;
    destination_words = (u32 *)&tmd_morph_scratch[1];
    vertices_left = vertex_count;
    do {
        *destination_words++ = *source_words++;
        *destination_words++ = *source_words++;
    } while (--vertices_left != 0);

    morph_object = (KfMorphObject *)record->unknown_08;
    {
        SVECTOR *scratch_vertex = &tmd_morph_scratch[morph_object->base_vertex];
        u32 saved_xy_word = ((u32 *)scratch_vertex)[0];
        u32 saved_z_pad_word = ((u32 *)scratch_vertex)[1];

        /* Blend the header-sized extra vector too, then restore its scratch entry. */
        gteMIMefunc(scratch_vertex, (SVECTOR *)&morph_object->base_vertex,
                    morph_object->vertex_count + 1, blend_fraction);
        ((u32 *)scratch_vertex)[0] = saved_xy_word;
        ((u32 *)scratch_vertex)[1] = saved_z_pad_word;
    }
    tmd_set_current_vertices(&tmd_morph_scratch[1]);
    record->state = 2;
    return (u16 *)record;
}
