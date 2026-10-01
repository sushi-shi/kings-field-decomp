#pragma once
#include <kf/net/codec.h>
#ifdef __cplusplus
extern "C" {
#endif

enum {
    KF_WORLD_PLAYERS = 4, KF_WORLD_ACTORS = 128, KF_WORLD_EFFECTS = 48,
    KF_WORLD_OBJECTS = 190, KF_WORLD_EVENTS = 8, KF_WORLD_ACTOR_DEFINITIONS = 12,
    KF_WORLD_ACTIONS = 16, KF_WORLD_FLOORS = 5, KF_WORLD_GRID_CELLS = 10000,
    KF_WORLD_ASSETS = 48, KF_WORLD_OBJECT_DEFINITIONS = 160,
    KF_WORLD_QUEST_REWARD_MASK = 31, KF_WORLD_SOUNDS = 32,
    KF_WORLD_CAST_POSE_TICKS = 12
};

/* Pointer-free semantic records. Explicit Rust encoding defines the wire format;
 * native struct padding and resource/cache pointers never enter a packet. */
typedef struct KfNetWorldHeader {
    uint32_t epoch, tick;
    int32_t floor;
    uint8_t full, variant;
} KfNetWorldHeader;

typedef struct KfNetWorldPlayer {
    int32_t experience;
    int32_t next_level_experience;
    uint8_t progress_state_level;
    uint8_t progress_state_unknown_01;
    uint8_t progress_state_current_floor;
    uint8_t progress_state_highest_floor;
    uint8_t map_variant;
    uint8_t allow_near_actor_spawn;
    uint8_t weapon_charge_delay;
    uint8_t unknown_0f;
    uint16_t vitals_maximum_hp;
    uint16_t vitals_current_hp;
    uint16_t vitals_maximum_mp;
    uint16_t vitals_current_mp;
    uint16_t attack_charge_state_current;
    uint16_t attack_charge_state_committed;
    uint16_t magic_charge;
    uint16_t physical_power_training;
    uint16_t magic_training;
    uint16_t base_physical_power;
    uint16_t base_magic;
    uint16_t physical_power;
    uint16_t magic;
    uint16_t status_effect_flags;
    uint32_t gold;
    uint16_t cutting_attack;
    uint16_t striking_attack;
    uint16_t piercing_attack;
    uint16_t holy_attack;
    uint16_t fire_attack;
    uint8_t unknown_3a[2];
    uint16_t cutting_defense;
    uint16_t striking_defense;
    uint16_t piercing_defense;
    uint16_t poison_resistance;
    uint16_t magic_defense;
    uint16_t fire_defense;
    int16_t curse_timer;
    int16_t darkness_timer;
    int16_t poison_timer;
    int16_t slowed_timer;
    int16_t fire_defense_timer;
    int16_t illusion_staff_timer;
    uint8_t unknown_54[4];
    uint32_t equipment_effect_ticks;
    uint8_t selected_magic_id;
    uint8_t unknown_5d[3];
    uint8_t equipped_weapon_id;
    uint8_t unknown_65[3];
    int16_t weapon_attack_phase;
    uint8_t unknown_72[2];
    uint8_t weapon_magic_shots_remaining;
    uint8_t weapon_magic_delay;
    uint8_t weapon_attack_fully_charged;
    uint8_t unknown_7b[1];
    uint8_t equipped_head_armor_id;
    uint8_t equipped_body_armor_id;
    uint8_t equipped_shield_id;
    uint8_t equipped_arm_armor_id;
    uint8_t equipped_leg_armor_id;
    uint8_t equipped_accessory_id;
    uint8_t audio_effects_enabled;
    uint8_t audio_music_enabled;
    uint8_t hud_gauges_enabled;
    uint8_t compass_enabled;
    int16_t view_rotation_offset_vx;
    int16_t view_rotation_offset_vy;
    int16_t view_rotation_offset_vz;
    uint8_t update_state;
    uint8_t unknown_a3;
    int32_t camera_position_vx;
    int32_t camera_position_vy;
    int32_t camera_position_vz;
    int32_t foot_height;
    int16_t camera_rotation_vx;
    int16_t camera_rotation_vy;
    int16_t camera_rotation_vz;
    int16_t motion_state_strafe_velocity;
    int16_t motion_state_forward_velocity;
    uint16_t motion_state_movement_speed;
    int16_t motion_state_yaw_step;
    int16_t motion_state_pitch_step;
    uint8_t motion_state_map_cell_x;
    uint8_t motion_state_map_cell_z;
    uint8_t previous_map_cell_x;
    uint8_t previous_map_cell_z;
    uint8_t unknown_ce[6];
    int16_t view_bob_offset;
    uint16_t view_bob_phase;
    uint16_t death_camera_pitch_step;
    int16_t death_visual_blend;
    int16_t vertical_velocity;
    uint8_t vertical_state;
    uint8_t unknown_df[1];
    uint8_t party_slot;
    uint32_t previous_input;
    uint8_t cast_pose_ticks;
    int32_t movement_velocity_limit;
    int32_t turn_step_limit;
    uint8_t item_stock[3][80];
    uint8_t learned_magic[24];
    uint32_t random_state;
} KfNetWorldPlayer;

typedef struct KfNetWorldActor {
    uint32_t generation;
    uint32_t random_state;
    uint8_t target_player_slot;
    uint32_t target_player_generation;
    uint16_t transform_step;
    uint8_t slot_state;
    uint8_t definition_id;
    uint8_t culling_mode;
    uint8_t heading_quadrant;
    uint8_t tile_z;
    uint8_t tile_x;
    uint8_t lifecycle;
    uint8_t spawn_chance;
    uint8_t action;
    uint8_t death_drop_object_id;
    uint8_t animation_clip;
    uint8_t vertical_state;
    uint8_t unknown_0c[2];
    int16_t local_z;
    int16_t local_x;
    uint16_t animation_phase;
    uint16_t health;
    uint16_t cell_x;
    uint16_t cell_z;
    int16_t unknown_1a;
    int32_t position_vx;
    int32_t position_vy;
    int32_t position_vz;
    int16_t rotation_vector_vx;
    int16_t rotation_vector_vy;
    int16_t rotation_vector_vz;
    uint8_t action_progress;
    uint8_t collision_state;
    int16_t movement_yaw;
    int16_t animation_step;
    int16_t vertical_velocity;
    int16_t movement_x;
    int16_t movement_z;
    int16_t movement_y;
    uint8_t unknown_46[2];
} KfNetWorldActor;

typedef struct KfNetWorldEffect {
    uint8_t owner_player_slot;
    uint32_t owner_player_generation;
    uint32_t generation;
    uint32_t age;
    uint32_t random_state;
    uint8_t target_player_slot;
    uint32_t target_player_generation;
    uint8_t slot_type;
    uint8_t kind;
    uint8_t base_render_id;
    uint8_t render_id;
    uint8_t animation_clip;
    uint8_t sound_played;
    uint8_t id;
    uint8_t phase;
    uint16_t visual;
    uint16_t unknown_0a;
    int32_t position_vx;
    int32_t position_vy;
    int32_t position_vz;
    int16_t rotation_vector_vx;
    int16_t rotation_vector_vy;
    int16_t rotation_vector_vz;
    uint16_t scale_x;
    uint16_t scale_y;
    uint16_t scale_z;
    uint16_t unknown_2a;
    int16_t direction_vector_vx;
    int16_t direction_vector_vy;
    int16_t direction_vector_vz;
    uint16_t control;
    uint16_t propagation;
} KfNetWorldEffect;

typedef struct KfNetWorldObject {
    uint32_t generation;
    uint8_t object_id;
    uint8_t unknown_01;
    uint16_t cell_x;
    uint16_t cell_z;
    uint8_t unknown_06[2];
    int32_t position_vx;
    int32_t position_vy;
    int32_t position_vz;
    int16_t rotation_vector_vx;
    int16_t rotation_vector_vy;
    int16_t rotation_vector_vz;
    uint32_t link_words[2];
    uint8_t action;
    uint8_t unknown_29;
    uint16_t action_timer;
} KfNetWorldObject;

typedef struct KfNetWorldEvent {
    uint8_t state;
    uint8_t character_id;
    uint8_t model_index;
    uint8_t dialogue_pages_last_page[5];
    uint8_t dialogue_stage_limit;
    uint8_t dialogue_stage;
    uint8_t dialogue_page;
    uint8_t dialogue_page_delay;
    uint8_t unknown_0c;
    uint8_t unknown_0d;
    uint8_t behavior;
    uint8_t animation_clip;
    uint8_t collision_turn_pending;
    uint8_t unknown_11;
    uint16_t animation_phase;
    int32_t home_x;
    int32_t home_z;
    uint16_t cell_x;
    uint16_t cell_z;
    uint16_t radius;
    uint16_t unknown_22;
    int32_t reference_position_vx;
    int32_t reference_position_vy;
    int32_t reference_position_vz;
    int16_t rotation_vx;
    int16_t rotation_vy;
    int16_t rotation_vz;
    int16_t rotation_target;
    uint16_t unknown_42;
} KfNetWorldEvent;

typedef struct KfNetWorldMember {
    uint8_t presence;
    uint8_t character_id[32];
    uint32_t generation, acknowledged_input;
    uint32_t quest_rewards;
    uint8_t connected, avatar;
    KfNetWorldPlayer player;
    uint8_t loot_claims[KF_WORLD_FLOORS][KF_WORLD_OBJECTS];
} KfNetWorldMember;
typedef struct KfNetWorldFloor {
    uint8_t script[10], records[1690];
} KfNetWorldFloor;
typedef struct KfNetWorldStory {
    uint8_t kind, initiator, effect;
    uint16_t tick, object;
    uint32_t generation;
    int32_t camera_x, camera_y, camera_z;
    int16_t pitch, yaw, roll;
    uint8_t page, ready_mask;
} KfNetWorldStory;
typedef struct KfNetWorldSound {
    uint32_t sequence;
    int32_t x, y, z;
    uint8_t program, tone, note, volume;
    int32_t max_distance, attenuation_distance;
} KfNetWorldSound;
typedef struct KfNetWorld {
    KfNetWorldHeader header;
    uint32_t quest_rewards;
    KfNetWorldMember members[KF_WORLD_PLAYERS];
    KfNetWorldActor actors[KF_WORLD_ACTORS];
    uint8_t action_animations[KF_WORLD_ACTOR_DEFINITIONS][KF_WORLD_ACTIONS];
    KfNetWorldEffect effects[KF_WORLD_EFFECTS];
    KfNetWorldObject objects[KF_WORLD_OBJECTS];
    KfNetWorldEvent events[KF_WORLD_EVENTS];
    uint16_t gold_drop_sequence, definition_drop_sequence, placement_drop_sequence;
    uint16_t dialogue_advance_gate, ambient_script_countdown;
    KfNetWorldFloor floors[KF_WORLD_FLOORS];
    uint32_t sound_sequence;
    KfNetWorldSound sounds[KF_WORLD_SOUNDS];
    uint8_t collision_flags[KF_WORLD_GRID_CELLS], cell_orientation[KF_WORLD_GRID_CELLS];
    uint8_t floor_height[KF_WORLD_GRID_CELLS], collision[KF_WORLD_GRID_CELLS], cell_attribute[KF_WORLD_GRID_CELLS];
    uint32_t random_state;
    KfNetWorldStory story;
} KfNetWorld;

/* Trusted limits from the loaded local resources. -1 means absent, 0 static. */
typedef struct KfNetWorldLimits {
    int32_t asset_clips[KF_WORLD_ASSETS];
    uint8_t actor_assets[KF_WORLD_ACTOR_DEFINITIONS];
    uint8_t object_operations[KF_WORLD_OBJECT_DEFINITIONS];
} KfNetWorldLimits;

typedef struct KfNetWorldSummary {
    uint32_t experience;
    uint16_t hp, maximum_hp, mp, maximum_mp;
    uint8_t floor, level, owner[32];
} KfNetWorldSummary;

/* Failure leaves decoded outputs unchanged. Caller-owned buffers/records must
 * be valid, aligned and disjoint, as for the other codec entry points. */
KfCodecResult kf_net_world_info(const uint8_t *, size_t, KfNetWorldHeader *);
/* Catalogue metadata only. Loading still requires resource-aware world_decode. */
KfCodecResult kf_net_world_summary(const uint8_t *, size_t, KfNetWorldSummary *);
KfCodecResult kf_net_world_encode(const KfNetWorld *, const KfNetWorldLimits *, uint8_t *, size_t, size_t *);
KfCodecResult kf_net_world_decode(const uint8_t *, size_t, const KfNetWorldLimits *, KfNetWorld *);
#ifdef __cplusplus
}
#endif
