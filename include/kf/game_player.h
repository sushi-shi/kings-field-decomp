#ifndef KF_GAME_PLAYER_H
#define KF_GAME_PLAYER_H

/* Player state, shared player data, and player operations. */

#include <kf/audio.h>
#include <kf/game_equipment.h>
#include <kf/game_map.h>
#include <kf/magic.h>

typedef struct KfPlayerProgressState {
    u8 level;
    u8 unknown_01;
    u8 current_floor;
    u8 highest_floor;
} KfPlayerProgressState;

/*
 * Record zero seeds a new player's values. Later records provide the next
 * absolute HP/MP values, stat steps, and cumulative experience threshold.
 * After level 40 the game extrapolates from the final two records.
 */
typedef struct KfPlayerLevelGrowth {
    u16 maximum_hp;
    u16 maximum_mp;
    u16 physical_power_step;
    u16 magic_step;
    u32 experience_threshold;
} KfPlayerLevelGrowth;

typedef struct KfPlayerVitals {
    u16 maximum_hp;
    u16 current_hp;
    u16 maximum_mp;
    u16 current_mp;
} KfPlayerVitals;

typedef struct KfPlayerAttackChargeState {
    u16 current;
    u16 committed;
} KfPlayerAttackChargeState;

/*
 * The player input loop ramps the two signed movement components, derives the
 * unsigned horizontal speed, and independently ramps the yaw and pitch steps.
 * The grouped clear routine proves the complete five-halfword extent.
 */
typedef struct KfPlayerMotionState {
    s16 strafe_velocity;
    s16 forward_velocity;
    u16 movement_speed;
    s16 yaw_step;
    s16 pitch_step;
} KfPlayerMotionState;

/*
 * The saved player object. save_file_write_slot copies exactly 0xe0 bytes
 * from 0x800a0780 into the save payload with the compiler's inline struct
 * loop, and player routines address these members from a registered base;
 * both prove one object rather than adjacent globals. Unknown spans remain
 * opaque.
 */
typedef struct KfPlayerState {
    s32 experience;
    s32 next_level_experience;
    KfPlayerProgressState progress_state;
    u8 map_variant;
    u8 unknown_0d;
    u8 weapon_charge_delay;
    u8 unknown_0f;
    KfPlayerVitals vitals;
    KfPlayerAttackChargeState attack_charge_state;
    u16 magic_charge;
    u16 physical_power_training;
    u16 magic_training;
    u16 base_physical_power;
    u16 base_magic;
    u16 physical_power;
    u16 magic;
    u16 status_effect_flags;
    u32 gold;
    u16 attack_component0;
    u16 attack_component1;
    u16 attack_component2;
    u16 attack_component3;
    u16 attack_component4;
    u8 unknown_3a[2];
    u16 damage_defense_component0;
    u16 damage_defense_component1;
    u16 damage_defense_component2;
    u16 status_effect2_resistance;
    u16 damage_defense_component3;
    u16 damage_defense_component4;
    s16 status_effect0_timer;
    s16 status_effect1_timer;
    s16 status_effect2_timer;
    s16 status_effect3_timer;
    s16 status_effect4_timer;
    s16 light_effect_timer;
    u8 unknown_54[4];
    u32 unknown_58;
    u8 selected_magic_id;
    u8 unknown_5d[3];
    KfMagicRecord *selected_magic_record;
    u8 equipped_weapon_id;
    u8 unknown_65[3];
    const KfWeaponRecord *equipped_weapon_record;
    u8 *weapon_asset_buffer;
    s16 weapon_attack_phase;
    u8 unknown_72[2];
    u32 unknown_74;
    u8 unknown_78;
    u8 unknown_79;
    u8 weapon_attack_fully_charged;
    u8 unknown_7b[1];
    KfArmorRecord *equipped_shield_record;
    KfArmorRecord *equipped_head_armor_record;
    KfArmorRecord *equipped_body_armor_record;
    KfArmorRecord *equipped_arm_armor_record;
    KfArmorRecord *equipped_leg_armor_record;
    u8 equipped_shield_id;
    u8 equipped_head_armor_id;
    u8 equipped_body_armor_id;
    u8 equipped_arm_armor_id;
    u8 equipped_leg_armor_id;
    u8 equipped_accessory_id;
    u8 audio_effects_enabled;
    u8 audio_music_enabled;
    u8 unknown_98[2];
    SVECTOR view_rotation_offset;
    u8 update_state;
    u8 unknown_a3;
    VECTOR camera_position;
    s32 floor_height;
    SVECTOR camera_rotation;
    KfPlayerMotionState motion_state;
    KfMapCell map_cell;
    KfMapCell previous_map_cell;
    u8 unknown_ce[6];
    s16 view_bob_offset;
    u16 view_bob_phase;
    u16 death_camera_pitch_step;
    s16 death_visual_blend;
    s16 vertical_velocity;
    u8 vertical_state;
    u8 unknown_df[1];
} KfPlayerState;

/*
 * Per-floor entry cell (one-based floor number). player_warp_to_floor_entry
 * copies byte 0 into previous_map_cell.x and byte 1 into .z, so this table
 * stores x first, unlike KfMapCell.
 */
typedef struct KfFloorEntryCell {
    u8 x;
    u8 z;
} KfFloorEntryCell;

extern KfMagicRecord magic_records[24];
extern const SoundRef player_sound_refs[3];
extern const KfFloorEntryCell floor_entry_cells[5];
extern MATRIX player_death_saved_color_matrix;
extern s32 player_death_saved_fog_near;
extern KfPlayerLevelGrowth player_level_growth_table[40];
extern VECTOR player_position_snapshot;
extern SVECTOR player_rotation_snapshot;
extern KfPlayerState player_state;

extern void player_add_experience(s16 amount);
extern void player_adjust_hp(s32 delta);
extern void player_adjust_mp(s32 delta);
extern void player_apply_damage(
    u16 component0, u16 component1, u16 component2,
    u16 status_effect_flags, u16 component3, u16 component4,
    u16 scale_q12, u16 multiplier_tenths);
extern void player_begin_weapon_attack(void);
extern s32 player_calculate_damage_component(s32 base_power, s32 defense, s32 attack);
extern void player_clear_motion(void);
extern void player_death_apply_visual_fade(const MATRIX *color_from, s32 blend);
extern void player_death_begin(void);
extern void player_death_restart(void);
extern void player_death_update(void);
extern void player_death_update_reverse_fade(void);
extern s32 player_distance_to_point(
    s32 point_x, s32 point_y, s32 point_z, s32 max_distance,
    s32 point_height);
extern s32 player_distance_to_point_in_cone(
    const struct KfVec3i *point, s16 facing, s32 max_distance,
    s32 angle_tolerance);
extern void player_equip_weapon(u8 weapon_id);
extern void player_increment_magic_training(void);
extern void player_increment_physical_power_training(void);
extern s32 player_move_horizontal(s32 heading, s32 distance);
extern void player_recalculate_combat_stats(void);
extern void player_restore_vitals_with_color_cycle(void);
extern void player_select_magic(u8 magic_id);
extern void player_set_equipment_slot(u8 item_id, u8 slot);
extern void player_status_apply_effect4(void);
extern void player_sync_position_to_map(void);
extern void player_use_item(u8 item_id);
extern void player_update(void);
extern void player_update_transform_snapshot(
    VECTOR *position_out, SVECTOR *rotation_out);
extern void player_update_vertical_motion(void);
extern void player_update_view_bob(void);
extern void player_update_weapon_attack(void);
extern void player_warp_change_floor(s32 floor, u8 variant);
extern void player_warp_shimmer(s32 mode, VECTOR *position);
extern void player_warp_shimmer_at_player(u32 mode);
extern void player_warp_same_floor(char variant, s32 cell_x, s32 cell_z);
extern void player_warp_to_floor_entry(void);
extern u32 player_warp_trigger_update(void);

#endif
