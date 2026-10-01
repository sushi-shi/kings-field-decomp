#include <kf/platform/frame_task.hpp>
#include <kf/game/world.h>
#include <kf/platform/prelude.h>
#include <kf/game/animation_cache.h>
#include <kf/game/graphics.h>
#include <kf/game/notify.h>
#include <kf/game/player.h>
#include <kf/game/render.h>
#include <kf/game/state.h>
#include <kf/game/system.h>
#include <kf/lib/geometry_types.h>
#include <kf/lib/math.h>

#include <array>

enum {
    HUD_GAUGE_WIDTH = 50,
    NOTIFICATION_RENDER_BRIGHTNESS = 255,
    NOTIFICATION_MODEL_Y = 160,
    NOTIFICATION_MODEL_Z = 200,
    HUD_CHARGE_UNITS_PER_PIXEL = KF_PLAYER_CHARGE_FULL / HUD_GAUGE_WIDTH
};

std::array<MATRIX, KF_RENDER_LIGHT_COUNT> render_light_matrices = {
    MATRIX{
        .m = {{
            {0, -3600, 0},
            {4096, 0, -4096},
            {0, 4096, 3000},
        }},
        .t = {},
    },
    MATRIX{
        .m = {{
            {0, 0, 4096},
            {0, 0, 4096},
            {0, 0, 0},
        }},
        .t = {},
    },
    MATRIX{
        .m = {{
            {0, 0, 4096},
            {0, 0, 4096},
            {0, -4096, 0},
        }},
        .t = {},
    },
    MATRIX{
        .m = {{
            {4096, 0, 0},
            {0, 4096, 0},
            {0, 0, 4096},
        }},
        .t = {},
    },
    MATRIX{
        .m = {{
            {0, 0, -4096},
            {3800, 0, -3800},
            {-3800, 0, -3800},
        }},
        .t = {},
    },
    MATRIX{
        .m = {{
            {0, 0, 3800},
            {3800, 0, -3800},
            {-3800, 0, -3800},
        }},
        .t = {},
    },
};

void render_world_frame(WorldState &world, PlayerContext &player, const VECTOR *position_or_null, const SVECTOR *rotation_or_null)
{
    MATRIX model;
    SVECTOR spin;
    KfHudSprite *poison_icon;
    KfHudSprite *compass_sprite;
    KfNotificationSprite *record;
    s16 i;

    render_set_view_transform(game_graphics_runtime.render_state, position_or_null, rotation_or_null);
    display_begin_frame(game_graphics_runtime.display_state);
    animation_cache_mark_stale();
    render_map_cells(world, player);
    poison_icon = &hud_sprites[KF_HUD_POISON_ICON];
    poison_icon->state = KF_SPRITE_HIDDEN;
    hud_sprites[KF_HUD_SLOWED_ICON].state = KF_SPRITE_HIDDEN;
    hud_sprites[KF_HUD_DARKNESS_ICON].state = KF_SPRITE_HIDDEN;
    hud_sprites[KF_HUD_CURSE_ICON].state = KF_SPRITE_HIDDEN;
    if (player.state.hud_gauges_enabled == KF_PLAYER_OPTION_ON) {
        KfPlayerStatusFlags flags;
        hud_sprites[KF_HUD_HP_GAUGE].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_MP_GAUGE].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_ATTACK_GAUGE].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_MAGIC_GAUGE].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_HP_PANEL].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_MP_PANEL].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_ATTACK_PANEL].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_MAGIC_PANEL].state = KF_SPRITE_VISIBLE;
        hud_sprites[KF_HUD_HP_GAUGE].sprite.w = (player.state.vitals.current_hp * HUD_GAUGE_WIDTH
                                   + (player.state.vitals.maximum_hp - 1) / HUD_GAUGE_WIDTH)
                                  / player.state.vitals.maximum_hp;
        hud_sprites[KF_HUD_MP_GAUGE].sprite.w = (player.state.vitals.current_mp * HUD_GAUGE_WIDTH
                                   + (player.state.vitals.maximum_mp - 1) / HUD_GAUGE_WIDTH)
                                  / player.state.vitals.maximum_mp;
        hud_sprites[KF_HUD_ATTACK_GAUGE].sprite.w = player.state.attack_charge_state.current / HUD_CHARGE_UNITS_PER_PIXEL;
        hud_sprites[KF_HUD_MAGIC_GAUGE].sprite.w = player.state.magic_charge / HUD_CHARGE_UNITS_PER_PIXEL;
        flags = player.state.status_effect_flags;
        if ((flags & KF_PLAYER_STATUS_CURSE) != KF_PLAYER_STATUS_NONE) {
            hud_sprites[KF_HUD_CURSE_ICON].state = KF_SPRITE_VISIBLE;
        } else if ((flags & KF_PLAYER_STATUS_DARKNESS) != KF_PLAYER_STATUS_NONE) {
            hud_sprites[KF_HUD_DARKNESS_ICON].state = KF_SPRITE_VISIBLE;
        } else if ((flags & KF_PLAYER_STATUS_POISON) != KF_PLAYER_STATUS_NONE) {
            poison_icon->state = KF_SPRITE_VISIBLE;
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

    compass_sprite = &hud_sprites[KF_HUD_COMPASS];
    compass_sprite->state = kf_enum_decode<KfSpriteState>(kf_enum_encode<u8>(player.state.compass_enabled));
    hud_models[KF_HUD_MODEL_COMPASS].state = kf_enum_decode<KfSpriteState>(kf_enum_encode<u8>(player.state.compass_enabled));
    hud_models[KF_HUD_MODEL_COMPASS].rotation.vz = -game_graphics_runtime.render_state.view_rotation.vy & KF_ANGLE_WRAP_MASK;
    render_hud_models(&render_light_matrices[KF_RENDER_LIGHT_HUD]);

    game_graphics_runtime.active_render_material = game_graphics_runtime.hud_material;
    game_graphics_runtime.active_render_color.b = game_graphics_runtime.hud_brightness;
    game_graphics_runtime.active_render_color.g = game_graphics_runtime.hud_brightness;
    game_graphics_runtime.active_render_color.r = game_graphics_runtime.hud_brightness;
    render_hud_sprites(compass_sprite - KF_HUD_COMPASS);


    game_graphics_runtime.active_render_color.r = NOTIFICATION_RENDER_BRIGHTNESS;
    game_graphics_runtime.active_render_color.g = NOTIFICATION_RENDER_BRIGHTNESS;
    game_graphics_runtime.active_render_color.b = NOTIFICATION_RENDER_BRIGHTNESS;
    model.t[0] = 0;
    model.t[1] = NOTIFICATION_MODEL_Y;
    model.t[2] = NOTIFICATION_MODEL_Z;
    spin = VECTOR{game_graphics_runtime.notification_state.control.effect_angle_x, 0, 0}.narrowed();
    kf::matrix_set_rotation_xyz(spin, model);

    game_graphics_runtime.active_render_material = game_graphics_runtime.notification_text_material;
    record = notification_sprites.data();
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

    render_party(world, player.party_slot);
    render_entities(world);
    render_weapon(player);
    kf::host_present_frame(game_graphics_runtime.display_state.frame_style);
    animation_cache_release_stale();
}

kf::FrameTask<void> render_frame(WorldState &world, PlayerContext &player,
    const VECTOR *position_or_null, const SVECTOR *rotation_or_null)
{
    notify_effect_update();
    render_world_frame(world, player, position_or_null, rotation_or_null);
    presentation_advance_floor_items();
    (co_await frame_pacer_wait());
}

void render_frame_reset_module_state(void)
{
    kf::restore_initial_value<render_light_matrices>();
}
