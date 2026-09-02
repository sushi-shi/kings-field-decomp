#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/semantic_types.h>

/*
 * Per-frame renderer entry, called by the player warp/update path.  It rebuilds
 * the view transform, opens the frame, refreshes the HUD gauge table, draws the
 * screen-space effect and notification sprites, sweeps the entity pools through
 * the scene dispatcher (render_entities), draws the held weapon, and presents.
 *
 * WIP: the HUD gauge records at DAT_80055c5c (stride 14, walked by
 * func_8001f9d4) and the notification descriptor globals are unresolved and
 * reached by their individual identities.
 */

extern KfRenderState render_state;
extern KfPlayerState player_state;
extern MATRIX render_light_matrices[6];

/* HUD gauge record table (12 records, 14-byte stride, walked by func_8001f9d4). */
extern u8 DAT_80055c5c;
extern u16 DAT_80055c66;
extern u8 DAT_80055c6a;
extern u16 DAT_80055c74;
extern u8 DAT_80055c78;
extern u16 DAT_80055c82;
extern u8 DAT_80055c86;
extern u16 DAT_80055c90;
extern u8 DAT_80055c94;
extern u8 DAT_80055ca2;
extern u8 DAT_80055cb0;
extern u8 DAT_80055cbe;
extern u8 DAT_80055ccc;
extern u8 DAT_80055cda;
extern u8 DAT_80055ce8;
extern u8 DAT_80055cf6;

extern u8 DAT_80055d04;   /* effect sprite tint anchor */
extern u8 DAT_80055d74;
extern u16 DAT_80055d86;
extern u8 DAT_80055d20[];  /* six notification sprite records */
extern u8 DAT_80055d2e;

extern u16 DAT_80095058;
extern u16 DAT_8009505a;
extern u8 DAT_8009505c;
extern u8 DAT_8009505d;
extern u8 DAT_8009505e;
extern u16 DAT_80095060;
extern u16 DAT_80095062;
extern u8 DAT_80095064;
extern u16 DAT_80095066;
extern u16 DAT_80095068;
extern u16 DAT_8009506a;
extern u16 DAT_8009506c;
extern u16 DAT_8009508a;

extern void render_set_view_transform(const VECTOR *position, const SVECTOR *rotation);
extern void display_begin_frame(void);
extern void pool_mark_allocated(void);
extern void func_8001e83c(void);
extern void func_8001f8b0(void);
extern void func_8001f9d4(u8 *table);
extern void func_8001fafc(void);
extern void func_8001e230(char *descriptor, s16 screen_scale, s32 flag);
extern void render_entities(void);
extern void func_8001f798(void);
extern void display_present_frame(void);
extern void func_80020a98(void);

ADDRESS(0x8001fde4, 0x518)
void func_8001fde4(VECTOR *position, SVECTOR *rotation)
{
    MATRIX model;
    SVECTOR spin;
    u8 *status4;
    u8 *tint;
    u8 *record;
    s16 i;

    render_set_view_transform(position, rotation);
    display_begin_frame();
    pool_mark_allocated();
    SetGeomScreen(0xc8);
    func_8001e83c();
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
    DAT_80055d74 = player_state.unknown_98[1];
    DAT_80055d86 = -render_state.view_rotation.vy & 0xfff;
    func_8001f8b0();

    DAT_8009505a = DAT_80095062;
    DAT_80095058 = DAT_80095060;
    DAT_8009505e = DAT_80095064;
    DAT_8009505d = DAT_80095064;
    DAT_8009505c = DAT_80095064;
    func_8001f9d4(tint - 168);

    SetLightMatrix(&render_light_matrices[5]);
    func_8001fafc();

    DAT_8009505c = 0xff;
    DAT_8009505d = 0xff;
    DAT_8009505e = 0xff;
    model.t[0] = 0;
    model.t[1] = 0xa0;
    model.t[2] = 0xc8;
    spin.vz = 0;
    spin.vy = 0;
    spin.vx = DAT_8009508a;
    RotMatrix(&spin, &model);
    SetRotMatrix(&model);
    SetTransMatrix(&model);

    DAT_8009505a = DAT_80095068;
    DAT_80095058 = DAT_80095066;
    record = DAT_80055d20;
    if (record[0] == 1) {
        func_8001e230((char *)(record + 2), 0, 0);
    }
    if (DAT_80055d2e == 1) {
        func_8001e230((char *)(record + 16), 0, 0);
    }
    record += 28;
    DAT_8009505a = DAT_8009506c;
    DAT_80095058 = DAT_8009506a;
    for (i = 3; i != -1; i--) {
        if (record[0] == 1) {
            func_8001e230((char *)(record + 2), 0, 0);
        }
        record += 14;
    }

    render_entities();
    func_8001f798();
    display_present_frame();
    func_80020a98();
}
