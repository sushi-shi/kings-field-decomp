#ifndef KF_GAME_MAP_H
#define KF_GAME_MAP_H

/*
 * Map, floor, collision and camera prototypes.
 *
 * Generated during extern-crutch removal: declarations that were duplicated
 * as `extern` across src/game/*.c now live here once. DAT_/func_ spellings
 * remain unresolved WIP identities. Byte-neutral: a declaration never changes
 * codegen.
 */

#include <kf/semantic_types.h>

extern void camera_path_begin(KfCameraPathState *path, const KfCameraPathPoint *points);
extern void camera_path_compute_segment(KfCameraPathState *path);
extern void camera_path_step(KfCameraPathState *path, s32 y_offset);
extern void collision_adjust_cell_occupancy(u16 cell_x, u16 cell_z, s32 delta);
extern u32 collision_query_world( s32 point_x, s32 point_y, s32 point_z, s32 radius, s32 height, u32 flags);
extern void map_apply_copy_region(u8 region_id);
extern void map_event_advance_rotation_blocking(KfMapEvent *event, u16 target, s16 step);
extern s32 map_event_distance_to_point( const KfMapEvent *event, s32 point_x, s32 point_z, s32 max_distance);
extern s32 map_event_pool_find_overlap(s32 point_x, s32 point_z, s32 radius_padding);
extern KfMapEvent *map_event_pool_find_target_in_cone( const struct KfVec3i *origin, s16 facing, s32 max_distance, s32 angle_tolerance, s32 *distance_out);
extern void map_event_pool_load(const KfMapEventDefinition *definitions);
extern void map_event_refresh_image_for_progress(KfMapEvent *event);
extern void map_event_set_current(KfMapEvent *event);
extern s32 map_floor_height_at_position(const VECTOR *position);
extern s32 map_floor_height_for_cell_position( u16 cell_index, s32 point_x, s32 point_z);
extern void map_load_floor(void);
extern void map_object_definitions_load(const KfMapObjectDefinition *definitions);
extern s32 map_object_distance_to_point( const KfMapObject *object, s32 point_x, s32 point_z, s32 max_distance);
extern KfMapObject *map_object_effect_pool_acquire(u16 first_index, u16 count, u16 sequence);
extern void map_object_mark_collision_edge(const KfMapObject *object, u8 value, u16 yaw);
extern void map_object_pool_clear(void);
extern void map_object_pool_clear_link(u8 link_id);
extern s32 map_object_pool_find_interaction_from(
    s32 start_index, s32 point_x, s32 point_z, s32 radius_padding);
extern s32 map_object_pool_find_near_point(s32 point_x, s32 point_z, s32 radius_padding);
extern void map_object_pool_load(const KfMapObjectPlacement *placements);
extern void map_object_pool_trigger_link(u8 link_id);
extern void map_object_pool_update(void);
extern s32 map_object_probe_forward(const KfMapObject *object, u16 yaw);
extern void map_object_spawn_actor_debris(u16 source, const struct KfVec3i *position, s32 y_offset);
extern void map_object_spawn_effect(u8 kind, u8 object_id, const struct KfVec3i *position, s32 y_offset);
extern void map_object_start_action_if_idle(KfMapObject *object, u8 action);
extern const u32 *map_resource_copy_words( u32 *destination, const u32 *source, u32 word_count);
extern void *map_resource_load_file(const char *filename);
extern void map_resource_path_set_floor(s32 floor);
extern void map_resources_load(s32 floor, s32 map_variant);
extern void map_variant_assets_load(void);

#endif
