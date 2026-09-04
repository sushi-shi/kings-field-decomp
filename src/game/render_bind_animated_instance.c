#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/game_asset.h>
#include <kf/game_types.h>
#include <kf/game.h>

/*
 * Binds and refreshes one animated model instance for the entity renderers
 * (render_actor, render_actor_sprite, render_map_event, render_weapon,
 * render_effect_sprites).  The instance's vertex cache lives in a pool record
 * reached through the caller's anchor slot.  The routine selects the animation
 * clip (tag) and the keyframe that the phase (variant) falls into, morphs the
 * base TMD vertices toward that keyframe at full weight, then blends the rest
 * pose toward the next keyframe by the sub-keyframe fraction before installing
 * the result as the current TMD vertex array.  A same-tag/same-keyframe hit
 * reuses the cached morph and only re-runs the per-frame fractional blend.
 *
 * WIP: the animation asset format (clip/keyframe/morph tables) and the scratch
 * vertex buffers at 0x800930f0/0x800930f8 are modelled locally until their
 * owners are reconstructed.  The keyframe index (kf_index) is deliberately left
 * uninitialised, matching the retail body.
 */

/* asset base + clip_table[tag]: one animation clip. */
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
u16 *render_bind_animated_instance(void *anchor, u16 asset, u16 tag, u16 variant, u16 count)
{
    KfPoolRecord *record = *(KfPoolRecord **)anchor;
    KfAssetHeader *ah = asset_registry_entries[asset];
    KfAnimClip *clip;
    KfAnimKeyframe *kf;
    KfMorphObject *obj;
    u32 *object_table;
    u32 *s;
    u32 *d;
    void *alloc;
    u16 vc;
    u16 mc;
    u16 accum;
    u16 prev;
    u16 kf_index;
    u16 frac;
    u16 i;

    if (ah->animation_data == 0) {
        if (record != 0) {
            pool_record_release(record);
        }
        asset_registry_select(asset);
        tmd_select_object_vertices(0);
        return (u16 *)1;
    }

    if (record != 0) {
        goto have_record;
    }
    record = pool_allocate();
    if (record == 0) {
        return (u16 *)0;
    }

reinit:
    record->unknown_02 = asset;
    record->backlink = (u32 *)anchor;
    do {
        alloc = memory_malloc_checked(count << 3);
        record->allocation = alloc;
        if (alloc == 0) {
            pool_release_all();
        }
    } while (alloc == 0);
    *(KfPoolRecord **)anchor = record;
    goto search;

have_record:
    if (record->unknown_02 == asset) {
        goto search;
    }
    pool_record_release(record);
    record->value_04 = 0xff;
    goto reinit;

search:
    accum = 0;
    prev = 0;
    clip = (KfAnimClip *)((char *)ah
                          + ((u32 *)((char *)ah + ah->clip_table_offset))[tag]);
    i = clip->keyframe_count;
    if (i != 0) {
        u32 *kfp = clip->keyframes;

        i = i - 1;
        do {
            kf = (KfAnimKeyframe *)((char *)ah + *kfp);
            kfp++;
            accum += kf->duration;
            if (variant < accum) {
                u32 f = ((u32)(u16)(variant - prev) << 12) / kf->duration;

                frac = kf->reverse == 0 ? f : 0x1000 - f;
                goto blend;
            }
            prev = accum;
            kf_index++;
        } while (i-- != 0);
    }
    kf_index--;
    frac = 0x1000;

blend:
    if (record->value_04 == tag && record->unknown_06 == kf_index) {
        goto finalize;
    }

    object_table = (u32 *)((char *)ah + ah->object_table_offset);
    asset_registry_select(asset);
    tmd_select_object_vertices(0);

    s = (u32 *)current_tmd_vertices;
    d = (u32 *)record->allocation;
    vc = count;
    do {
        *d++ = *s++;
        *d++ = *s++;
    } while (--vc != 0);

    mc = kf->morph_count;
    if (mc != 0) {
        u16 *mi = kf->morph_indices;

        mc = mc - 1;
        do {
            obj = (KfMorphObject *)((char *)ah + object_table[*mi]);
            mi++;
            gteMIMefunc(&((SVECTOR *)record->allocation)[obj->base_vertex],
                        obj->deltas, obj->vertex_count, 0x1000);
        } while (mc-- != 0);
    }

    record->unknown_08 = (u32)(KfMorphObject *)((char *)ah + object_table[kf->rest_index]);

finalize:
    record->unknown_06 = kf_index;
    record->value_04 = tag;

    s = (u32 *)record->allocation;
    d = DAT_800930f8;
    vc = count;
    do {
        *d++ = *s++;
        *d++ = *s++;
    } while (--vc != 0);

    obj = (KfMorphObject *)record->unknown_08;
    {
        SVECTOR *dst = &DAT_800930f0[obj->base_vertex];
        u32 save0 = ((u32 *)dst)[0];
        u32 save1 = ((u32 *)dst)[1];

        gteMIMefunc(dst, (SVECTOR *)((char *)obj + 4), obj->vertex_count + 1, frac);
        ((u32 *)dst)[0] = save0;
        ((u32 *)dst)[1] = save1;
    }
    tmd_set_current_vertices(&DAT_800930f0[1]);
    record->state = 2;
    return (u16 *)record;
}
