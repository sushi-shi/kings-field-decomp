#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/game_render.h>
#include <kf/notify.h>
#include <kf/game_player.h>
#include <kf/game_state.h>
#include <kf/pool.h>

/*
 * Per-frame renderer entry, called by the player warp/update path.  It rebuilds
 * the view transform, opens the frame, refreshes the HUD gauge table, draws the
 * screen-space effect and notification sprites, sweeps the entity pools through
 * the scene dispatcher (render_entities), draws the held weapon, and presents.
 *
 * The HUD, effect, and notification sprite families use their complete table
 * layouts; unknown bytes within the records remain explicitly opaque.
 */

DATA(0x80055d74, 0x38)
KfEffectSprite effect_sprites[2] = {
    {1, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, 0},
    {0xff},
};

ADDRESS(0x8001fde4, 0x518)
void render_frame(const VECTOR *position, const SVECTOR *rotation)
{
    MATRIX model;
    SVECTOR spin;
    KfHudSprite *status_sprite;
    KfHudSprite *auxiliary_sprite;
    KfNotificationSprite *record;
    s16 i;

    render_set_view_transform(position, rotation);
    display_begin_frame();
    pool_mark_allocated();
    SetGeomScreen(0xc8);
    render_map_cells();
    SetLightMatrix(&render_light_matrices[4]);
    status_sprite = &hud_sprites[4];
    status_sprite->state = 0;
    hud_sprites[5].state = 0;
    hud_sprites[6].state = 0;
    hud_sprites[7].state = 0;
    if (player_state.unknown_98[0] == 1) {
        u16 flags;
        hud_sprites[0].state = 1;
        hud_sprites[1].state = 1;
        hud_sprites[2].state = 1;
        hud_sprites[3].state = 1;
        hud_sprites[8].state = 1;
        hud_sprites[9].state = 1;
        hud_sprites[10].state = 1;
        hud_sprites[11].state = 1;
        hud_sprites[0].sprite.w = (player_state.vitals.current_hp * 50
                                   + (player_state.vitals.maximum_hp - 1) / 50)
                                  / player_state.vitals.maximum_hp;
        hud_sprites[1].sprite.w = (player_state.vitals.current_mp * 50
                                   + (player_state.vitals.maximum_mp - 1) / 50)
                                  / player_state.vitals.maximum_mp;
        hud_sprites[2].sprite.w = player_state.attack_charge_state.current / 100;
        hud_sprites[3].sprite.w = player_state.magic_charge / 100;
        flags = player_state.status_effect_flags;
        if (flags & 1) {
            hud_sprites[7].state = 1;
        } else if (flags & 2) {
            hud_sprites[6].state = 1;
        } else if (flags & 4) {
            status_sprite->state = 1;
        } else if (flags & 8) {
            hud_sprites[5].state = 1;
        }
    } else {
        hud_sprites[0].state = 0;
        hud_sprites[1].state = 0;
        hud_sprites[2].state = 0;
        hud_sprites[3].state = 0;
        hud_sprites[8].state = 0;
        hud_sprites[9].state = 0;
        hud_sprites[10].state = 0;
        hud_sprites[11].state = 0;
    }

    auxiliary_sprite = &hud_sprites[12];
    auxiliary_sprite->state = player_state.unknown_98[1];
    effect_sprites[0].state = player_state.unknown_98[1];
    effect_sprites[0].rotation.vz = -render_state.view_rotation.vy & 0xfff;
    render_effect_sprites();

    active_render_tpage = DAT_80095062;
    active_render_clut = DAT_80095060;
    active_render_blue = DAT_80095064;
    active_render_green = DAT_80095064;
    active_render_red = DAT_80095064;
    render_hud_gauges(auxiliary_sprite - 12);

    SetLightMatrix(&render_light_matrices[5]);
    notify_effect_update();

    active_render_red = 0xff;
    active_render_green = 0xff;
    active_render_blue = 0xff;
    model.t[0] = 0;
    model.t[1] = 0xa0;
    model.t[2] = 0xc8;
    spin.vz = 0;
    spin.vy = 0;
    spin.vx = notification_state.effect_angle_x;
    RotMatrix(&spin, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);

    active_render_tpage = DAT_80095068;
    active_render_clut = DAT_80095066;
    record = notification_sprites;
    if (record[0].active == 1) {
        render_enqueue_sprite(&record[0].sprite, 0, 0);
    }
    if (notification_sprites[1].active == 1) {
        render_enqueue_sprite(&record[1].sprite, 0, 0);
    }
    record += 2;
    active_render_tpage = DAT_8009506c;
    active_render_clut = DAT_8009506a;
    for (i = 3; i != -1; i--) {
        if (record->active == 1) {
            render_enqueue_sprite(&record->sprite, 0, 0);
        }
        record++;
    }

    render_entities();
    render_weapon();
    display_present_frame();
    pool_release_stale();
}
