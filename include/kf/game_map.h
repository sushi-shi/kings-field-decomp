#ifndef KF_GAME_MAP_H
#define KF_GAME_MAP_H

/* Map, floor, map-object, event, and camera layouts and operations. */

#include <kf/game_types.h>
#include <kf/psyq.h>
#include <kf/game_math.h>

typedef struct KfMapCell {
    u8 z;
    u8 x;
} KfMapCell;

typedef struct KfMapCopyRegion {
    u8 source_x;
    u8 source_z;
    u8 destination_x;
    u8 destination_z;
    u8 width;
    u8 height;
} KfMapCopyRegion;

typedef struct KfMapObjectLink {
    u8 link_id;
    u8 action_parameter;
    u16 spawn_sequence;
    s16 vertical_velocity;
    u8 unknown_06[2];
} KfMapObjectLink;

typedef struct KfMapObjectPlacement {
    u8 object_id;
    u8 unknown_01;
    u8 tile_z;
    u8 tile_x;
    u16 yaw;
    s16 local_z;
    s16 local_x;
    s16 local_y;
    KfMapObjectLink link;
} KfMapObjectPlacement;

typedef struct KfMapObjectDefinition {
    u8 behavior_type;
    u8 unknown_01;
    u16 collision_radius;
    u16 interaction_radius;
    u8 unknown_06[2];
} KfMapObjectDefinition;

typedef struct KfMapObject {
    u8 object_id;
    u8 unknown_01;
    u16 cell_x;
    u16 cell_z;
    u8 unknown_06[2];
    s32 position_x;
    s32 position_y;
    s32 position_z;
    u8 unknown_14[4];
    struct KfEulerAngles rotation;
    u16 unknown_1e;
    KfMapObjectLink link;
    u8 action;
    u8 unknown_29;
    u16 action_timer;
} KfMapObject;

/*
 * Cutscene camera paths use 0x1c-byte serialized points and a 0x64-byte
 * runtime interpolator. The fourth vector lane and two trailing halfwords
 * remain unresolved.
 */
typedef struct KfCameraPathPoint {
    VECTOR position;
    SVECTOR rotation;
    s16 speed;
    s16 unknown_1a;
} KfCameraPathPoint;

typedef struct KfCameraPathState {
    const KfCameraPathPoint *points;
    VECTOR position;
    SVECTOR rotation;
    VECTOR position_fixed;
    VECTOR rotation_fixed;
    VECTOR position_delta;
    VECTOR rotation_delta;
    s16 point_index;
    s16 unknown_5e;
    s32 frames_remaining;
} KfCameraPathState;

/* Five opaque bytes copied as one block from a map-event definition. */
typedef struct KfMapEventTag {
    u8 bytes[5];
} KfMapEventTag;

typedef struct KfMapEventDefinition {
    u8 state;
    u8 kind;
    u8 variant;
    u8 cell_z;
    u8 cell_x;
    KfMapEventTag tag;
    u8 image_limit;
    u8 unknown_0b;
    u8 unknown_0c;
    u8 unknown_0d;
    s16 position_z_offset;
    s16 position_x_offset;
    u16 initial_rotation;
    u16 radius;
    u16 unknown_16;
} KfMapEventDefinition;

typedef struct KfMapEvent {
    u8 state;
    u8 kind;
    u8 variant;
    KfMapEventTag tag;
    u8 image_limit;
    u8 image_index;
    u8 image_dirty;
    u8 image_delay;
    u8 unknown_0c;
    u8 unknown_0d;
    u8 unknown_0e;
    u8 unknown_0f;
    u8 unknown_10;
    u8 unknown_11;
    u16 rotation_phase;
    s32 position_x;
    s32 position_z;
    u16 cell_x;
    u16 cell_z;
    u16 radius;
    u16 unknown_22;
    s32 reference_x;
    s32 position_y;
    s32 reference_z;
    u8 unknown_30[4];
    u16 unknown_34;
    s16 rotation;
    u16 unknown_38;
    u8 unknown_3a[6];
    s16 rotation_target;
    u16 unknown_42;
} KfMapEvent;

/* Definitions and the live pool form one base-register-relative aggregate. */
typedef struct KfMapObjectState {
    KfMapObjectDefinition definitions[160];
    KfMapObject objects[190];
} KfMapObjectState;

extern KfMapCopyRegion map_copy_regions[4];
extern KfMapEvent map_event_pool[8];
extern KfMapEvent *current_map_event;
extern u16 map_event_animation_gate;
extern u16 map_ambient_script_countdown;
extern KfMapObjectState map_object_state;
extern u16 map_object_effect_sequence_160;
extern u16 map_object_effect_sequence_170;
extern u16 map_object_effect_sequence_180;
extern char map_resource_path[12];
extern u8 *map_variant_asset_buffer;
extern u32 map_world_state_base;
#define MAP_WORLD_STATE_BYTES ((u8 *)&map_world_state_base)

extern void camera_path_begin(KfCameraPathState *path, const KfCameraPathPoint *points);
extern void camera_path_compute_segment(KfCameraPathState *path);
extern void camera_path_step(KfCameraPathState *path, s32 y_offset);
extern void map_apply_copy_region(u8 region_id);
extern void map_ambient_script_floor1(void);
extern void map_ambient_script_floor2(void);
extern void map_ambient_script_floor3(void);
extern void map_ambient_script_floor4(void);
extern void map_ambient_script_floor5(void);
extern void map_action_script_floor1(void);
extern void map_action_script_floor2(void);
extern void map_action_script_floor3(void);
extern void map_action_script_floor4(void);
extern void map_action_script_floor5(void);
extern void map_event_advance_rotation_blocking(KfMapEvent *event, u16 target, s16 step);
extern s32 map_event_distance_to_point( const KfMapEvent *event, s32 point_x, s32 point_z, s32 max_distance);
extern s32 map_event_pool_find_overlap(s32 point_x, s32 point_z, s32 radius_padding);
extern KfMapEvent *map_event_pool_find_target_in_cone( const struct KfVec3i *origin, s16 facing, s32 max_distance, s32 angle_tolerance, s32 *distance_out);
extern void map_event_pool_load(const KfMapEventDefinition *definitions);
extern void map_event_pool_update(void);
extern void map_event_refresh_image_for_progress(KfMapEvent *event);
extern void map_event_set_current(KfMapEvent *event);
extern void map_event_timers_reset(void);
extern void map_interaction_dispatch(
    const VECTOR *position, SVECTOR *rotation);
extern void func_800365f8(void);
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
extern void map_unload_floor(void);
extern void map_variant_assets_load(void);
extern void map_world_state_persist(void);

#endif
