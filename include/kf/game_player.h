#ifndef KF_GAME_PLAYER_H
#define KF_GAME_PLAYER_H

/* Player state, shared player data, and player operations. */

#include <kf/audio.h>
#include <kf/enum.h>
#include <kf/game_equipment.h>
#include <kf/game_effect.h>
#include <kf/game_map.h>
#include <kf/magic.h>

struct KfPoolRecord;
struct KfAssetHeader;

enum {
    KF_PLAYER_SOUND_WEAPON_ATTACK = 0,
    KF_PLAYER_SOUND_DEATH = 1,
    KF_PLAYER_SOUND_LEVEL_UP = 2,
    KF_PLAYER_SOUND_COUNT = 3
};

/* Saved bytes and signed-word configuration copies share these option values. */
KF_ENUM_BEGIN(KfPlayerOption, s32)
    KF_PLAYER_OPTION_OFF = 0,
    KF_PLAYER_OPTION_ON = 1
KF_ENUM_END(KfPlayerOption)

KF_ENUM_BEGIN(KfPlayerVerticalState, u8)
    KF_PLAYER_VERTICAL_GROUNDED = 0,
    KF_PLAYER_VERTICAL_FALLING = 0x10,
    KF_PLAYER_VERTICAL_STEP_UP = 0x20
KF_ENUM_END(KfPlayerVerticalState)

/* Downward-positive world Y; view bob is added independently. */
enum {
    KF_PLAYER_CAMERA_HEIGHT = 1500
};

/* Charge counters share a full value; weapon phase has a separate time base. */
enum {
    KF_PLAYER_CHARGE_FULL = 5000,
    KF_PLAYER_CHARGE_GAIN_MULTIPLIER = 2,
    KF_WEAPON_ATTACK_INACTIVE = -1,
    KF_WEAPON_ATTACK_PHASE_STEP = 300,
    KF_WEAPON_ATTACK_PHASE_END = 4096
};

KF_ENUM_BEGIN(KfWarpShimmerMode, s32)
    KF_WARP_SHIMMER_GROW_REMOVE = 0,
    KF_WARP_SHIMMER_SHRINK_REMOVE = 1,
    KF_WARP_SHIMMER_GROW_KEEP = 2
KF_ENUM_END(KfWarpShimmerMode)

enum {
    KF_FLOOR5_ENTRY_VARIANT = 1,
    KF_FLOOR5_ALTERNATE_MUSIC_VARIANT = 3,
    KF_PLAYER_FLOOR_ENTRY_COUNT = 5
};

enum {
    KF_PLAYER_UPDATE_NORMAL = 0,
    KF_PLAYER_DAMAGE_FRAME_FIRST = 1,
    KF_PLAYER_DAMAGE_FRAME_END = 8,
    KF_PLAYER_UPDATE_RECOVERY_FADE = 0xfe,
    KF_PLAYER_UPDATE_DYING = 0xff
};

/* Status timers count executions of their update blocks; -1 is inactive. */
enum {
    KF_PLAYER_STATUS_CURSE = 1 << 0,
    KF_PLAYER_STATUS_DARKNESS = 1 << 1,
    KF_PLAYER_STATUS_POISON = 1 << 2,
    KF_PLAYER_STATUS_SLOWED = 1 << 3,
    KF_PLAYER_STATUS_FIRE_DEFENSE_BOOST = 1 << 4,
    KF_PLAYER_STATUS_TIMER_INACTIVE = -1,
    KF_CURSE_DURATION_UPDATES = 600,
    KF_DARKNESS_DURATION_UPDATES = 1000,
    KF_DARKNESS_REAPPLY_TIMER = 970,
    KF_POISON_DURATION_UPDATES = 600,
    KF_SLOWED_DURATION_UPDATES = 300,
    KF_FIRE_DEFENSE_DURATION_UPDATES = 500
};

enum {
    KF_ILLUSION_STAFF_INACTIVE = -1
};

/* Runtime progression limits; only the first forty resource rows are loaded. */
enum {
    KF_PLAYER_POWER_MAX = 999,
    KF_PLAYER_VITAL_MAX = 9999,
    KF_PLAYER_EXPERIENCE_MAX = 99999,
    KF_PLAYER_LEVEL_MAX = 255,
    KF_PLAYER_LEVEL_GROWTH_COUNT = 40,
    KF_PLAYER_TRAINING_POINTS_PER_GAIN = 100
};

/* Final player-damage multiplier is expressed in tenths. */
enum {
    KF_PLAYER_DAMAGE_MULTIPLIER_ONE = 10
};

typedef struct KfPlayerProgressState {
    u8 level;
    u8 unknown_01;
    KF_ENUM_STORAGE(KfFloorId, u8) current_floor;
    KF_ENUM_STORAGE(KfFloorId, u8) highest_floor;
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
 * The clear routine resets these five halfwords while retaining the cell.
 */
typedef struct KfPlayerMotionFields {
    s16 strafe_velocity;
    s16 forward_velocity;
    u16 movement_speed;
    s16 yaw_step;
    s16 pitch_step;
    KfMapCell map_cell;
} KfPlayerMotionFields;

/* The third aligned word spans pitch and the current map-cell halfword. */
typedef union KfPlayerMotionState {
    KfPlayerMotionFields fields;
    u32 words[3];
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
    u8 allow_near_actor_spawn;
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
    u16 cutting_attack;
    u16 striking_attack;
    u16 piercing_attack;
    u16 holy_attack;
    u16 fire_attack;
    u8 unknown_3a[2];
    u16 cutting_defense;
    u16 striking_defense;
    u16 piercing_defense;
    u16 poison_resistance;
    u16 magic_defense;
    u16 fire_defense;
    s16 curse_timer;
    s16 darkness_timer;
    s16 poison_timer;
    s16 slowed_timer;
    s16 fire_defense_timer;
    s16 illusion_staff_timer;
    u8 unknown_54[4];
    u32 equipment_effect_ticks;
    KfSelectedMagicId selected_magic_id;
    u8 unknown_5d[3];
    KfMagicRecord *selected_magic_record;
    KfItemId equipped_weapon_id;
    u8 unknown_65[3];
    KfWeaponRecord *equipped_weapon_record;
    struct KfAssetHeader *weapon_asset_buffer;
    s16 weapon_attack_phase;
    u8 unknown_72[2];
    struct KfPoolRecord *weapon_animation_cache;
    u8 weapon_magic_shots_remaining;
    u8 weapon_magic_delay;
    u8 weapon_attack_fully_charged;
    u8 unknown_7b[1];
    KfArmorRecord *equipped_head_armor_record;
    KfArmorRecord *equipped_body_armor_record;
    KfArmorRecord *equipped_shield_record;
    KfArmorRecord *equipped_arm_armor_record;
    KfArmorRecord *equipped_leg_armor_record;
    KfItemId equipped_head_armor_id;
    KfItemId equipped_body_armor_id;
    KfItemId equipped_shield_id;
    KfItemId equipped_arm_armor_id;
    KfItemId equipped_leg_armor_id;
    KfItemId equipped_accessory_id;
    KF_ENUM_STORAGE(KfPlayerOption, u8) audio_effects_enabled;
    KF_ENUM_STORAGE(KfPlayerOption, u8) audio_music_enabled;
    KF_ENUM_STORAGE(KfPlayerOption, u8) hud_gauges_enabled;
    KF_ENUM_STORAGE(KfPlayerOption, u8) compass_enabled;
    SVECTOR view_rotation_offset;
    u8 update_state;
    u8 unknown_a3;
    VECTOR camera_position;
    s32 floor_height;
    SVECTOR camera_rotation;
    KfPlayerMotionState motion_state;
    KfMapCell previous_map_cell;
    u8 unknown_ce[6];
    s16 view_bob_offset;
    u16 view_bob_phase;
    u16 death_camera_pitch_step;
    s16 death_visual_blend;
    s16 vertical_velocity;
    KfPlayerVerticalState vertical_state;
    u8 unknown_df[1];
} KfPlayerState;

typedef char check_player_state_size[sizeof(KfPlayerState) == 0xe0 ? 1 : -1];
typedef char check_player_motion_size[sizeof(KfPlayerMotionState) == 0x0c ? 1 : -1];
#define KF_PLAYER_MOTION_OFFSET_CHECK(label, member, offset) \
    typedef char check_player_motion_##label[ \
        ((unsigned long)&((KfPlayerState *)0)->member == (offset)) ? 1 : -1]
KF_PLAYER_MOTION_OFFSET_CHECK(word, motion_state.words[2], 0xc8);
KF_PLAYER_MOTION_OFFSET_CHECK(pitch, motion_state.fields.pitch_step, 0xc8);
KF_PLAYER_MOTION_OFFSET_CHECK(cell_z, motion_state.fields.map_cell.coords.z, 0xca);
KF_PLAYER_MOTION_OFFSET_CHECK(cell_x, motion_state.fields.map_cell.coords.x, 0xcb);
KF_PLAYER_MOTION_OFFSET_CHECK(previous_cell, previous_map_cell.word, 0xcc);
#undef KF_PLAYER_MOTION_OFFSET_CHECK

/*
 * Per-floor entry cell (one-based floor number). player_warp_to_floor_entry
 * copies byte 0 into previous_map_cell.coords.x and byte 1 into .coords.z, so this table
 * stores x first, unlike KfMapCell.
 */
typedef struct KfFloorEntryCell {
    u8 x;
    u8 z;
} KfFloorEntryCell;

extern SoundRef player_sound_refs[KF_PLAYER_SOUND_COUNT];
extern KfFloorEntryCell floor_entry_cells[KF_PLAYER_FLOOR_ENTRY_COUNT];
extern MATRIX player_death_saved_color_matrix;
extern s32 player_death_saved_fog_near;
extern KfPlayerLevelGrowth player_level_growth_table[KF_PLAYER_LEVEL_GROWTH_COUNT];
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
extern void player_apply_radial_damage(
    const VECTOR *origin, u32 radius, u16 falloff_q12, u16 base_power,
    u16 component0, u16 component1, u16 component2, u16 component3, u16 component4,
    u16 scale_q12, u16 multiplier_tenths);
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
    const VECTOR *point, s16 facing, s32 max_distance,
    s32 angle_tolerance);
extern void player_equip_weapon(KfItemId weapon_id);
extern void game_initialize_session(void);
extern void game_state_initialize(void);
extern void player_increment_magic_training(void);
extern void player_increment_physical_power_training(void);
extern s32 player_move_horizontal(s32 heading, s32 distance);
extern void player_recalculate_combat_stats(void);
extern void player_restore_vitals_with_color_cycle(void);
extern void player_select_magic(KfSelectedMagicId magic_id);
extern void player_set_equipment_slot(KfItemId item_id, KfEquipmentSlot slot);
extern void player_apply_fire_defense_boost(void);
extern void player_sync_position_to_map(void);
extern void player_use_item(KfItemId item_id);
extern void player_update(void);
extern void player_update_transform_snapshot(
    VECTOR *position_out, SVECTOR *rotation_out);
extern void player_update_vertical_motion(void);
extern void player_update_view_bob(void);
extern void player_update_weapon_attack(void);
/* Warp variant inputs remain full words until the player-state byte store. */
extern void player_warp_change_floor(KfFloorId floor, u32 variant);
extern void player_warp_shimmer(KfWarpShimmerMode mode, VECTOR *position);
extern void player_warp_shimmer_at_player(KF_ENUM_PARAM(KfWarpShimmerMode, u32) mode);
extern void player_warp_same_floor(u32 variant, s32 cell_x, s32 cell_z);
extern void player_warp_to_floor_entry(void);
extern u32 player_warp_trigger_update(void);

#endif
