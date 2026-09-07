#ifndef KF_GAME_ACTOR_H
#define KF_GAME_ACTOR_H

/* Actor and combatant layouts, state, and operations. */

#include <kf/game_types.h>
#include <kf/enum.h>
#include <kf/psyq.h>
#include <kf/audio.h>
#include <kf/game_math.h>

struct KfPoolRecord;

enum {
    KF_ACTOR_DEFINITION_COUNT = 12,
    KF_ACTOR_CAPACITY = 128
};

KF_ENUM_BEGIN(KfActorSlotState, u8)
    KF_ACTOR_SLOT_DYNAMIC = 0,
    KF_ACTOR_SLOT_PERSISTENT = 1,
    KF_ACTOR_SLOT_RESPAWNING = 2,
    KF_ACTOR_SLOT_HOMEBOUND = 3,
    KF_ACTOR_SLOT_FREE = 0xff
KF_ENUM_END(KfActorSlotState)

/* Lifecycle controls activation independently of slot/respawn policy. */
KF_ENUM_BEGIN(KfActorLifecycle, u8)
    KF_ACTOR_LIFECYCLE_DORMANT = 0,
    KF_ACTOR_LIFECYCLE_ACTIVE = 1,
    KF_ACTOR_LIFECYCLE_WAIT_FOR_RANGE_EXIT = 2,
    KF_ACTOR_LIFECYCLE_DISABLED = 3
KF_ENUM_END(KfActorLifecycle)

KF_ENUM_BEGIN(KfActorAction, u8)
    KF_ACTOR_ACTION_IDLE = 0,
    KF_ACTOR_ACTION_WANDER = 1,
    KF_ACTOR_ACTION_PURSUE = 2,
    KF_ACTOR_ACTION_RETREAT = 3,
    KF_ACTOR_ACTION_MELEE_ATTACK = 4,
    KF_ACTOR_ACTION_HIT_REACTION = 5,
    KF_ACTOR_ACTION_DYING = 6,
    KF_ACTOR_ACTION_JUMP_ATTACK = 16,
    KF_ACTOR_ACTION_SPECIAL_ATTACK = 17,
    KF_ACTOR_ACTION_DRIFT = 18,
    KF_ACTOR_ACTION_EFFECT0 = 19,
    KF_ACTOR_ACTION_EFFECT1 = 20,
    KF_ACTOR_ACTION_EFFECT2 = 21,
    KF_ACTOR_ACTION_MULTI_HIT_ATTACK = 22,
    KF_ACTOR_ACTION_EXIT_BLOCKED_PLACEMENT = 32,
    KF_ACTOR_ACTION_RETURN_HOME = 33,
    KF_ACTOR_ACTION_POST_DEATH = 127,
    KF_ACTOR_ACTION_NONE = 0xff
KF_ENUM_END(KfActorAction)

/* LOCKED suppresses automatic selection; damage can still change the action. */
enum {
    KF_ACTOR_PROGRESS_INIT = 0,
    KF_ACTOR_PROGRESS_RUNNING = 1,
    KF_ACTOR_PROGRESS_LOCKED = 0xf0,
    KF_ACTOR_PROGRESS_COMPLETE = 0xff
};

KF_ENUM_BEGIN(KfActorVerticalState, u8)
    KF_ACTOR_VERTICAL_NONE = 0,
    KF_ACTOR_VERTICAL_STEP_UP = 1,
    KF_ACTOR_VERTICAL_FALL = 2,
    KF_ACTOR_VERTICAL_LONG_DROP = 3,
    KF_ACTOR_VERTICAL_JUMP_ATTACK = 4
KF_ENUM_END(KfActorVerticalState)

/* Horizontal steering state, distinct from collision-query return codes. */
KF_ENUM_BEGIN(KfActorCollisionState, u8)
    KF_ACTOR_COLLISION_CLEAR = 0,
    KF_ACTOR_COLLISION_SLIDING = 1,
    KF_ACTOR_COLLISION_BLOCKED = 2
KF_ENUM_END(KfActorCollisionState)

/* Parallel definition-table indices, independent of action and resource IDs. */
enum {
    KF_ACTOR_ANIM_SLOT_IDLE = 0,
    KF_ACTOR_ANIM_SLOT_MOVE = 1,
    KF_ACTOR_ANIM_SLOT_MELEE = 2,
    KF_ACTOR_ANIM_SLOT_HIT_REACTION = 3,
    KF_ACTOR_ANIM_SLOT_DEATH = 4,
    KF_ACTOR_ANIM_SLOT_JUMP_ATTACK = 5,
    KF_ACTOR_ANIM_SLOT_SPECIAL_ATTACK = 6,
    KF_ACTOR_ANIM_SLOT_DRIFT = 7,
    KF_ACTOR_ANIM_SLOT_EFFECT0 = 8,
    KF_ACTOR_ANIM_SLOT_EFFECT1 = 9,
    KF_ACTOR_ANIM_SLOT_EFFECT2 = 10,
    KF_ACTOR_ANIM_SLOT_MULTI_HIT_ATTACK = 11,
    KF_ACTOR_ANIM_SLOT_COUNT = 16
};

enum {
    KF_ACTOR_ANIMATION_NONE = 0xff,
    KF_ACTOR_ANIMATION_PHASE_PERIOD = 0x1000,
    KF_ACTOR_ANIMATION_PHASE_MAX = KF_ACTOR_ANIMATION_PHASE_PERIOD - 1,
    KF_ACTOR_AIM_TOLERANCE = 0x155
};

enum {
    KF_ACTOR_SOUND_ATTACK = 0,
    KF_ACTOR_SOUND_HIT_REACTION = 1,
    KF_ACTOR_SOUND_DEATH = 2,
    KF_ACTOR_SOUND_COUNT = 3
};

/* Shared actor selection/action ranges and random-angle extraction. */
enum {
    KF_ACTOR_MULTI_HIT_FORWARD_MIN_RANGE = 8000,
    KF_ACTOR_MULTI_HIT_MAX_RANGE = 11000,
    KF_ACTOR_RANDOM_YAW_SHIFT = 3,
    KF_ACTOR_BOSS_DEATH_SOUND_COUNT = 4
};

/* Definition flags in the placement stream; unrelated to effect-code bits. */
enum {
    KF_ACTOR_PLACEMENT_DEFINITION_MASK = 0x1f,
    KF_ACTOR_PLACEMENT_VARIANT = 0x20
};

/* The same encoded effect byte selects its action profile and spawned kind. */
enum {
    KF_ACTOR_ACTION_PROFILE_COUNT = 25,
    KF_ACTOR_EFFECT_KIND_MASK = 0x1f,
    KF_ACTOR_EFFECT_PAIRED = 0x20
};

enum {
    KF_ACTOR_PARAM_EFFECT0 = 0,
    KF_ACTOR_PARAM_EFFECT1 = 1,
    KF_ACTOR_PARAM_EFFECT2 = 2,
    KF_ACTOR_PARAM_EFFECT0_CHANCE = 3,
    KF_ACTOR_PARAM_EFFECT1_CHANCE = 4,
    KF_ACTOR_PARAM_EFFECT2_CHANCE = 5,
    KF_ACTOR_PARAM_DROP_OBJECT = 6,
    KF_ACTOR_PARAM_DROP_CHANCE = 7,
    KF_ACTOR_PARAM_COUNT = 8
};

/* Only the exact credit nibble 0x10 awards player training and experience. */
enum {
    KF_ACTOR_DAMAGE_SCALE_ONE = 5000,
    KF_ACTOR_DAMAGE_CREDIT_MASK = 0xf0,
    KF_ACTOR_DAMAGE_CREDIT_PLAYER = 0x10
};

/*
 * GAME.EXE keeps twelve 0x98-byte actor definitions immediately before a
 * pool of 128 0x48-byte live actors. Only reviewed fields are named; the
 * remaining bytes deliberately stay opaque.
 *
 * The three animation tables share KF_ACTOR_ANIM_SLOT indices. Their byte
 * entries contain resource animation IDs, or KF_ACTOR_ANIMATION_NONE when
 * unavailable. Actions 19..21 use effect slots 8..10; slots 12..15 have no
 * decoded dispatcher use.
 */
typedef struct KfActorDefinition {
    u8 pursuit_distance_scale; /* action 2 distance threshold, in units of 256 */
    u8 model_and_texture;
    u8 melee_attack_chance;
    u8 status_effect;
    u8 status_effect_chance;
    u8 action_parameters[KF_ACTOR_PARAM_COUNT];
    u8 move_speed;
    u8 action_animations[KF_ACTOR_ANIM_SLOT_COUNT];
    u8 turn_rate;
    SoundRef sounds[KF_ACTOR_SOUND_COUNT];
    struct KfVec3s attachment_offsets[2];
    s16 special_attack_chance;
    s16 special_attack_range;
    u8 unknown_38[2];
    u16 action_animation_steps[KF_ACTOR_ANIM_SLOT_COUNT];
    u16 action_animation_phases[KF_ACTOR_ANIM_SLOT_COUNT];
    u16 collision_radius;
    u16 collision_height;
    u16 awareness_distance;
    u16 initial_health;
    u16 effect_owner_id;
    u16 experience_reward;
    u16 attack_components[3];
    u16 defenses[5];
    u16 gold_drop_limit; /* exclusive upper bound of rand-scaled gold drop */
} KfActorDefinition;

typedef struct KfActorActionProfile {
    s16 far_distance;
    s16 far_weight;
    s16 near_distance;
    s16 middle_weight;
    s16 near_weight;
} KfActorActionProfile;

/* 16-byte actor placement record from the map's MIXA.DAT stream. */
typedef struct KfActorPlacement {
    KfActorSlotState slot_state;
    u8 definition_flags;
    u8 heading_quadrant;
    u8 tile_z;
    u8 tile_x;
    u8 spawn_chance;
    u8 death_drop_object_id;
    u8 unknown_07[3];
    s16 local_z;
    s16 local_x;
    u8 unknown_0e[2];
} KfActorPlacement;

typedef struct KfActor {
    KfActorSlotState slot_state;
    u8 definition_id;
    u8 variant;
    u8 heading_quadrant;
    u8 tile_z;
    u8 tile_x;
    KfActorLifecycle lifecycle;
    u8 spawn_chance;
    KfActorAction action;
    u8 death_drop_object_id;
    u8 animation_id;
    KfActorVerticalState vertical_state;
    u8 unknown_0c[2];
    s16 local_z;
    s16 local_x;
    u16 animation_phase;
    u16 health;
    u16 cell_x;
    u16 cell_z;
    s16 unknown_1a;
    VECTOR position;
    struct KfEulerAngles rotation;
    u16 unknown_32;
    struct KfPoolRecord *animation_cache;
    u8 action_progress;
    KfActorCollisionState collision_state;
    s16 movement_yaw;
    s16 animation_step;
    s16 vertical_velocity;
    s16 movement_x;
    s16 movement_z;
    s16 movement_y;
    u8 unknown_46[2];
} KfActor;

#if KF_MODERN_TYPES
static_assert(__builtin_offsetof(KfActorPlacement, slot_state) == 0);
static_assert(__builtin_offsetof(KfActor, slot_state) == 0);
static_assert(__builtin_offsetof(KfActor, lifecycle) == 0x06);
static_assert(__builtin_offsetof(KfActor, action) == 0x08);
static_assert(__builtin_offsetof(KfActor, vertical_state) == 0x0b);
static_assert(__builtin_offsetof(KfActor, collision_state) == 0x39);
#endif

/*
 * Actor routines derive the definition array and current context from the
 * live-actor base through one register, proving this complete aggregate.
 */
typedef struct KfActorState {
    KfActorDefinition definitions[KF_ACTOR_DEFINITION_COUNT];
    KfActor actors[KF_ACTOR_CAPACITY];
    VECTOR player_position;
    SVECTOR player_rotation;
    KfActorDefinition *current_definition;
    KfActor *current;
    u16 current_index;
    u16 current_definition_id;
    KfActor *player_target;
} KfActorState;

extern KfActorActionProfile actor_action_profiles[KF_ACTOR_ACTION_PROFILE_COUNT];
extern KfActorState actor_state;
extern const SoundRef boss_death_loop_sound;
extern SoundRef boss_death_phase_sounds[KF_ACTOR_BOSS_DEATH_SOUND_COUNT];

extern s32 actor_animation_crossed_phase(const KfActor *actor, u16 phase);
extern void actor_advance_animation_clamped(KfActor *actor, s16 delta);
extern void actor_advance_animation_wrapped(KfActor *actor, s16 delta);
extern void actor_apply_horizontal_movement(void);
extern void actor_apply_random_movement(s16 step, s16 limit);
extern void actor_bind_current(KfActor *actor);
extern void actor_apply_damage(
    u16 actor_index, u16 base_power, u16 component0, u16 component1,
    u16 component2, u16 component3, u16 component4, u16 scale, u16 hit_flags);
extern void actor_definitions_load(const KfActorDefinition *definitions);
extern void actor_initialize(KfActor *actor);
extern void actor_initialize_current(void);
extern void actor_initialize_slot(u16 actor_index);
extern s32 actor_move_along_heading(s32 direction, s32 stop_on_collision);
extern s32 actor_move_xz_with_collision(const struct KfVecXZs *delta, s32 stop_on_collision);
extern void actor_play_sound_at_phase(const SoundRef *sound, u16 phase);
extern void actor_pool_begin_death_by_definition(u16 definition_id);
extern void actor_pool_clear(void);
extern void actor_pool_apply_radial_damage(
    const struct KfVec3i *origin, u32 radius, u16 falloff, u16 base_power,
    u16 component0, u16 component1, u16 component2, u16 component3,
    u16 component4, u16 scale, u16 hit_flags);
extern s32 actor_pool_find_at_tile(u8 tile_x, u8 tile_z);
extern KfActor *actor_pool_find_target_in_cone(
    const struct KfVec3i *origin, s16 facing, u32 max_distance,
    s32 angle_tolerance, s32 *distance_out);
extern s32 actor_pool_find_overlap(s32 x, s32 y, s32 z, s32 extra_radius, s32 point_height);
extern void actor_pool_load_placements(const KfActorPlacement *placements);
extern void actor_pool_spawn(
    u8 definition_id, const struct KfVec3i *position,
    const struct KfVec3s *rotation);
extern void actor_pool_update(void);
extern s32 actor_distance_to_point(
    const KfActor *actor, s32 point_x, s32 point_y, s32 point_z,
    s32 max_distance, s32 actor_height, s32 point_height);
extern void actor_prepare_charge_toward_player(void);
extern void actor_select_next_action(s32 player_distance);
extern void actor_set_action(KfActor *actor, KfActorAction action);
extern void actor_set_player_transform( const VECTOR *position, const SVECTOR *rotation);
extern KfActorAction actor_try_select_action_distance_facing(
    KfActorAction action, s32 distance, u16 chance, u16 distance_scale);
extern KfActorAction actor_try_select_facing_action(
    KfActorAction action, s32 distance, u16 chance);
extern KfActorAction actor_try_select_ground_action(
    KfActorAction action, s32 distance, u16 chance);
extern KfActorAction actor_try_select_profiled_action(
    KfActorAction action, s32 distance, u16 profile_index, u16 chance);
extern void actor_try_attack_player(
    u16 minimum_distance, u16 maximum_distance,
    s16 angle_offset, s16 angle_tolerance);
extern void actor_update_awareness(void);
extern void actor_update_current_action(void);
extern void actor_update_effect_action(s32 action);
extern void actor_transform_definition5_to6(KfActor *actor);

#endif
