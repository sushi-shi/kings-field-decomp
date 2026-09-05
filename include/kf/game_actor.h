#ifndef KF_GAME_ACTOR_H
#define KF_GAME_ACTOR_H

/* Actor and combatant layouts, state, and operations. */

#include <kf/game_types.h>
#include <kf/psyq.h>
#include <kf/audio.h>
#include <kf/game_math.h>

struct KfPoolRecord;

/*
 * GAME.EXE keeps twelve 0x98-byte actor definitions immediately before a
 * pool of 128 0x48-byte live actors. Only reviewed fields are named; the
 * remaining bytes deliberately stay opaque.
 *
 * Per-action tables are indexed by KF_ACTOR_ACTION_INDEX(action): the hit
 * action (5) and death action (6) occupy entries 3 and 4, and the eight
 * effect actions occupy entries 8..15 (actor_update_effect_action).
 */
typedef struct KfActorDefinition {
    u8 unknown_00[0x03];
    u8 status_effect;
    u8 status_effect_chance;
    u8 action_parameters[8];
    u8 move_speed;
    u8 action_animations[16];
    u8 turn_rate;
    SoundRef sounds[3];
    struct KfVec3s attachment_offsets[2];
    s16 unknown_34;
    s16 unknown_36;
    u8 unknown_38[2];
    u16 action_animation_steps[16];
    u16 action_animation_phases[16];
    u16 collision_radius;
    u16 collision_height;
    u16 awareness_distance;
    u16 initial_health;
    u16 unknown_82;
    u16 experience_reward;
    u16 attack_components[3];
    u16 defenses[5];
    u16 unknown_96;
} KfActorDefinition;

#define KF_ACTOR_ACTION_INDEX(action) ((action) - 2)

typedef struct KfActorActionProfile {
    s16 far_distance;
    s16 far_weight;
    s16 near_distance;
    s16 middle_weight;
    s16 near_weight;
} KfActorActionProfile;

/* 16-byte actor placement record from the map's MIXA.DAT stream. */
typedef struct KfActorPlacement {
    u8 slot_state;
    u8 definition_flags;
    u8 heading_quadrant;
    u8 tile_z;
    u8 tile_x;
    u8 unknown_05;
    u8 unknown_06;
    u8 unknown_07[3];
    s16 local_z;
    s16 local_x;
    u8 unknown_0e[2];
} KfActorPlacement;

typedef struct KfActor {
    u8 slot_state;
    u8 definition_id;
    u8 variant;
    u8 heading_quadrant;
    u8 tile_z;
    u8 tile_x;
    u8 lifecycle;
    u8 unknown_07;
    u8 action;
    u8 unknown_09;
    u8 animation_id;
    u8 vertical_state;
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
    u8 action_timer;
    u8 collision_state;
    s16 movement_yaw;
    s16 animation_step;
    s16 vertical_velocity;
    s16 movement_x;
    s16 movement_z;
    s16 movement_y;
    u8 unknown_46[2];
} KfActor;

/*
 * Actor routines derive the definition array and current context from the
 * live-actor base through one register, proving this complete aggregate.
 */
typedef struct KfActorState {
    KfActorDefinition definitions[12];
    KfActor actors[128];
    VECTOR player_position;
    SVECTOR player_rotation;
    KfActorDefinition *current_definition;
    KfActor *current;
    u16 current_index;
    u16 current_definition_id;
    KfActor *player_target;
} KfActorState;

extern KfActorActionProfile actor_action_profiles[25];
extern KfActorState actor_state;
extern const SoundRef boss_death_loop_sound;
extern SoundRef boss_death_phase_sounds[4];

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
    const struct KfVec3i *origin, s32 facing, u32 max_distance,
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
extern void actor_set_action(KfActor *actor, u8 action);
extern void actor_set_player_transform( const VECTOR *position, const SVECTOR *rotation);
extern u8 actor_try_select_action_distance_facing( u8 action, s32 distance, u16 chance, u16 distance_scale);
extern u8 actor_try_select_facing_action(u8 action, s32 distance, u16 chance);
extern u8 actor_try_select_ground_action(u8 action, s32 distance, u16 chance);
extern u8 actor_try_select_profiled_action(u8 action, s32 distance, u8 profile_index, u16 chance);
extern void actor_try_attack_player(
    u16 minimum_distance, u16 maximum_distance,
    s16 angle_offset, s16 angle_tolerance);
extern void actor_update_awareness(void);
extern void actor_update_current_action(void);
extern void actor_update_effect_action(s32 action);
extern void actor_transform_definition5_to6(KfActor *actor);

#endif
