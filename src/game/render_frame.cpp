#include <kf/game/graphics.h>

#include <kf/lib/geometry_types.h>
#include <kf/game/render.h>
#include <kf/game/notify.h>
#include <kf/game/player.h>
#include <kf/lib/math.h>
#include <kf/game/state.h>
#include <kf/game/pool.h>
#include <kf/game/system.h>

enum {
    HUD_GAUGE_WIDTH = 50,
    NOTIFICATION_RENDER_BRIGHTNESS = 255,
    NOTIFICATION_MODEL_Y = 160,
    NOTIFICATION_MODEL_Z = 200,
    HUD_CHARGE_UNITS_PER_PIXEL = KF_PLAYER_CHARGE_FULL / HUD_GAUGE_WIDTH
};

MATRIX render_light_matrices[KF_RENDER_LIGHT_COUNT] = {
    {{{0, -3600, 0}, {4096, 0, -4096}, {0, 4096, 3000}}, {0, 0, 0}},
    {{{0, 0, 4096}, {0, 0, 4096}, {0, 0, 0}}, {0, 0, 0}},
    {{{0, 0, 4096}, {0, 0, 4096}, {0, -4096, 0}}, {0, 0, 0}},
    {{{4096, 0, 0}, {0, 4096, 0}, {0, 0, 4096}}, {0, 0, 0}},
    {{{0, 0, -4096}, {3800, 0, -3800}, {-3800, 0, -3800}}, {0, 0, 0}},
    {{{0, 0, 3800}, {3800, 0, -3800}, {-3800, 0, -3800}}, {0, 0, 0}},
};

void render_frame(const VECTOR *position_or_null, const SVECTOR *rotation_or_null)
{
    MATRIX model;
    SVECTOR spin;
    KfHudSprite *status_sprite;
    KfHudSprite *auxiliary_sprite;
    KfNotificationSprite *record;
    s16 i;

    render_set_view_transform(position_or_null, rotation_or_null);
    display_begin_frame();
    pool_mark_allocated();
    render_map_cells();
    status_sprite = &hud_sprites[KF_HUD_POISON_ICON];
    status_sprite->state = KF_SPRITE_HIDDEN;
    hud_sprites[KF_HUD_SLOWED_ICON].state = KF_SPRITE_HIDDEN;
    hud_sprites[KF_HUD_DARKNESS_ICON].state = KF_SPRITE_HIDDEN;
    hud_sprites[KF_HUD_CURSE_ICON].state = KF_SPRITE_HIDDEN;
    if (player_state.hud_gauges_enabled == KF_PLAYER_OPTION_ON) {
        KfPlayerStatusFlags flags;
        hud_sprites[KF_HUD_HP_GAUGE].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_MP_GAUGE].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_ATTACK_GAUGE].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_MAGIC_GAUGE].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_HP_PANEL].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_MP_PANEL].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_ATTACK_PANEL].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_MAGIC_PANEL].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_HP_GAUGE].sprite.w = (player_state.vitals.current_hp * HUD_GAUGE_WIDTH
                                   + (player_state.vitals.maximum_hp - 1) / HUD_GAUGE_WIDTH)
                                  / player_state.vitals.maximum_hp;
        hud_sprites[KF_HUD_MP_GAUGE].sprite.w = (player_state.vitals.current_mp * HUD_GAUGE_WIDTH
                                   + (player_state.vitals.maximum_mp - 1) / HUD_GAUGE_WIDTH)
                                  / player_state.vitals.maximum_mp;
        hud_sprites[KF_HUD_ATTACK_GAUGE].sprite.w = player_state.attack_charge_state.current / HUD_CHARGE_UNITS_PER_PIXEL;
        hud_sprites[KF_HUD_MAGIC_GAUGE].sprite.w = player_state.magic_charge / HUD_CHARGE_UNITS_PER_PIXEL;
        flags = player_state.status_effect_flags;
        if ((flags & KF_PLAYER_STATUS_CURSE) != KF_PLAYER_STATUS_NONE) {
            hud_sprites[KF_HUD_CURSE_ICON].state = KF_SPRITE_VISIBLE;
        } else if ((flags & KF_PLAYER_STATUS_DARKNESS) != KF_PLAYER_STATUS_NONE) {
            hud_sprites[KF_HUD_DARKNESS_ICON].state = KF_SPRITE_VISIBLE;
        } else if ((flags & KF_PLAYER_STATUS_POISON) != KF_PLAYER_STATUS_NONE) {
            status_sprite->state = KF_SPRITE_VISIBLE;
        } else if ((flags & KF_PLAYER_STATUS_SLOWED) != KF_PLAYER_STATUS_NONE) {
            hud_sprites[KF_HUD_SLOWED_ICON].state = KF_SPRITE_VISIBLE;
        }
    } else {
        hud_sprites[KF_HUD_HP_GAUGE].state = KF_SPRITE_HIDDEN;
        hud_sprites[KF_HUD_MP_GAUGE].state = KF_SPRITE_HIDDEN;
        hud_sprites[KF_HUD_ATTACK_GAUGE].state = KF_SPRITE_HIDDEN;
        hud_sprites[KF_HUD_MAGIC_GAUGE].state = KF_SPRITE_HIDDEN;
        hud_sprites[KF_HUD_HP_PANEL].state = KF_SPRITE_HIDDEN;
        hud_sprites[KF_HUD_MP_PANEL].state = KF_SPRITE_HIDDEN;
        hud_sprites[KF_HUD_ATTACK_PANEL].state = KF_SPRITE_HIDDEN;
        hud_sprites[KF_HUD_MAGIC_PANEL].state = KF_SPRITE_HIDDEN;
    }

    auxiliary_sprite = &hud_sprites[KF_HUD_COMPASS];
    auxiliary_sprite->state = kf_enum_decode<KfSpriteState>(kf_enum_encode<u8>(player_state.compass_enabled));
    effect_sprites[KF_EFFECT_SPRITE_COMPASS].state = kf_enum_decode<KfSpriteState>(kf_enum_encode<u8>(player_state.compass_enabled));
    effect_sprites[KF_EFFECT_SPRITE_COMPASS].rotation.vz = -game_graphics_runtime.render_state.view_rotation.vy & KF_ANGLE_WRAP_MASK;
    render_effect_sprites(&render_light_matrices[KF_RENDER_LIGHT_HUD]);

    game_graphics_runtime.active_render_material = game_graphics_runtime.hud_material;
    game_graphics_runtime.active_render_color.b = game_graphics_runtime.hud_brightness;
    game_graphics_runtime.active_render_color.g = game_graphics_runtime.hud_brightness;
    game_graphics_runtime.active_render_color.r = game_graphics_runtime.hud_brightness;
    render_hud_gauges(auxiliary_sprite - KF_HUD_COMPASS);

    notify_effect_update();

    game_graphics_runtime.active_render_color.r = NOTIFICATION_RENDER_BRIGHTNESS;
    game_graphics_runtime.active_render_color.g = NOTIFICATION_RENDER_BRIGHTNESS;
    game_graphics_runtime.active_render_color.b = NOTIFICATION_RENDER_BRIGHTNESS;
    model.t[0] = 0;
    model.t[1] = NOTIFICATION_MODEL_Y;
    model.t[2] = NOTIFICATION_MODEL_Z;
    spin.vz = 0;
    spin.vy = 0;
    spin.vx = game_graphics_runtime.notification_state.control.effect_angle_x;
    kf::matrix_set_rotation_xyz(spin, model);

    game_graphics_runtime.active_render_material = game_graphics_runtime.notification_text_material;
    record = notification_sprites;
    if (record[KF_NOTIFICATION_TEXT_SPRITE].active == KF_SPRITE_VISIBLE) {
        render_enqueue_sprite(&record[KF_NOTIFICATION_TEXT_SPRITE].sprite, 0, KF_SPRITE_DEPTH_CUE_NORMAL, &render_light_matrices[KF_RENDER_LIGHT_NOTIFICATION], &model, game_graphics_runtime.render_state.projection);
    }
    if (notification_sprites[KF_NOTIFICATION_GOLD_SPRITE].active == KF_SPRITE_VISIBLE) {
        render_enqueue_sprite(&record[KF_NOTIFICATION_GOLD_SPRITE].sprite, 0, KF_SPRITE_DEPTH_CUE_NORMAL, &render_light_matrices[KF_RENDER_LIGHT_NOTIFICATION], &model, game_graphics_runtime.render_state.projection);
    }
    record += KF_NOTIFICATION_ONES_SPRITE;
    game_graphics_runtime.active_render_material = game_graphics_runtime.notification_digit_material;
    for (i = KF_NOTIFICATION_THOUSANDS_SPRITE - KF_NOTIFICATION_ONES_SPRITE; i != -1; i--) {
        if (record->active == KF_SPRITE_VISIBLE) {
            render_enqueue_sprite(&record->sprite, 0, KF_SPRITE_DEPTH_CUE_NORMAL, &render_light_matrices[KF_RENDER_LIGHT_NOTIFICATION], &model, game_graphics_runtime.render_state.projection);
        }
        record++;
    }

    render_entities();
    render_weapon();
    display_present_frame();
    pool_release_stale();
    // World rendering also advances floor sprites and notifications. Pace every
    // caller, including blocking scripts; presentation consumes this deadline.
    frame_pacer_wait();
}


void render_frame_reset_module_state(void)
{
    kf::restore_initial_value<render_light_matrices>();
}
