#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_asset.h>
#include <kf/game_math.h>
#include <kf/game_player.h>
#include <kf/game_render.h>
#include <kf/psyq.h>

enum {
    WEAPON_PROJECTED_DEPTH_SHIFT = 3,
    WEAPON_DEPTH_BIAS_SHIFT = 5,
    WEAPON_BASE_DEPTH_BIAS = 50
};

DATA(0x80055c5c, 0xc4)
KfHudSprite hud_sprites[KF_HUD_TABLE_ROWS] = {
    {KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x15, 0x32, 5}},
    {KF_HUD_VISIBLE, 0, {0x50, 0x03, 1, 5, 0x1f, 0x23, 0x32, 5}},
    {KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x16, 0x32, 2}},
    {KF_HUD_VISIBLE, 0, {0x50, 0x15, 1, 2, 0x81, 0x24, 0x32, 2}},
    {KF_HUD_HIDDEN, 0, {0, 0x50, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},
    {KF_HUD_HIDDEN, 0, {0, 0x60, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},
    {KF_HUD_HIDDEN, 0, {0, 0x40, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},
    {KF_HUD_HIDDEN, 0, {0, 0x70, 0x38, 0x10, 0xc7, 0x16, 0x38, 0x10}},
    {KF_HUD_VISIBLE, 0, {0, 0, 0x4a, 0x0b, 0x0a, 0x12, 0x4a, 0x0b}},
    {KF_HUD_VISIBLE, 0, {0, 0x10, 0x4a, 0x0b, 0x0a, 0x20, 0x4a, 0x0b}},
    {KF_HUD_VISIBLE, 0, {0, 0x20, 0x5c, 8, 0x5e, 0x13, 0x5c, 8}},
    {KF_HUD_VISIBLE, 0, {0, 0x30, 0x5c, 8, 0x5e, 0x21, 0x5c, 8}},
    {KF_HUD_VISIBLE, 0, {0, 0x80, 0x21, 0x20, 0x10d, 0x12, 0x21, 0x20}},
    {KF_HUD_END, 0, {0xff, 0xff, 0xff, 0xff, 0xffff, 0xffff, 0xffff, 0xffff}},
};

/*
 * Screen-space geometry emitters that run every frame from the top-level frame
 * renderer (render_frame).  Each walks a per-subsystem table of live entries,
 * feeds one through the GTE, and hands the projected model to the shared
 * display-list builder.
 */

/* Six light/color matrices fed to SetLightMatrix, one per render subsystem. */

/*
 * Draws the equipped weapon model held in the player's view.  Skips entirely
 * while no weapon swing is in progress (attack phase -1).  The weapon record
 * carries its own geometry-screen distance, an in-view translation, and a
 * rotation vector; the record's render-transform fields overlap the currently
 * opaque interior of KfWeaponRecord, so they are read through byte views until
 * that object's render block is modelled.  The projected depth is biased by the
 * record's Z translation before the model is enqueued.
 */
ADDRESS(0x8001f798, 0x118)
void render_weapon(void)
{
    MATRIX model;
    const KfWeaponRecord *weapon;
    const u8 *fields;
    KfTmdObject *object;
    s32 depth_bias;

    if (player_state.weapon_attack_phase == KF_WEAPON_ATTACK_INACTIVE) {
        return;
    }
    SetLightMatrix(&render_light_matrices[KF_RENDER_LIGHT_WEAPON]);
    SetGeomScreen(*(const u16 *)((const u8 *)player_state.equipped_weapon_record + 16));
    weapon = player_state.equipped_weapon_record;
    fields = (const u8 *)weapon;
    model.t[0] = *(const s16 *)(fields + 28);
    model.t[1] = *(const s16 *)(fields + 30);
    model.t[2] = *(const s16 *)(fields + 32);
    RotMatrix((SVECTOR *)(fields + 36), &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);
    asset_registry_select(KF_ASSET_WEAPON);
    object = tmd_get_object(0);
    if (render_bind_animated_instance(
            &player_state.weapon_animation_cache, KF_ASSET_WEAPON, 0,
            player_state.weapon_attack_phase,
            object->vertex_count) != 0) {
        tmd_project_vertices_shift(object->vertex_count, WEAPON_PROJECTED_DEPTH_SHIFT);
        depth_bias =
            (s16)*(const u16 *)((const u8 *)player_state.equipped_weapon_record + 32) >> WEAPON_DEPTH_BIAS_SHIFT;
        render_enqueue_tmd(0, -depth_bias + WEAPON_BASE_DEPTH_BIAS);
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
    SetColorMatrix(&game_graphics_runtime.render_state.effect_color_matrix);
    scale.vz = KF_FIXED12_ONE;
    entry = effect_sprites;
    while (entry->state == KF_EFFECT_SPRITE_ACTIVE) {
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
        asset_registry_select(KF_ASSET_EFFECT_SPRITES);
        object = tmd_get_object(0);
        if (render_bind_animated_instance(
                &entry->animation_cache, KF_ASSET_EFFECT_SPRITES,
                entry->visibility_tag, entry->asset_variant,
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
void render_hud_gauges(KfHudSprite *table)
{
    KfHudSprite *entry;

    entry = table;
    if (entry->state == KF_HUD_END) {
        return;
    }
    do {
        if (entry->state == KF_HUD_VISIBLE) {
            render_screen_sprite(&entry->sprite);
        }
        entry++;
    } while (entry->state != KF_HUD_END);
}
