#ifndef KF_GAME_MAP_H
#define KF_GAME_MAP_H

/* Map, floor, map-object, event, and camera layouts and operations. */

#include <kf/game_types.h>
#include <kf/floor.h>
#include <kf/enum.h>
#include <kf/notify_types.h>
#include <kf/psyq.h>
#include <kf/game_math.h>

struct KfPoolRecord;

/* Shared forward probe and door-facing policy for interaction and item use. */
enum {
    MAP_INTERACTION_PROBE_DISTANCE = 1000,
    MAP_INTERACTION_RADIUS_PADDING = 800,
    MAP_DOOR_FACING_TOLERANCE = KF_ANGLE_FULL_TURN / 12
};

enum {
    KF_MAP_OBJECT_DEFINITION_COUNT = 160,
    KF_MAP_RESOURCE_PATH_BYTES = 12,
    KF_MAP_OBJECT_CAPACITY = 190,
    KF_MAP_CONTAINER_ITEM_COUNT = 4,
    KF_MAP_OBJECT_EFFECT_FIRST = 160,
    KF_MAP_EVENT_CAPACITY = 8
};

/* Each saved floor slot starts with script bytes before its runtime records. */
enum {
    KF_MAP_SAVED_FLOOR_COUNT = 5,
    KF_MAP_SAVED_FLOOR_BYTES = 1700,
    KF_MAP_SAVED_RECORDS_OFFSET = 10,
    KF_MAP_SAVED_RECORD_BYTES = 1690,
    KF_MAP_SAVED_WORLD_WORDS = 2125,
    KF_MAP_SAVED_YAW_SHIFT = 4,
    KF_MAP_FLOOR3_REQUIRED_REVEALS = 4
};

KF_ENUM_BEGIN(KfMapScriptFlag, u8)
    KF_MAP_SCRIPT_UNSET = 0,
    KF_MAP_SCRIPT_SET = 1
KF_ENUM_END(KfMapScriptFlag)

KF_ENUM_BEGIN(KfMapAreaTriggerStage, u8)
    KF_MAP_TRIGGER_AWAIT_ENTRY = 0,
    KF_MAP_TRIGGER_AWAIT_EXIT = 1,
    KF_MAP_TRIGGER_COMPLETE = 2
KF_ENUM_END(KfMapAreaTriggerStage)

typedef struct KfMapFloor1Script {
    KfMapAreaTriggerStage object_removal_stage;
    KfMapAreaTriggerStage actor_activation_stage;
    KfMapScriptFlag passage_opened;
    KfMapScriptFlag revival_enabled;
} KfMapFloor1Script;

typedef struct KfMapFloor3Script {
    u8 revealed_piece_count;
} KfMapFloor3Script;

typedef struct KfMapFloor5Script {
    KfMapScriptFlag character_arrived;
    KfMapScriptFlag weapon_transformed;
    KfMapScriptFlag boss_encounter_started;
    KfMapScriptFlag boss_defeat;
} KfMapFloor5Script;

/* The floor selects the prefix interpretation; other bytes stay serialized. */
typedef union KfMapFloorScript {
    u8 bytes[KF_MAP_SAVED_RECORDS_OFFSET];
    KfMapFloor1Script floor1;
    KfMapFloor3Script floor3;
    KfMapFloor5Script floor5;
} KfMapFloorScript;

typedef struct KfMapSavedFloor {
    KfMapFloorScript script;
    u8 records[KF_MAP_SAVED_RECORD_BYTES];
} KfMapSavedFloor;

/* Save I/O copies aligned words; scripts address typed bytes within slots. */
typedef union KfMapSavedWorld {
    u32 words[KF_MAP_SAVED_WORLD_WORDS];
    KfMapSavedFloor floors[KF_MAP_SAVED_FLOOR_COUNT];
} KfMapSavedWorld;

/* Definition behavior and running action are separate byte domains. */
KF_ENUM_BEGIN(KfMapObjectBehavior, u8)
    KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR = 0,
    KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR_PARTNER = 1,
    KF_MAP_OBJECT_BEHAVIOR_LIFT_DOOR = 2,
    KF_MAP_OBJECT_BEHAVIOR_HINGED_CONTAINER = 8,
    KF_MAP_OBJECT_BEHAVIOR_ITEM_CONTAINER = 9,
    KF_MAP_OBJECT_BEHAVIOR_COPY_REGION = 10,
    KF_MAP_OBJECT_BEHAVIOR_RESTORE_POINT = 11,
    KF_MAP_OBJECT_BEHAVIOR_REVEAL_MAP_PIECE = 12,
    KF_MAP_OBJECT_BEHAVIOR_SCREEN_IMAGE = 13,
    KF_MAP_OBJECT_BEHAVIOR_SAVE_POINT = 14,
    KF_MAP_OBJECT_BEHAVIOR_ITEM_PICKUP = 64,
    KF_MAP_OBJECT_BEHAVIOR_GOLD_PICKUP = 65,
    KF_MAP_OBJECT_BEHAVIOR_EFFECT_SWITCH = 83,
    KF_MAP_OBJECT_BEHAVIOR_NONE = 255,
    KF_MAP_OBJECT_BEHAVIOR_HINGED_DOOR_END = 2,
    KF_MAP_OBJECT_BEHAVIOR_LINK_TRIGGER_END = 8,
    KF_MAP_OBJECT_BEHAVIOR_LINK_CLEAR_LAST = 8
KF_ENUM_END(KfMapObjectBehavior)

KF_ENUM_BEGIN(KfMapObjectAction, u8)
    KF_MAP_OBJECT_ACTION_SWING_DOOR = 0,
    KF_MAP_OBJECT_ACTION_SWING_DOOR_PARTNER = 1,
    KF_MAP_OBJECT_ACTION_LIFT_DOOR = 2,
    KF_MAP_OBJECT_ACTION_COPY_REGION = 10,
    KF_MAP_OBJECT_ACTION_ENABLE_RESTORE_POINT = 11,
    KF_MAP_OBJECT_ACTION_REVEAL_MAP_PIECE = 12,
    KF_MAP_OBJECT_ACTION_PROJECTILE_EMITTER = 80,
    KF_MAP_OBJECT_ACTION_RELEASE_ORBIT_OR_SHORT_SWING = 81,
    KF_MAP_OBJECT_ACTION_RELEASE_LONG_SWING = 82,
    KF_MAP_OBJECT_ACTION_EFFECT_SWITCH = 83,
    KF_MAP_OBJECT_ACTION_FALL_AND_TIP = 96,
    KF_MAP_OBJECT_ACTION_FALL_AND_SPIN = 97,
    KF_MAP_OBJECT_ACTION_BOUNCE = 98,
    KF_MAP_OBJECT_ACTION_IDLE = 255
KF_ENUM_END(KfMapObjectAction)

/* Door/link dispatch reuses the definition's encoded value as an action. */
#if KF_MODERN_TYPES
constexpr KfMapObjectAction map_object_action_from_behavior(KfMapObjectBehavior behavior)
{
    return KF_ENUM_DECODE(KfMapObjectAction, KF_ENUM_ENCODE(u8, behavior));
}
#else
#define map_object_action_from_behavior(behavior) ((KfMapObjectAction)(behavior))
#endif

/* Authored groups selected by weapon and boss progress. */
enum {
    KF_MAP_LINK_BOSS_EMITTERS = 13,
    KF_MAP_LINK_WEAPON_TRANSFORM_DOORS = 51,
    KF_MAP_LINK_FLOOR5_SWORD_DOOR = 52
};

KF_ENUM_BEGIN(KfMapCopyRegionId, u8)
    KF_MAP_COPY_FLOOR1_GRAVESTONE = 0,
    KF_MAP_COPY_FLOOR1_PASSAGE = 1,
    KF_MAP_COPY_FLOOR3_REVEAL_FIRST = 2,
    KF_MAP_COPY_FLOOR3_REVEAL_SECOND = 3,
    KF_MAP_COPY_FLOOR5_BOSS_ENCOUNTER = 4,
    KF_MAP_COPY_REGION_NONE = 255
KF_ENUM_END(KfMapCopyRegionId)

/* Link IDs 128..254 permit repeated switch/door activation; 255 is absent. */
enum {
    KF_MAP_COPY_REGION_COUNT = 5,
    KF_MAP_LINK_NONE = 255,
    KF_MAP_OBJECT_PARAMETER_NONE = 255,
    KF_MAP_LINK_REUSABLE_FIRST = 128,
    KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY = 10,
    KF_MAP_OBJECT_GOLD_DROP_FIRST = 160,
    KF_MAP_OBJECT_DEFINITION_DROP_FIRST = 170,
    KF_MAP_OBJECT_PLACEMENT_DROP_FIRST = 180,
    KF_MAP_OBJECT_DROP_FROM_PLACEMENT = 0,
    KF_MAP_OBJECT_DROP_FROM_DEFINITION = 1,
    KF_MAP_OBJECT_SWITCH_READY = 0,
    KF_MAP_OBJECT_SWITCH_FORWARD = 1,
    KF_MAP_OBJECT_SWITCH_DISABLED = 2,
    KF_MAP_OBJECT_SWITCH_REVERSE = 3
};

/* GAME model/definition IDs; OPEN's encoded placements use another table. */
KF_ENUM_BEGIN(KfMapObjectId, u8)
    KF_MAP_OBJECT_DRAGON_SWORD = 10,
    KF_MAP_OBJECT_MOONLIGHT_SWORD = 11,
    KF_MAP_OBJECT_GOLD_COIN = 39,
    KF_MAP_OBJECT_DRAGON_CHALICE = 56,
    KF_MAP_OBJECT_WATER_SEAL_STONE = 63,
    KF_MAP_OBJECT_EARTH_SEAL_STONE = 64,
    KF_MAP_OBJECT_FIRE_SEAL_STONE = 68,
    KF_MAP_OBJECT_WIND_SEAL_STONE = 69,
    KF_MAP_OBJECT_BEVELED_WOODEN_LID = 81,
    KF_MAP_OBJECT_FLAT_WOODEN_LID = 83,
    KF_MAP_OBJECT_STONE_CONTAINER_LID = 85,
    KF_MAP_OBJECT_GRAVESTONE = 89,
    KF_MAP_OBJECT_BROKEN_STONE_CROSS = 92,
    KF_MAP_OBJECT_DROP_DISABLED = 99,
    KF_MAP_OBJECT_DRY_FOUNTAIN = 111,
    KF_MAP_OBJECT_BOSS_PROJECTILE_EMITTER = 115,
    KF_MAP_OBJECT_LIFTING_GATE = 117,
    KF_MAP_OBJECT_PORTCULLIS = 118,
    KF_MAP_OBJECT_HINGED_DOOR = 119,
    KF_MAP_OBJECT_HINGED_DOOR_PARTNER = 120,
    KF_MAP_OBJECT_TALL_HINGED_DOOR = 121,
    KF_MAP_OBJECT_TALL_HINGED_DOOR_PARTNER = 122,
    KF_MAP_OBJECT_FILLED_FOUNTAIN = 123,
    KF_MAP_OBJECT_FIRE_BALL_EMITTER = 124,
    KF_MAP_OBJECT_WIND_CUTTER_EMITTER = 125,
    KF_MAP_OBJECT_SIGNBOARD = 130,
    KF_MAP_OBJECT_INSCRIPTION_PANEL = 131,
    KF_MAP_OBJECT_EFFECT_SWITCH = 135,
    KF_MAP_OBJECT_ORBITING_PROJECTILE = 136,
    KF_MAP_OBJECT_PROJECTILE_EMITTER = 137,
    KF_MAP_OBJECT_SHORT_SWING = 138,
    KF_MAP_OBJECT_LONG_SWING = 139,
    KF_MAP_OBJECT_FREE = 255,
    /* Exclusive rendering and drop-animation boundaries. */
    KF_MAP_OBJECT_RENDER_ID_END = 133,
    KF_MAP_DROP_TIP_ID_END = 43,
    KF_MAP_DROP_SPIN_ID_END = 48,
    KF_MAP_DROP_BOUNCE_ID_END = 65
KF_ENUM_END(KfMapObjectId)

/* Positive Y hides the piece; five settling updates undo the reveal overshoot. */
enum {
    KF_MAP_OBJECT_REVEAL_DEPTH = 10000,
    KF_MAP_OBJECT_REVEAL_SETTLE_STEP = 40
};

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


typedef union KfMapObjectSpawn {
    u16 sequence;
    u8 effect_id;
} KfMapObjectSpawn;

typedef struct KfMapObjectLinkFields {
    u8 link_id;
    u8 action_parameter;
    KfMapObjectSpawn spawn;
    s16 vertical_velocity;
    KfNotificationId linked_notification;
    KfNotificationId default_notification;
} KfMapObjectLinkFields;

typedef struct KfMapObjectHingedContainer {
    u8 link_id;
    u8 item_ids[KF_MAP_CONTAINER_ITEM_COUNT];
} KfMapObjectHingedContainer;

/* Placements copy two words; saved floors preserve all eight bytes. */
typedef union KfMapObjectLink {
    KfMapObjectLinkFields fields;
    u16 gold_amount;
    KfMapObjectHingedContainer hinged_container;
    u8 item_ids[KF_MAP_CONTAINER_ITEM_COUNT];
    u32 words[2];
    u8 bytes[8];
} KfMapObjectLink;

typedef char check_map_object_spawn_size[sizeof(KfMapObjectSpawn) == 2 ? 1 : -1];
typedef char check_map_object_link_fields_size[sizeof(KfMapObjectLinkFields) == 8 ? 1 : -1];
typedef char check_map_object_hinged_container_size[
    sizeof(KfMapObjectHingedContainer) == 5 ? 1 : -1];
#define KF_MAP_LINK_OFFSET(field, offset) \
    typedef char check_map_link_##field[ \
        (unsigned long)&((KfMapObjectLinkFields *)0)->field == (offset) ? 1 : -1]
KF_MAP_LINK_OFFSET(link_id, 0);
KF_MAP_LINK_OFFSET(action_parameter, 1);
KF_MAP_LINK_OFFSET(spawn, 2);
KF_MAP_LINK_OFFSET(vertical_velocity, 4);
KF_MAP_LINK_OFFSET(linked_notification, 6);
KF_MAP_LINK_OFFSET(default_notification, 7);
#undef KF_MAP_LINK_OFFSET

/* Encoded model byte is decoded against the consuming image's model table. */
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
    KfMapObjectBehavior behavior_type;
    u8 unknown_01;
    u16 collision_radius;
    u16 interaction_radius;
    u8 unknown_06[2];
} KfMapObjectDefinition;

typedef struct KfMapObject {
    KfMapObjectId object_id;
    u8 unknown_01;
    u16 cell_x;
    u16 cell_z;
    u8 unknown_06[2];
    VECTOR position;
    KfRotation rotation;
    KfMapObjectLink link;
    KfMapObjectAction action;
    u8 unknown_29;
    u16 action_timer;
} KfMapObject;

/* The pool reset and placement loader access the whole link as two words. */
typedef char check_map_object_link_offset[
    (unsigned long)&((KfMapObject *)0)->link == 0x20 ? 1 : -1];
typedef char check_map_placement_link_offset[
    (unsigned long)&((KfMapObjectPlacement *)0)->link == 0x0c ? 1 : -1];

/*
 * Cutscene camera paths use 0x1c-byte serialized points and a 0x64-byte
 * runtime interpolator. The fourth vector lane and two trailing halfwords
 * remain unresolved.
 */
enum {
    KF_CAMERA_PATH_END_X = -1,
    KF_CAMERA_PATH_FINISHED = -1
};

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

/* Role identities established by interaction and placement evidence. */
KF_ENUM_BEGIN(KfCharacterId, u8)
    KF_CHARACTER_KEY_OF_THE_DEAD_EXCHANGE = 3,
    KF_CHARACTER_HARP_EXCHANGE = 7,
    KF_CHARACTER_HEALING_EXCHANGE = 8,
    KF_CHARACTER_FLOOR3_DOOR_UNLOCKER = 12
KF_ENUM_END(KfCharacterId)

KF_ENUM_BEGIN(KfMapEventState, u8)
    KF_MAP_EVENT_INACTIVE = 0,
    KF_MAP_EVENT_ACTIVE = 1,
    KF_MAP_EVENT_DISABLED = 3,
    KF_MAP_EVENT_FREE = 255
KF_ENUM_END(KfMapEventState)

KF_ENUM_BEGIN(KfMapEventBehavior, u8)
    KF_MAP_EVENT_BEHAVIOR_SHOP = 0,
    KF_MAP_EVENT_BEHAVIOR_WANDER = 1,
    KF_MAP_EVENT_BEHAVIOR_ANIMATION_LOOP = 2
KF_ENUM_END(KfMapEventBehavior)

KF_ENUM_BEGIN(KfMapEventAnimationClip, u8)
    KF_MAP_EVENT_CLIP_BASE = 0,
    KF_MAP_EVENT_CLIP_INTERACTION = 1
KF_ENUM_END(KfMapEventAnimationClip)

KF_ENUM_BEGIN(KfMapEventCollisionTurn, u8)
    KF_MAP_EVENT_COLLISION_TURN_NONE = 0,
    KF_MAP_EVENT_COLLISION_TURN_PENDING = 1
KF_ENUM_END(KfMapEventCollisionTurn)

enum {
    KF_DIALOGUE_STAGE_COUNT = 5,
    KF_DIALOGUE_FIRST_STAGE = 1,
    KF_DIALOGUE_FIRST_PAGE = 1,
    KF_DIALOGUE_GATE_RELOAD = 3,
    KF_DIALOGUE_PAGE_DELAY_TICKS = 40,
    KF_MAP_EVENT_ANIMATION_PHASE_MASK = 4095,
    KF_MAP_EVENT_ANIMATION_TALK_POSE = 2048,
    KF_MAP_EVENT_ANIMATION_WANDER_STEP = 110,
    KF_MAP_EVENT_ANIMATION_LOOP_STEP = 200,
    KF_MAP_EVENT_ANIMATION_TALK_STEP = 200,
    KF_MAP_EVENT_ANIMATION_FINISH_STEP = 400
};

/* Last accessible page for each one-based dialogue stage; copied as a block. */
typedef struct KfDialoguePageLimits {
    u8 last_page[KF_DIALOGUE_STAGE_COUNT];
} KfDialoguePageLimits;

typedef struct KfMapEventDefinition {
    KfMapEventState state;
    KfCharacterId character_id;
    u8 model_index;
    u8 cell_z;
    u8 cell_x;
    KfDialoguePageLimits dialogue_pages;
    u8 dialogue_stage_limit;
    u8 unknown_0b;
    u8 unknown_0c;
    KfMapEventBehavior behavior;
    s16 position_z_offset;
    s16 position_x_offset;
    u16 initial_rotation;
    u16 radius;
    u16 unknown_16;
} KfMapEventDefinition;

typedef struct KfMapEvent {
    KfMapEventState state;
    KfCharacterId character_id;
    u8 model_index;
    KfDialoguePageLimits dialogue_pages;
    u8 dialogue_stage_limit;
    u8 dialogue_stage;
    u8 dialogue_page;
    u8 dialogue_page_delay;
    u8 unknown_0c;
    u8 unknown_0d;
    KfMapEventBehavior behavior;
    KfMapEventAnimationClip animation_clip;
    KfMapEventCollisionTurn collision_turn_pending;
    u8 unknown_11;
    u16 animation_phase;
    s32 position_x;
    s32 position_z;
    u16 cell_x;
    u16 cell_z;
    u16 radius;
    u16 unknown_22;
    VECTOR reference_position;
    SVECTOR rotation;
    struct KfPoolRecord *animation_cache;
    s16 rotation_target;
    u16 unknown_42;
} KfMapEvent;

typedef char check_map_object_size[sizeof(KfMapObject) == 0x2c ? 1 : -1];
typedef char check_map_event_size[sizeof(KfMapEvent) == 0x44 ? 1 : -1];
#define KF_MAP_TRANSFORM_OFFSET_CHECK(type, label, member, offset) \
    typedef char check_map_transform_##label[ \
        ((unsigned long)&((type *)0)->member == (offset)) ? 1 : -1]
KF_MAP_TRANSFORM_OFFSET_CHECK(KfMapObject, object_position, position, 0x08);
KF_MAP_TRANSFORM_OFFSET_CHECK(KfMapObject, object_position_pad, position.pad, 0x14);
KF_MAP_TRANSFORM_OFFSET_CHECK(KfMapObject, object_rotation, rotation, 0x18);
KF_MAP_TRANSFORM_OFFSET_CHECK(KfMapObject, object_rotation_pad, rotation.vector.pad, 0x1e);
KF_MAP_TRANSFORM_OFFSET_CHECK(KfMapEvent, event_position, reference_position, 0x24);
KF_MAP_TRANSFORM_OFFSET_CHECK(KfMapEvent, event_position_pad, reference_position.pad, 0x30);
KF_MAP_TRANSFORM_OFFSET_CHECK(KfMapEvent, event_rotation, rotation, 0x34);
KF_MAP_TRANSFORM_OFFSET_CHECK(KfMapEvent, event_rotation_pad, rotation.pad, 0x3a);
KF_MAP_TRANSFORM_OFFSET_CHECK(KfMapEvent, event_animation_cache, animation_cache, 0x3c);
#undef KF_MAP_TRANSFORM_OFFSET_CHECK

/* Definitions and the live pool form one base-register-relative aggregate. */
typedef struct KfMapObjectState {
    KfMapObjectDefinition definitions[KF_MAP_OBJECT_DEFINITION_COUNT];
    KfMapObject objects[KF_MAP_OBJECT_CAPACITY];
} KfMapObjectState;

/* Cleared as 0x2360 bytes; the final 0x2134 bytes are copied by save I/O. */
typedef struct KfMapRuntimeState {
    KfMapEvent events[KF_MAP_EVENT_CAPACITY];
    KfMapEvent *current_event;
    u8 *variant_asset_buffer;
    u16 dialogue_advance_gate;
    u16 ambient_script_countdown;
    KfMapSavedWorld world_state;
} KfMapRuntimeState;

extern KfMapCopyRegion map_copy_regions[KF_MAP_COPY_REGION_COUNT];
extern KfMapRuntimeState map_runtime_state;
/* Member spellings used by consumers, not independently owned globals. */
#define map_event_pool (map_runtime_state.events)
#define current_map_event (map_runtime_state.current_event)
#define map_variant_asset_buffer (map_runtime_state.variant_asset_buffer)
#define map_dialogue_advance_gate (map_runtime_state.dialogue_advance_gate)
#define map_ambient_script_countdown (map_runtime_state.ambient_script_countdown)
#define map_world_state_base (map_runtime_state.world_state.words[0])
#define MAP_WORLD_STATE_BYTES ((u8 *)map_runtime_state.world_state.words)
#define map_floor1_script (map_runtime_state.world_state.floors[0].script.floor1)
#define map_floor3_script (map_runtime_state.world_state.floors[2].script.floor3)
#define map_floor5_script (map_runtime_state.world_state.floors[4].script.floor5)
#define boss_defeat_complete (map_floor5_script.boss_defeat)
extern KfMapObjectState map_object_state;
extern u16 map_object_effect_sequence_160;
extern u16 map_object_effect_sequence_170;
extern u16 map_object_effect_sequence_180;
extern char map_resource_path[KF_MAP_RESOURCE_PATH_BYTES];

extern void camera_path_begin(KfCameraPathState *path, const KfCameraPathPoint *points);
extern void camera_path_compute_segment(KfCameraPathState *path);
extern void camera_path_step(KfCameraPathState *path, s32 y_offset);
extern void map_apply_copy_region(KfMapCopyRegionId region_id);
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
extern void map_event_advance_animation_blocking(KfMapEvent *event, u16 target, s16 step);
extern s32 map_event_distance_to_point( const KfMapEvent *event, s32 point_x, s32 point_z, s32 max_distance);
extern s32 map_event_pool_find_overlap(s32 point_x, s32 point_z, s32 radius_padding);
extern KfMapEvent *map_event_pool_find_target_in_cone( const VECTOR *origin, s16 facing, s32 max_distance, s32 angle_tolerance, s32 *distance_out);
extern void map_event_pool_load(const KfMapEventDefinition *definitions);
extern void map_event_pool_update(void);
extern void map_event_refresh_dialogue_stage(KfMapEvent *event);
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
extern void map_object_spawn_actor_debris(u16 source, const VECTOR *position, s32 y_offset);
extern void map_object_spawn_effect(u8 kind, KfMapObjectId object_id, const VECTOR *position, s32 y_offset);
extern void map_object_start_action_if_idle(KfMapObject *object, KfMapObjectAction action);
extern const u32 *map_resource_copy_words( u32 *destination, const u32 *source, u32 word_count);
extern void *map_resource_load_file(const char *filename);
extern void map_resource_path_set_floor(KfFloorId floor);
extern void map_resources_load(KfFloorId floor, s32 map_variant);
extern void map_unload_floor(void);
extern void map_variant_assets_load(void);
extern void map_world_state_persist(void);

#endif
