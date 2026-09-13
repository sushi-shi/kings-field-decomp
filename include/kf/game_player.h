#ifndef KF_GAME_PLAYER_H
#define KF_GAME_PLAYER_H

/* Player state, shared player data, and player operations. */

#include <kf/bool.h>
#include <kf/combat.h>
#include <kf/audio.h>
#include <kf/enum.h>
#include <kf/game_equipment.h>
#include <kf/game_effect.h>
#include <kf/game_map.h>
#include <kf/magic.h>
#include <kf/player_status.h>

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

/* Normal view clamp, also approached by the container-opening camera. */
enum {
    KF_PLAYER_CAMERA_PITCH_LIMIT = 191
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
    KF_PLAYER_FLOOR_ENTRY_COUNT = 5
};

/* Damage animation advances through frames 1..8 before returning to normal. */
KF_ENUM_BEGIN(KfPlayerUpdateState, u8)
    KF_PLAYER_UPDATE_NORMAL = 0,
    KF_PLAYER_DAMAGE_FRAME_FIRST = 1,
    KF_PLAYER_DAMAGE_FRAME_2 = 2,
    KF_PLAYER_DAMAGE_FRAME_3 = 3,
    KF_PLAYER_DAMAGE_FRAME_4 = 4,
    KF_PLAYER_DAMAGE_FRAME_5 = 5,
    KF_PLAYER_DAMAGE_FRAME_6 = 6,
    KF_PLAYER_DAMAGE_FRAME_7 = 7,
    KF_PLAYER_DAMAGE_FRAME_END = 8,
    KF_PLAYER_UPDATE_RECOVERY_FADE = 0xfe,
    KF_PLAYER_UPDATE_DYING = 0xff
KF_ENUM_END(KfPlayerUpdateState)
KF_ENUM_COUNTER(KfPlayerUpdateState, u8)

KF_ENUM_BEGIN(KfActorSpawnPermission, u8)
    KF_ACTOR_NEAR_SPAWN_FORBIDDEN = 0,
    KF_ACTOR_NEAR_SPAWN_ALLOWED = 1
KF_ENUM_END(KfActorSpawnPermission)

KF_ENUM_BEGIN(KfWeaponAttackCharge, u8)
    KF_WEAPON_ATTACK_NORMAL_CHARGE = 0,
    KF_WEAPON_ATTACK_FULL_CHARGE = 1
KF_ENUM_END(KfWeaponAttackCharge)

/* Status timers count executions of their update blocks; -1 is inactive. */
enum {
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
    KfMapVariant map_variant;
    KfActorSpawnPermission allow_near_actor_spawn;
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
    KfPlayerStatusFlags status_effect_flags;
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
    KfEffectKind selected_magic_id;
    u8 unknown_5d[3];
    KfMagicRecord *selected_magic_record;
    KfObjectId equipped_weapon_id;
    u8 unknown_65[3];
    KfWeaponRecord *equipped_weapon_record;
    struct KfAssetHeader *weapon_asset_buffer;
    s16 weapon_attack_phase;
    u8 unknown_72[2];
    struct KfPoolRecord *weapon_animation_cache;
    u8 weapon_magic_shots_remaining;
    u8 weapon_magic_delay;
    KfWeaponAttackCharge weapon_attack_fully_charged;
    u8 unknown_7b[1];
    KfArmorRecord *equipped_head_armor_record;
    KfArmorRecord *equipped_body_armor_record;
    KfArmorRecord *equipped_shield_record;
    KfArmorRecord *equipped_arm_armor_record;
    KfArmorRecord *equipped_leg_armor_record;
    KfObjectId equipped_head_armor_id;
    KfObjectId equipped_body_armor_id;
    KfObjectId equipped_shield_id;
    KfObjectId equipped_arm_armor_id;
    KfObjectId equipped_leg_armor_id;
    KfObjectId equipped_accessory_id;
    KF_ENUM_STORAGE(KfPlayerOption, u8) audio_effects_enabled;
    KF_ENUM_STORAGE(KfPlayerOption, u8) audio_music_enabled;
    KF_ENUM_STORAGE(KfPlayerOption, u8) hud_gauges_enabled;
    KF_ENUM_STORAGE(KfPlayerOption, u8) compass_enabled;
    SVECTOR view_rotation_offset;
    KfPlayerUpdateState update_state;
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

/* Side-effect-free VECTOR lvalue; preserves X/Z/floor-Y publication order. */
#define PLAYER_FLOOR_POSITION(position) ( \
    (position).vx = player_state.camera_position.vx, \
    (position).vz = player_state.camera_position.vz, \
    (position).vy = player_state.floor_height)

/* Inventory table index, evaluated once for each slot until a match.
 * Supply a side-effect-free expression; magic uses a separate ID domain. */
#define PLAYER_ITEM_IS_EQUIPPED(item_index) \
    ((item_index) == KF_ENUM_ENCODE(u8, player_state.equipped_weapon_id) || \
     (item_index) == KF_ENUM_ENCODE(u8, player_state.equipped_head_armor_id) || \
     (item_index) == KF_ENUM_ENCODE(u8, player_state.equipped_body_armor_id) || \
     (item_index) == KF_ENUM_ENCODE(u8, player_state.equipped_shield_id) || \
     (item_index) == KF_ENUM_ENCODE(u8, player_state.equipped_arm_armor_id) || \
     (item_index) == KF_ENUM_ENCODE(u8, player_state.equipped_leg_armor_id) || \
     (item_index) == KF_ENUM_ENCODE(u8, player_state.equipped_accessory_id))

extern void player_add_experience(s16 amount);
extern void player_adjust_hp(s32 delta);
extern void player_adjust_mp(s32 delta);
extern void player_apply_damage(
    u16 component0, u16 component1, u16 component2,
    KfPlayerStatusFlags status_effect_flags, u16 component3, u16 component4,
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
extern void player_equip_weapon(KfObjectId weapon_id);
extern void game_initialize_session(void);
extern void game_state_initialize(void);
extern void player_increment_magic_training(void);
extern void player_increment_physical_power_training(void);
extern s32 player_move_horizontal(s32 heading, s32 distance);
extern void player_recalculate_combat_stats(void);
extern void player_restore_vitals_with_color_cycle(void);
extern void player_select_magic(KfEffectKind magic_id);
extern void player_set_equipment_slot(KfObjectId item_id, KfEquipmentSlot slot);
extern void player_apply_fire_defense_boost(void);
extern void player_sync_position_to_map(void);
extern void player_use_item(KfObjectId item_id);
extern void player_update(void);
extern void player_update_transform_snapshot(
    VECTOR *position_out, SVECTOR *rotation_out);
extern void player_update_vertical_motion(void);
extern void player_update_view_bob(void);
extern void player_update_weapon_attack(void);
/* Warp variant inputs remain full words until the player-state byte store. */
extern void player_warp_change_floor(KfFloorId floor, KF_ENUM_PARAM(KfMapVariant, u32) variant);
extern void player_warp_shimmer(KfWarpShimmerMode mode, VECTOR *position);
extern void player_warp_shimmer_at_player(KF_ENUM_PARAM(KfWarpShimmerMode, u32) mode);
extern void player_warp_same_floor(KF_ENUM_PARAM(KfMapVariant, u32) variant, s32 cell_x, s32 cell_z);
extern void player_warp_to_floor_entry(void);
extern KfBoolU32 player_warp_trigger_update(void);

#endif
