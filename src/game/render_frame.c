#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Per-frame renderer entry, called by the player warp/update path.  It rebuilds
 * the view transform, opens the frame, refreshes the HUD gauge table, draws the
 * screen-space effect and notification sprites, sweeps the entity pools through
 * the scene dispatcher (render_entities), draws the held weapon, and presents.
 *
 * WIP: the HUD gauge records at DAT_80055c5c (stride 14, walked by
 * render_hud_gauges) remain unresolved and are reached by their individual
 * identities.
 */

/* HUD gauge table: thirteen drawable 14-byte rows followed by a terminator. */

DATA(0x80055d74, 0x38)
KfEffectSprite effect_sprites[2] = {
    {1, 0, 0, 0x33, 0x11e, 0x22, 0xc8, {0, 0}, {0, 0, 0, 0}, {0, 0}, {0, 0, 0, 0}},
    {0xff},
};

ADDRESS(0x8001fde4, 0x518)
void render_frame(const VECTOR *position, const SVECTOR *rotation)
{
    MATRIX model;
    SVECTOR spin;
    u8 *status4;
    u8 *tint;
    KfNotificationSprite *record;
    s16 i;

    render_set_view_transform(position, rotation);
    display_begin_frame();
    pool_mark_allocated();
    SetGeomScreen(0xc8);
    render_map_cells();
    SetLightMatrix(&render_light_matrices[4]);
    status4 = &DAT_80055c94;
    *status4 = 0;
    DAT_80055ca2 = 0;
    DAT_80055cb0 = 0;
    DAT_80055cbe = 0;
    if (player_state.unknown_98[0] == 1) {
        u16 flags;
        DAT_80055c5c = 1;
        DAT_80055c6a = 1;
        DAT_80055c78 = 1;
        DAT_80055c86 = 1;
        DAT_80055ccc = 1;
        DAT_80055cda = 1;
        DAT_80055ce8 = 1;
        DAT_80055cf6 = 1;
        DAT_80055c66 = (player_state.vitals.current_hp * 50
                        + (player_state.vitals.maximum_hp - 1) / 50)
                       / player_state.vitals.maximum_hp;
        DAT_80055c74 = (player_state.vitals.current_mp * 50
                        + (player_state.vitals.maximum_mp - 1) / 50)
                       / player_state.vitals.maximum_mp;
        DAT_80055c82 = player_state.attack_charge_state.current / 100;
        DAT_80055c90 = player_state.magic_charge / 100;
        flags = player_state.status_effect_flags;
        if (flags & 1) {
            DAT_80055cbe = 1;
        } else if (flags & 2) {
            DAT_80055cb0 = 1;
        } else if (flags & 4) {
            *status4 = 1;
        } else if (flags & 8) {
            DAT_80055ca2 = 1;
        }
    } else {
        DAT_80055c5c = 0;
        DAT_80055c6a = 0;
        DAT_80055c78 = 0;
        DAT_80055c86 = 0;
        DAT_80055ccc = 0;
        DAT_80055cda = 0;
        DAT_80055ce8 = 0;
        DAT_80055cf6 = 0;
    }

    tint = &DAT_80055d04;
    *tint = player_state.unknown_98[1];
    effect_sprites[0].state = player_state.unknown_98[1];
    effect_sprites[0].rotation.vz = -render_state.view_rotation.vy & 0xfff;
    render_effect_sprites();

    DAT_8009505a = DAT_80095062;
    DAT_80095058 = DAT_80095060;
    DAT_8009505e = DAT_80095064;
    DAT_8009505d = DAT_80095064;
    DAT_8009505c = DAT_80095064;
    render_hud_gauges(tint - 168);

    SetLightMatrix(&render_light_matrices[5]);
    notify_effect_update();

    DAT_8009505c = 0xff;
    DAT_8009505d = 0xff;
    DAT_8009505e = 0xff;
    model.t[0] = 0;
    model.t[1] = 0xa0;
    model.t[2] = 0xc8;
    spin.vz = 0;
    spin.vy = 0;
    spin.vx = notification_effect_angle_x;
    RotMatrix(&spin, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);

    DAT_8009505a = DAT_80095068;
    DAT_80095058 = DAT_80095066;
    record = notification_sprites;
    if (record[0].active == 1) {
        render_enqueue_sprite(&record[0].sprite, 0, 0);
    }
    if (notification_sprites[1].active == 1) {
        render_enqueue_sprite(&record[1].sprite, 0, 0);
    }
    record += 2;
    DAT_8009505a = DAT_8009506c;
    DAT_80095058 = DAT_8009506a;
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
