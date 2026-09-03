#ifndef KF_GAME_ACTOR_H
#define KF_GAME_ACTOR_H

/*
 * Actor, effect, magic and combatant prototypes.
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
extern void actor_definitions_load(const KfActorDefinition *definitions);
extern void actor_initialize(KfActor *actor);
extern void actor_initialize_current(void);
extern void actor_initialize_slot(u16 actor_index);
extern s32 actor_move_along_heading(s32 direction, s32 stop_on_collision);
extern s32 actor_move_xz_with_collision(const struct KfVecXZs *delta, s32 stop_on_collision);
extern void actor_play_sound_at_phase(const SoundRef *sound, u16 phase);
extern void actor_pool_begin_death_by_definition(u16 definition_id);
extern void actor_pool_clear(void);
extern s32 actor_pool_find_at_tile(u8 tile_x, u8 tile_z);
extern s32 actor_pool_find_overlap(s32 x, s32 y, s32 z, s32 extra_radius, s32 point_height);
extern void actor_pool_load_placements(const KfActorPlacement *placements);
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
extern u32 effect_map_collision(VECTOR *position, s32 param);
extern char *effect_pool_find_free(void);
extern void effect_pool_reset(void);
extern void effect_pool_sweep(void);
extern void effect_update_dispatch(void);
extern void magic_cast(void);

#endif
