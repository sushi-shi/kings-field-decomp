#ifndef KF_GAME_PLAYER_H
#define KF_GAME_PLAYER_H

struct WorldState;

struct PlayerContext;

#include <kf/platform/frame_task.hpp>
#include <kf/game/combat.h>
#include <kf/game/audio.h>
#include <kf/game/effect.h>
#include <kf/lib/map.h>
#include <kf/game/equipment.h>
#include <kf/game/magic.h>
#include <kf/game/player_motion.h>
#include <kf/game/player_status.h>
#include <kf/lib/enum.h>
#include <kf/lib/player_stats_types.h>

#include <array>
#include <kf/platform/input.h>
#include <kf/lib/random.h>

struct KfAnimationCacheRecord;

enum {
    KF_PLAYER_SOUND_WEAPON_ATTACK = 0,
    KF_PLAYER_SOUND_DEATH = 1,
    KF_PLAYER_SOUND_LEVEL_UP = 2,
    KF_PLAYER_SOUND_COUNT = 3
};

enum class KfPlayerOption : s32 {
    KF_PLAYER_OPTION_OFF = 0,
    KF_PLAYER_OPTION_ON = 1
}; using enum KfPlayerOption;

enum class KfWarpShimmerMode : s32 {
    KF_WARP_SHIMMER_GROW_REMOVE = 0,
    KF_WARP_SHIMMER_SHRINK_REMOVE = 1,
    KF_WARP_SHIMMER_GROW_KEEP = 2
}; using enum KfWarpShimmerMode;

enum {
    KF_PLAYER_FLOOR_ENTRY_COUNT = 5
};

enum class KfPlayerUpdateState : u8 {
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
}; using enum KfPlayerUpdateState;
inline KfPlayerUpdateState& operator++(KfPlayerUpdateState& value)
    { value = static_cast<KfPlayerUpdateState>(static_cast<u8>(value) + 1); return value; }
    inline KfPlayerUpdateState operator++(KfPlayerUpdateState& value, int)
    { KfPlayerUpdateState previous = value; ++value; return previous; }
    inline KfPlayerUpdateState& operator--(KfPlayerUpdateState& value)
    { value = static_cast<KfPlayerUpdateState>(static_cast<u8>(value) - 1); return value; }
    inline KfPlayerUpdateState operator--(KfPlayerUpdateState& value, int)
    { KfPlayerUpdateState previous = value; --value; return previous; }

enum class KfActorSpawnPermission : u8 {
    KF_ACTOR_NEAR_SPAWN_FORBIDDEN = 0,
    KF_ACTOR_NEAR_SPAWN_ALLOWED = 1
}; using enum KfActorSpawnPermission;

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

enum {
    KF_PLAYER_DAMAGE_MULTIPLIER_ONE = 10
};

typedef struct KfPlayerProgressState {
    u8 level;
    u8 unknown_01;
    KfFloorId current_floor;
    KfFloorId highest_floor;
} KfPlayerProgressState;

typedef struct KfPlayerMotionState {
    s16 strafe_velocity;
    s16 forward_velocity;
    u16 movement_speed;
    s16 yaw_step;
    s16 pitch_step;
    KfMapCellCoordinates map_cell;
} KfPlayerMotionState;

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
    std::array<u8, 2> unknown_3a;
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
    std::array<u8, 4> unknown_54;
    u32 equipment_effect_ticks;
    KfEffectKind selected_magic_id;
    std::array<u8, 3> unknown_5d;
    KfMagicRecord *selected_magic_record;
    KfObjectId equipped_weapon_id;
    std::array<u8, 3> unknown_65;
    KfWeaponRecord *equipped_weapon_record;
    u8 *weapon_asset_buffer;
    s16 weapon_attack_phase;
    std::array<u8, 2> unknown_72;
    struct KfAnimationCacheRecord *weapon_animation_cache;
    u8 weapon_magic_shots_remaining;
    u8 weapon_magic_delay;
    KfWeaponAttackCharge weapon_attack_fully_charged;
    std::array<u8, 1> unknown_7b;
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
    KfPlayerOption audio_effects_enabled;
    KfPlayerOption audio_music_enabled;
    KfPlayerOption hud_gauges_enabled;
    KfPlayerOption compass_enabled;
    SVECTOR view_rotation_offset;
    KfPlayerUpdateState update_state;
    u8 unknown_a3;
    VECTOR camera_position;
    s32 foot_height;
    SVECTOR camera_rotation;
    KfPlayerMotionState motion_state;
    KfMapCellCoordinates previous_map_cell;
    std::array<u8, 6> unknown_ce;
    s16 view_bob_offset;
    u16 view_bob_phase;
    u16 death_camera_pitch_step;
    s16 death_visual_blend;
    s16 vertical_velocity;
    KfPlayerVerticalState vertical_state;
    std::array<u8, 1> unknown_df;
} KfPlayerState;

// Local view state is neither a character save nor authoritative world state.
struct PlayerPresentation {
    VECTOR position_snapshot {};
    SVECTOR rotation_snapshot {};
    MATRIX death_saved_color_matrix {};
    s32 death_saved_fog_near {};
};

struct PlayerInput {
    u32 buttons {};
    kf::LookDelta look {};
};

// Gameplay always receives the character it operates on. Asset references in
// KfPlayerState remain process-local and must never enter the wire/save format.
struct PlayerActions;
struct PlayerContext {
    u8 party_slot = 0xff;
    kf::RandomStream random {};
    bool local_view = true;
    bool prediction {};
    PlayerActions *actions {};
    KfPlayerState state {};
    std::array<std::array<u8, KF_ITEM_COUNT>, KF_ITEM_STOCK_BANK_COUNT> item_stock {};
    std::array<KfMagicLearningState, KF_MAGIC_RECORD_COUNT> learned_magic {};
    u32 previous_input {};
    u8 cast_pose_ticks {};
    s32 movement_velocity_limit {};
    s32 turn_step_limit {};
    PlayerPresentation presentation {};
};

typedef struct KfFloorEntryCell {
    u8 x;
    u8 z;
} KfFloorEntryCell;

extern std::array<SoundRef, KF_PLAYER_SOUND_COUNT> player_sound_refs;
extern std::array<KfFloorEntryCell, KF_PLAYER_FLOOR_ENTRY_COUNT> floor_entry_cells;
void player_refresh_weapon_asset(PlayerContext &player);
extern std::array<KfPlayerLevelGrowth, KF_PLAYER_LEVEL_GROWTH_COUNT> player_level_growth_table;

extern void player_get_floor_position(PlayerContext &player, VECTOR &position);

extern bool player_item_is_equipped(PlayerContext &player, KfObjectId item_id);
extern KfMapAttribute player_current_map_attribute(WorldState &world, PlayerContext &player);

extern void player_add_experience(PlayerContext &player, s16 amount);
extern void player_adjust_hp(PlayerContext &player, s32 delta);
extern void player_adjust_mp(PlayerContext &player, s32 delta);
extern void player_apply_damage(PlayerContext &player,
    u16 component0, u16 component1, u16 component2,
    KfPlayerStatusFlags status_effect_flags, u16 component3, u16 component4,
    u16 scale_q12, u16 multiplier_tenths);
extern void player_begin_weapon_attack(PlayerContext &player);
extern void player_apply_radial_damage(PlayerContext &player,
    const VECTOR *origin, u32 radius, u16 falloff_q12,
    u16 component0, u16 component1, u16 component2, u16 component3, u16 component4,
    u16 scale_q12, u16 multiplier_tenths);
extern s32 player_calculate_damage_component(s32 defender_power, s32 defense, s32 attack);
extern void player_clear_motion(PlayerContext &player);
extern void player_death_apply_visual_fade(PlayerContext &player, const MATRIX *color_from, s32 blend);
extern void player_death_begin(PlayerContext &player);
extern kf::FrameTask<void> player_death_restart(WorldState &world, PlayerContext &player);
extern kf::FrameTask<void> player_death_update(WorldState &world, PlayerContext &player);
extern void player_death_update_reverse_fade(PlayerContext &player);
extern s32 player_distance_to_point(PlayerContext &player,
    s32 point_x, s32 point_y, s32 point_z, s32 max_distance,
    s32 point_height);
extern s32 player_distance_to_point_in_cone(PlayerContext &player,
    const VECTOR *point, s16 facing, s32 max_distance,
    s32 angle_tolerance);
extern void player_equip_weapon(PlayerContext &player, KfObjectId weapon_id);
extern void game_initialize_session(WorldState &world, PlayerContext &player);
extern void game_state_initialize(WorldState &world, PlayerContext &player);
extern void player_initialize_character(WorldState &world, PlayerContext &player);
extern s32 player_random_next(PlayerContext &player);
extern void player_increment_magic_training(PlayerContext &player);
extern void player_increment_physical_power_training(PlayerContext &player);
extern s32 player_move_horizontal(WorldState &world, PlayerContext &player, s32 heading, s32 distance);
extern void player_recalculate_combat_stats(PlayerContext &player);
extern kf::FrameTask<void> player_restore_vitals_with_color_cycle(WorldState &world, PlayerContext &player);
extern void player_select_magic(WorldState &world, PlayerContext &player, KfEffectKind magic_id);
extern void player_set_equipment_slot(PlayerContext &player, KfObjectId item_id, KfEquipmentSlot slot);
extern void player_apply_fire_defense_boost(PlayerContext &player);
extern void player_sync_position_to_map(WorldState &world, PlayerContext &player);
extern kf::FrameTask<void> player_use_item(WorldState &world, PlayerContext &player, KfObjectId item_id);
extern kf::FrameTask<void> player_update(WorldState &world, PlayerContext &player, PlayerInput command);
extern void player_tick_effects(WorldState &world, PlayerContext &player);
extern void player_update_transform_snapshot(PlayerContext &player,
    VECTOR *position_out, SVECTOR *rotation_out);
extern void player_update_vertical_motion(WorldState &world, PlayerContext &player);
extern void player_update_view_bob(PlayerContext &player);
extern void player_update_weapon_attack(WorldState &world, PlayerContext &player);

extern kf::FrameTask<void> player_warp_change_floor(WorldState &world, PlayerContext &player, KfFloorId floor, KfMapVariant map_variant);
extern kf::FrameTask<void> player_warp_shimmer(WorldState &world, PlayerContext &player, KfWarpShimmerMode shimmer_mode, VECTOR *position);
extern kf::FrameTask<void> player_warp_shimmer_at_player(WorldState &world, PlayerContext &player, KfWarpShimmerMode shimmer_mode);
extern kf::FrameTask<void> player_warp_same_floor(WorldState &world, PlayerContext &player, KfMapVariant map_variant, s32 cell_x, s32 cell_z);
extern kf::FrameTask<void> player_warp_to_floor_entry(WorldState &world, PlayerContext &player);
extern kf::FrameTask<bool> player_warp_trigger_update(WorldState &world, PlayerContext &player);

extern s32 fixed6_ratio_step(s32 value, s32 span);

#endif // KF_GAME_PLAYER_H
