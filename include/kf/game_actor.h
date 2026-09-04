#ifndef KF_GAME_ACTOR_H
#define KF_GAME_ACTOR_H

/*
 * Actor and combatant prototypes.
 *
 * Generated during extern-crutch removal: declarations that were duplicated
 * as `extern` across src/game/*.c now live here once. DAT_/func_ spellings
 * remain unresolved WIP identities. Byte-neutral: a declaration never changes
 * codegen.
 */

#include <kf/semantic_types.h>

extern s32 actor_animation_crossed_phase(const KfActor *actor, u16 phase);
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
extern void actor_prepare_charge_toward_player(void);
extern void actor_select_next_action(s32 player_distance);
extern void actor_set_action(KfActor *actor, u8 action);
extern void actor_set_player_transform( const VECTOR *position, const SVECTOR *rotation);
extern u8 actor_try_select_action_distance_facing( u8 action, s32 distance, u16 chance, u16 distance_scale);
extern u8 actor_try_select_facing_action(u8 action, s32 distance, u16 chance);
extern u8 actor_try_select_ground_action(u8 action, s32 distance, u16 chance);
extern u8 actor_try_select_profiled_action(u8 action, s32 distance, u8 profile_index, u16 chance);
extern void actor_update_awareness(void);
extern void actor_update_current_action(void);
extern void actor_update_effect_action(s32 action);

#endif
