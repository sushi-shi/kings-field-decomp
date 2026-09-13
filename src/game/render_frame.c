#include <kf/game_graphics.h>
#include <kf/address.h>
#include <psyq/sdk.h>
#include <kf/game_render.h>
#include <kf/notify.h>
#include <kf/game_player.h>
#include <kf/game_math.h>
#include <kf/game_state.h>
#include <kf/pool.h>

enum {
    HUD_GAUGE_WIDTH = 50,
    NOTIFICATION_RENDER_BRIGHTNESS = 255,
    NOTIFICATION_MODEL_Y = 160,
    NOTIFICATION_MODEL_Z = 200,
    HUD_CHARGE_UNITS_PER_PIXEL = KF_PLAYER_CHARGE_FULL / HUD_GAUGE_WIDTH
};

/*
 * Per-frame renderer entry, called by the player warp/update path.  It rebuilds
 * the view transform, opens the frame, refreshes the HUD gauge table, draws the
 * screen-space effect and notification sprites, sweeps the entity pools through
 * the scene dispatcher (render_entities), draws the held weapon, and presents.
 *
 * The HUD, effect, and notification sprite families use their complete table
 * layouts; unknown bytes within the records remain explicitly opaque.
 */

DATA(0x80055f68, 0xc0)
MATRIX render_light_matrices[KF_RENDER_LIGHT_COUNT] = {
    {{{0, -3600, 0}, {4096, 0, -4096}, {0, 4096, 3000}}, {0, 0, 0}},
    {{{0, 0, 4096}, {0, 0, 4096}, {0, 0, 0}}, {0, 0, 0}},
    {{{0, 0, 4096}, {0, 0, 4096}, {0, -4096, 0}}, {0, 0, 0}},
    {{{4096, 0, 0}, {0, 4096, 0}, {0, 0, 4096}}, {0, 0, 0}},
    {{{0, 0, -4096}, {3800, 0, -3800}, {-3800, 0, -3800}}, {0, 0, 0}},
    {{{0, 0, 3800}, {3800, 0, -3800}, {-3800, 0, -3800}}, {0, 0, 0}},
};

ADDRESS(0x8001fde4, 0x518)
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
    SetGeomScreen(KF_DEFAULT_PROJECTION_DISTANCE);
    render_map_cells();
    SetLightMatrix(&render_light_matrices[KF_RENDER_LIGHT_HUD]);
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
    auxiliary_sprite->state = KF_ENUM_DECODE(KfSpriteState,
        KF_ENUM_ENCODE(u8, player_state.compass_enabled));
    effect_sprites[KF_EFFECT_SPRITE_COMPASS].state = KF_ENUM_DECODE(KfSpriteState,
        KF_ENUM_ENCODE(u8, player_state.compass_enabled));
    effect_sprites[KF_EFFECT_SPRITE_COMPASS].rotation.vz = -game_graphics_runtime.render_state.view_rotation.vy & KF_ANGLE_WRAP_MASK;
    render_effect_sprites();

    game_graphics_runtime.active_render_tpage = game_graphics_runtime.hud_tpage;
    game_graphics_runtime.active_render_clut = game_graphics_runtime.hud_clut;
    game_graphics_runtime.active_render_color.b = game_graphics_runtime.hud_brightness;
    game_graphics_runtime.active_render_color.g = game_graphics_runtime.hud_brightness;
    game_graphics_runtime.active_render_color.r = game_graphics_runtime.hud_brightness;
    render_hud_gauges(auxiliary_sprite - KF_HUD_COMPASS);

    SetLightMatrix(&render_light_matrices[KF_RENDER_LIGHT_NOTIFICATION]);
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
    RotMatrix(&spin, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);

    game_graphics_runtime.active_render_tpage = game_graphics_runtime.notification_text_tpage;
    game_graphics_runtime.active_render_clut = game_graphics_runtime.notification_text_clut;
    record = notification_sprites;
    if (record[KF_NOTIFICATION_TEXT_SPRITE].active == KF_SPRITE_VISIBLE) {
        render_enqueue_sprite(&record[KF_NOTIFICATION_TEXT_SPRITE].sprite, 0, KF_SPRITE_DEPTH_CUE_NORMAL);
    }
    if (notification_sprites[KF_NOTIFICATION_GOLD_SPRITE].active == KF_SPRITE_VISIBLE) {
        render_enqueue_sprite(&record[KF_NOTIFICATION_GOLD_SPRITE].sprite, 0, KF_SPRITE_DEPTH_CUE_NORMAL);
    }
    record += KF_NOTIFICATION_ONES_SPRITE;
    game_graphics_runtime.active_render_tpage = game_graphics_runtime.notification_digit_tpage;
    game_graphics_runtime.active_render_clut = game_graphics_runtime.notification_digit_clut;
    for (i = KF_NOTIFICATION_THOUSANDS_SPRITE - KF_NOTIFICATION_ONES_SPRITE; i != -1; i--) {
        if (record->active == KF_SPRITE_VISIBLE) {
            render_enqueue_sprite(&record->sprite, 0, KF_SPRITE_DEPTH_CUE_NORMAL);
        }
        record++;
    }

    render_entities();
    render_weapon();
    display_present_frame();
    pool_release_stale();
}
