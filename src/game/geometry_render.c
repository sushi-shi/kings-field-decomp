#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Screen-space geometry emitters that run every frame from the top-level frame
 * renderer (render_frame).  Each walks a per-subsystem table of live entries,
 * feeds one through the GTE, and hands the projected model to the shared
 * display-list builder.
 */

/* Six light/color matrices fed to SetLightMatrix, one per render subsystem. */

/*
 * One entry of the animated decal/sprite table at DAT_80055d74 (stride 28).
 * WIP: field roles beyond the transform inputs are unresolved, so the trailing
 * bytes stay opaque until the table's producers are reconstructed.
 */
typedef struct KfEffectSprite {
    u8 state;          /* +0: 1 while the slot is live */
    u8 visibility_tag; /* +1 */
    u16 asset_variant; /* +2 */
    u16 scale;         /* +4: uniform X/Y scale numerator */
    s16 translation_x; /* +6 */
    s16 translation_y; /* +8 */
    s16 translation_z; /* +10 */
    u8 unknown_0c[2];  /* +12 */
    SVECTOR rotation;  /* +14 */
    u8 unknown_16[2];  /* +22 */
    u8 anchor[4];      /* +24: visibility/projection anchor */
} KfEffectSprite;

extern KfEffectSprite DAT_80055d74[];

/*
 * Draws the equipped weapon model held in the player's view.  Skips entirely
 * while no weapon swing is in progress (attack phase -1).  The weapon record
 * carries its own geometry-screen distance, an in-view translation, and a
 * rotation vector; the record's render-transform fields overlap the currently
 * opaque interior of KfWeaponRecord, so they are read through byte views until
 * that object's render block is modelled.  The projected depth is biased by the
 * record's swing angle before the model is enqueued.
 */
ADDRESS(0x8001f798, 0x118)
void render_weapon(void)
{
    MATRIX model;
    const KfWeaponRecord *weapon;
    const u8 *fields;
    KfTmdObject *object;
    s32 depth_bias;

    if (player_state.weapon_attack_phase == -1) {
        return;
    }
    SetLightMatrix(&render_light_matrices[3]);
    SetGeomScreen(*(const u16 *)((const u8 *)player_state.equipped_weapon_record + 16));
    weapon = player_state.equipped_weapon_record;
    fields = (const u8 *)weapon;
    model.t[0] = *(const s16 *)(fields + 28);
    model.t[1] = *(const s16 *)(fields + 30);
    model.t[2] = *(const s16 *)(fields + 32);
    RotMatrix((SVECTOR *)(fields + 36), &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);
    asset_registry_select(0x14);
    object = tmd_get_object(0);
    if (render_bind_animated_instance(
            &player_state.unknown_74, 0x14, 0, player_state.weapon_attack_phase,
            object->vertex_count) != 0) {
        tmd_project_vertices_shift(object->vertex_count, 3);
        depth_bias =
            (s16)*(const u16 *)((const u8 *)player_state.equipped_weapon_record + 32) >> 5;
        render_enqueue_tmd(0, -depth_bias + 50);
    }
}

/*
 * Draws the animated decal/sprite list.  The current color matrix is saved and
 * replaced with render_state's sprite color matrix for the whole pass and
 * restored afterwards.  Each live entry builds a rotated, uniformly scaled
 * model matrix with the translation folded into its t column, tests visibility
 * through render_bind_animated_instance, and, if visible, transforms and enqueues asset 0x15.
 */
ADDRESS(0x8001f8b0, 0x124)
void render_effect_sprites(void)
{
    MATRIX model;
    VECTOR scale;
    MATRIX saved_color_matrix;
    KfEffectSprite *entry;
    KfTmdObject *object;
    u16 scale_numerator;

    ReadColorMatrix(&saved_color_matrix);
    SetColorMatrix(&render_state.unknown_80);
    scale.vz = 0x1000;
    entry = DAT_80055d74;
    while (entry->state == 1) {
        model.t[0] = entry->translation_x;
        model.t[1] = entry->translation_y;
        model.t[2] = entry->translation_z;
        RotMatrix(&entry->rotation, &model);
        scale_numerator = entry->scale;
        scale.vy = scale_numerator;
        scale.vx = scale_numerator;
        ScaleMatrix(&model, &scale);
        SetRotMatrix(&model);
        SetTransMatrix(&model);
        asset_registry_select(0x15);
        object = tmd_get_object(0);
        if (render_bind_animated_instance(
                &entry->anchor, 0x15, entry->visibility_tag, entry->asset_variant,
                object->vertex_count) != 0) {
            tmd_transform_vertices(object->vertex_count);
            render_enqueue_tmd(0, 0);
        }
        entry++;
    }
    SetColorMatrix(&saved_color_matrix);
}

/*
 * Walks a stride-14 table terminated by a 0xff type byte and dispatches every
 * active (type == 1) entry to render_screen_sprite, pointing it past the two-byte
 * header.  The table base is supplied by the caller.
 */
ADDRESS(0x8001f9d4, 0x70)
void render_hud_gauges(u8 *table)
{
    u8 *entry;

    entry = table;
    if (entry[0] == 0xff) {
        return;
    }
    do {
        if (entry[0] == 1) {
            render_screen_sprite((KfSpriteQuad *)(entry + 2));
        }
        entry += 14;
    } while (entry[0] != 0xff);
}
