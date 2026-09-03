#ifndef KF_GAME_PLAYER_H
#define KF_GAME_PLAYER_H

/*
 * Player state and movement prototypes.
 *
 * Generated during extern-crutch removal: declarations that were duplicated
 * as `extern` across src/game/*.c now live here once. DAT_/func_ spellings
 * remain unresolved WIP identities. Byte-neutral: a declaration never changes
 * codegen.
 */

#include <kf/semantic_types.h>

extern void player_add_experience(s16 amount);
extern void player_adjust_hp(s32 delta);
extern void player_adjust_mp(s32 delta);
extern void player_apply_damage( u16 component0, u16 component1, u16 component2, u16 status_effect_flags, u16 component3, u16 component4, u16 scale_q12, u16 multiplier_tenths);
extern void player_begin_weapon_attack(void);
extern s32 player_calculate_damage_component(s32 base_power, s32 defense, s32 attack);
extern void player_clear_motion(void);
extern void player_death_apply_visual_fade(const MATRIX *color_from, s32 blend);
extern void player_death_begin(void);
extern void player_death_restart(void);
extern void player_death_update(void);
extern void player_death_update_reverse_fade(void);
extern s32 player_distance_to_point( s32 point_x, s32 point_y, s32 point_z, s32 max_distance, s32 point_height);
extern s32 player_distance_to_point_in_cone( const struct KfVec3i *point, s16 facing, s32 max_distance, s32 angle_tolerance);
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
extern void player_update(void);
extern void player_update_transform_snapshot(VECTOR *position_out, SVECTOR *rotation_out);
extern void player_update_vertical_motion(void);
extern void player_update_view_bob(void);
extern void player_update_weapon_attack(void);
extern void player_warp_change_floor(s32 floor, u8 variant);
extern void player_warp_same_floor(char variant, s32 cell_x, s32 cell_z);
extern void player_warp_to_floor_entry(void);

#endif
