#include <kf/address.h>
#include <kf/semantic_types.h>

/* Psy-Q LIBGTE: void ReadColorMatrix(MATRIX *m). */
extern void ReadColorMatrix(struct KfMatrix *matrix);
extern void sound_ref_play(const SoundRef *sound, s16 volume);
extern void player_equip_weapon(u8 weapon_id);
extern void player_set_equipment_slot(u8 item_id, u8 slot);
extern void player_select_magic(u8 magic_id);
extern void func_80035b5c(void);
extern void func_80020a2c(void);
extern void audio_close_vab(void);
extern void func_800365f8(void);
extern void player_sync_position_to_map(void);
extern void game_state_initialize(void);

extern u8 player_update_state;
extern u16 player_death_camera_pitch_step;
extern s16 player_death_visual_blend;
extern SoundRef player_sound_refs[3];
extern struct KfMatrix player_death_saved_color_matrix;
extern s32 fog_near_distance;
extern s32 player_death_saved_fog_near;

extern u32 player_experience;
extern u32 player_next_level_experience;
extern KfPlayerProgressState player_progress_state;
extern u8 DAT_800a078c;
extern u32 DAT_800a07ac;
extern KfPlayerAttackChargeState player_attack_charge_state;
extern u16 player_magic_charge;
extern u8 player_weapon_charge_delay;
extern u16 player_damage_defense_component0;
extern u16 player_damage_defense_component1;
extern u16 player_damage_defense_component2;
extern u16 player_status_effect2_resistance;
extern u16 player_damage_defense_component3;
extern u16 player_damage_defense_component4;
extern s16 player_view_bob_offset;
extern u16 player_view_bob_phase;
extern u8 player_vertical_state;
extern s16 player_vertical_velocity;
extern KfPlayerLevelGrowth player_level_growth_table[40];
extern KfPlayerVitals player_vitals;
extern u16 player_base_physical_power;
extern u16 player_base_magic;
extern u8 player_equipped_shield_id;
extern u8 player_equipped_head_armor_id;
extern u8 player_equipped_body_armor_id;
extern u8 player_equipped_arm_armor_id;
extern u8 player_equipped_leg_armor_id;
extern u8 player_equipped_accessory_id;
extern s16 DAT_800a07d0;
extern s16 DAT_800a07d2;
extern s16 player_status_effect0_timer;
extern s16 player_status_effect1_timer;
extern s16 player_status_effect2_timer;
extern s16 player_status_effect3_timer;
extern u16 player_status_effect_flags;
extern u32 DAT_8009ddb4;
extern u8 DAT_800652a8[240];
extern struct KfVec4i camera_position;
extern struct KfVec4s camera_rotation;
extern KfMapCell player_map_cell;
extern KfMapCell player_previous_map_cell;
extern s32 player_floor_height;
extern u8 DAT_80095064;
extern struct KfVec4s player_view_rotation_offset;

ADDRESS(0x80015164, 0x68)
void player_death_begin(void)
{
    player_update_state = 0xff;
    player_death_camera_pitch_step = 0;
    player_death_visual_blend = 0;
    sound_ref_play(&player_sound_refs[1], 0x7f);
    ReadColorMatrix(&player_death_saved_color_matrix);
    player_death_saved_fog_near = fog_near_distance;
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

    player_experience = 0;
    player_progress_state.level = 1;
    player_progress_state.current_floor = 1;
    player_progress_state.highest_floor = 1;
    DAT_800a07ac = 0x96;
    player_attack_charge_state.current = 0;
    player_magic_charge = 0;
    player_weapon_charge_delay = 0;
    player_damage_defense_component0 = 5;
    player_damage_defense_component1 = 5;
    player_damage_defense_component2 = 5;
    player_status_effect2_resistance = 5;
    player_damage_defense_component3 = 5;
    player_damage_defense_component4 = 5;
    player_view_bob_offset = 0;
    player_view_bob_phase = 0;
    player_vertical_state = 0;
    player_vertical_velocity = 0;
    player_vitals.current_hp = player_level_growth_table[0].maximum_hp;
    player_vitals.maximum_hp = player_level_growth_table[0].maximum_hp;
    player_vitals.current_mp = player_level_growth_table[0].maximum_mp;
    player_vitals.maximum_mp = player_level_growth_table[0].maximum_mp;
    player_base_physical_power = player_level_growth_table[0].physical_power_step;
    player_base_magic = player_level_growth_table[0].magic_step;
    player_next_level_experience = player_level_growth_table[0].experience_threshold;
    player_equip_weapon(0);
    player_equipped_shield_id = 0xff;
    player_equipped_head_armor_id = 0xff;
    player_equipped_arm_armor_id = 0xff;
    player_equipped_leg_armor_id = 0xff;
    player_equipped_body_armor_id = 0xff;
    player_equipped_accessory_id = 0xff;
    player_set_equipment_slot(0, 0xff);
    player_select_magic(8);
    DAT_800a07d0 = -1;
    DAT_800a07d2 = -1;
    player_status_effect3_timer = -1;
    player_status_effect2_timer = -1;
    player_status_effect1_timer = -1;
    player_status_effect0_timer = -1;
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
    s32 floor = player_progress_state.current_floor;

    if (((u8 *)&DAT_8009ddb4)[3] == 1 && DAT_800652a8[0x2f] != 0) {
        DAT_800652a8[0x2f]--;
        func_80035b5c();
        camera_position.x = 0xfa00;
        player_vitals.current_hp = player_vitals.maximum_hp;
        player_vitals.current_mp = player_vitals.maximum_mp;
        camera_position.z = 0x4e20;
        camera_rotation.y = 0;
    } else {
        camera_position.x = 0x7918;
        camera_position.z = 0x1388;
        camera_rotation.y = 0;
        game_state_initialize();
        floor = 0xff;
    }
    player_status_effect_flags = 0;
    camera_rotation.z = 0;
    camera_rotation.x = 0;
    if (floor != 1) {
        player_progress_state.current_floor = 1;
        DAT_800a078c = 0;
        func_80020a2c();
        audio_close_vab();
        func_800365f8();
    }
    player_sync_position_to_map();
    player_view_bob_offset = 0;
    player_update_state = 0xfe;
    player_death_camera_pitch_step = 0;
    player_death_visual_blend = 0;
    DAT_80095064 = 0;
    player_view_rotation_offset.z = 0;
    player_view_rotation_offset.y = 0;
    player_view_rotation_offset.x = 0;
    player_previous_map_cell.x = player_map_cell.x;
    player_previous_map_cell.z = player_map_cell.z;
    camera_position.y = player_floor_height - 1500;
}
