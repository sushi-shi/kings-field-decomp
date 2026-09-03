#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/* Psy-Q LIBGTE: void ReadColorMatrix(MATRIX *m). */

extern SoundRef player_sound_refs[3];

extern u32 DAT_8009ddb4;
extern u8 DAT_800652a8[240];

ADDRESS(0x80015164, 0x68)
void player_death_begin(void)
{
    player_state.update_state = 0xff;
    player_state.death_camera_pitch_step = 0;
    player_state.death_visual_blend = 0;
    sound_ref_play(&player_sound_refs[1], 0x7f);
    ReadColorMatrix(&player_death_saved_color_matrix);
    player_death_saved_fog_near = render_state.fog_near_distance;
}

/*
 * The two byte loops clear whole BSS runs that start at the named objects
 * (0x2134 bytes of map-event state, 0xf0 bytes of progress flags) before the
 * flags that start set are raised again.
 */
ADDRESS(0x800151cc, 0x2e4)
void game_state_initialize(void)
{
    u8 *cursor;
    s32 count;

    player_state.experience = 0;
    player_state.progress_state.level = 1;
    player_state.progress_state.current_floor = 1;
    player_state.progress_state.highest_floor = 1;
    player_state.unknown_2c = 0x96;
    player_state.attack_charge_state.current = 0;
    player_state.magic_charge = 0;
    player_state.weapon_charge_delay = 0;
    player_state.damage_defense_component0 = 5;
    player_state.damage_defense_component1 = 5;
    player_state.damage_defense_component2 = 5;
    player_state.status_effect2_resistance = 5;
    player_state.damage_defense_component3 = 5;
    player_state.damage_defense_component4 = 5;
    player_state.view_bob_offset = 0;
    player_state.view_bob_phase = 0;
    player_state.vertical_state = 0;
    player_state.vertical_velocity = 0;
    player_state.vitals.current_hp = player_level_growth_table[0].maximum_hp;
    player_state.vitals.maximum_hp = player_level_growth_table[0].maximum_hp;
    player_state.vitals.current_mp = player_level_growth_table[0].maximum_mp;
    player_state.vitals.maximum_mp = player_level_growth_table[0].maximum_mp;
    player_state.base_physical_power = player_level_growth_table[0].physical_power_step;
    player_state.base_magic = player_level_growth_table[0].magic_step;
    player_state.next_level_experience = player_level_growth_table[0].experience_threshold;
    player_equip_weapon(0);
    player_state.equipped_shield_id = 0xff;
    player_state.equipped_head_armor_id = 0xff;
    player_state.equipped_arm_armor_id = 0xff;
    player_state.equipped_leg_armor_id = 0xff;
    player_state.equipped_body_armor_id = 0xff;
    player_state.equipped_accessory_id = 0xff;
    player_set_equipment_slot(0, 0xff);
    player_select_magic(8);
    player_state.status_effect4_timer = -1;
    player_state.light_effect_timer = -1;
    player_state.status_effect3_timer = -1;
    player_state.status_effect2_timer = -1;
    player_state.status_effect1_timer = -1;
    player_state.status_effect0_timer = -1;
    cursor = (u8 *)&DAT_8009ddb4;
    count = 0x2133;
    do {
        *cursor++ = 0;
    } while (--count != -1);
    cursor = DAT_800652a8;
    count = 0xef;
    do {
        *cursor++ = 0;
    } while (--count != -1);
    DAT_800652a8[0x00] = 1;
    DAT_800652a8[0x2b] = 1;
    DAT_800652a8[0x50] = 1;
    DAT_800652a8[0x51] = 1;
    DAT_800652a8[0x52] = 1;
    DAT_800652a8[0x5d] = 1;
    DAT_800652a8[0x5e] = 1;
    DAT_800652a8[0x64] = 1;
    DAT_800652a8[0x6a] = 1;
    DAT_800652a8[0x6b] = 1;
    DAT_800652a8[0x70] = 1;
    DAT_800652a8[0x73] = 1;
    DAT_800652a8[0x7b] = 1;
    DAT_800652a8[0x7c] = 1;
    DAT_800652a8[0x7d] = 1;
    DAT_800652a8[0x7f] = 1;
    DAT_800652a8[0xa2] = 1;
    DAT_800652a8[0xa3] = 1;
    DAT_800652a8[0xa6] = 1;
    DAT_800652a8[0xae] = 1;
    DAT_800652a8[0xaf] = 1;
    DAT_800652a8[0xb3] = 1;
    DAT_800652a8[0xb5] = 1;
    DAT_800652a8[0xb6] = 1;
    DAT_800652a8[0xbc] = 1;
    DAT_800652a8[0xc5] = 1;
    DAT_800652a8[0xcb] = 1;
    DAT_800652a8[0xcc] = 1;
    DAT_800652a8[0xcd] = 1;
    DAT_800652a8[0xce] = 1;
    DAT_800652a8[0xcf] = 1;
    DAT_800652a8[0xd0] = 1;
    DAT_800652a8[0xd4] = 1;
}

ADDRESS(0x800154b0, 0x19c)
void player_death_restart(void)
{
    s32 floor = player_state.progress_state.current_floor;

    if (((u8 *)&DAT_8009ddb4)[3] == 1 && DAT_800652a8[0x2f] != 0) {
        DAT_800652a8[0x2f]--;
        func_80035b5c();
        player_state.camera_position.vx = 0xfa00;
        player_state.vitals.current_hp = player_state.vitals.maximum_hp;
        player_state.vitals.current_mp = player_state.vitals.maximum_mp;
        player_state.camera_position.vz = 0x4e20;
        player_state.camera_rotation.vy = 0;
    } else {
        player_state.camera_position.vx = 0x7918;
        player_state.camera_position.vz = 0x1388;
        player_state.camera_rotation.vy = 0;
        game_state_initialize();
        floor = 0xff;
    }
    player_state.status_effect_flags = 0;
    player_state.camera_rotation.vz = 0;
    player_state.camera_rotation.vx = 0;
    if (floor != 1) {
        player_state.progress_state.current_floor = 1;
        player_state.map_variant = 0;
        pool_release_all();
        audio_close_vab();
        func_800365f8();
    }
    player_sync_position_to_map();
    player_state.view_bob_offset = 0;
    player_state.update_state = 0xfe;
    player_state.death_camera_pitch_step = 0;
    player_state.death_visual_blend = 0;
    DAT_80095064 = 0;
    player_state.view_rotation_offset.vz = 0;
    player_state.view_rotation_offset.vy = 0;
    player_state.view_rotation_offset.vx = 0;
    player_state.previous_map_cell.x = player_state.map_cell.x;
    player_state.previous_map_cell.z = player_state.map_cell.z;
    player_state.camera_position.vy = player_state.floor_height - 1500;
}
