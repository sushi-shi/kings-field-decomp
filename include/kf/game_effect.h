#ifndef KF_GAME_EFFECT_H
#define KF_GAME_EFFECT_H

/* Shared projectile launch and actor-target acquisition parameters. */
enum {
    KF_EFFECT_PROJECTILE_DEFAULT_SPEED = 600,
    KF_EFFECT_LIGHTNING_SPEED = 800,
    KF_EFFECT_WIND_CUTTER_SPEED = 800,
    KF_EFFECT_ACTOR_TARGET_MAX_DISTANCE = 20000,
    KF_EFFECT_ACTOR_TARGET_WIDE_CONE = 0x555
};

/*
 * GAME.EXE effect-pool lifecycle, construction, collision, and dispatch.
 * The general constructor has five fixed arguments followed by slots whose
 * meanings depend on KIND; callers must supply only the slots used by that
 * effect kind.
 */

#include <kf/animation.h>
#include <kf/game_math.h>
#include <kf/magic.h>

struct KfPoolRecord;

enum {
    KF_EFFECT_CAPACITY = 48
};

/* Resource identities follow their effect use; billboard IDs may advance by frame. */
KF_ENUM_BEGIN(KfEffectBillboardId, u8)
    KF_EFFECT_BILLBOARD_FIRE_BALL = 0,
    KF_EFFECT_BILLBOARD_FIRE_BALL_FRAME_1 = 1,
    KF_EFFECT_BILLBOARD_FIRE_BALL_FRAME_2 = 2,
    KF_EFFECT_BILLBOARD_FIRE_BALL_FRAME_3 = 3,
    KF_EFFECT_BILLBOARD_FIRE_BALL_FRAME_4 = 4,
    KF_EFFECT_BILLBOARD_WIND_CUTTER = 5,
    KF_EFFECT_BILLBOARD_LIGHTNING_BOLT = 6,
    KF_EFFECT_BILLBOARD_LIGHTNING_BOLT_FRAME_1 = 7,
    KF_EFFECT_BILLBOARD_DARKNESS_PROJECTILE = 8,
    KF_EFFECT_BILLBOARD_SCATTER_PROJECTILE = 9,
    KF_EFFECT_BILLBOARD_CURSE_PROJECTILE = 10,
    KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT = 11,
    KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT_FRAME_1 = 12,
    KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT_FRAME_2 = 13,
    KF_EFFECT_BILLBOARD_GROUND_TRAIL = 14,
    KF_EFFECT_BILLBOARD_GROUND_TRAIL_FRAME_1 = 15,
    KF_EFFECT_BILLBOARD_GROUND_TRAIL_FRAME_2 = 16,
    KF_EFFECT_BILLBOARD_LIGHTNING_BOLT_ALTERNATE = 17,
    KF_EFFECT_BILLBOARD_LIGHTNING_ALTERNATE_FRAME_1 = 18,
    KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT_ALTERNATE = 19,
    KF_EFFECT_BILLBOARD_IMPACT_ALTERNATE_FRAME_1 = 20,
    KF_EFFECT_BILLBOARD_IMPACT_ALTERNATE_FRAME_2 = 21,
    KF_EFFECT_BILLBOARD_NONE = 0xff
KF_ENUM_END(KfEffectBillboardId)
KF_ENUM_COUNTER(KfEffectBillboardId, u8)

enum { KF_EFFECT_BILLBOARD_SPRITE_COUNT = 22 };

KF_ENUM_BEGIN(KfEffectModelId, u8)
    KF_EFFECT_MODEL_LIGHTNING_RADIAL_BLAST = 0,
    KF_EFFECT_MODEL_GROUND_BRANCH = 1,
    KF_EFFECT_MODEL_GROUND_BRANCH_VISUAL = 2,
    KF_EFFECT_MODEL_ACTOR_SPAWNER = 3,
    KF_EFFECT_MODEL_MAP_EMITTER_PROJECTILE = 4,
    KF_EFFECT_MODEL_MAP_SWITCH = 5,
    KF_EFFECT_MODEL_LIGHT_NEEDLE = 6,
    KF_EFFECT_MODEL_SWINGING_HAZARD = 7,
    KF_EFFECT_MODEL_EMERGING_PROJECTILE = 9,
    KF_EFFECT_MODEL_MOONLIGHT_PROJECTILE = 10,
    KF_EFFECT_MODEL_RADIAL_BLAST = 11,
    KF_EFFECT_MODEL_HOMING_PROJECTILE = 12,
    KF_EFFECT_MODEL_WARP_SHIMMER = 13,
    KF_EFFECT_MODEL_PHYSICAL_PROJECTILE = 14,
    KF_EFFECT_MODEL_LIGHTNING_RADIAL_BLAST_ALTERNATE = 15,
    KF_EFFECT_MODEL_HOMING_PROJECTILE_ALTERNATE = 16,
    KF_EFFECT_MODEL_RADIAL_BLAST_ALTERNATE = 17,
    KF_EFFECT_MODEL_NONE = 0xff
KF_ENUM_END(KfEffectModelId)

/* Normalized packed codes retain word arithmetic before the byte API boundary. */
typedef KF_ENUM_PROMOTED(KfEffectKind) KfEffectKindArgument;

/* Low type bits select actors/player after the separate terrain checks.
 * The power bit alone does not imply the actor-damage player-credit class. */
KF_ENUM_BEGIN(KfEffectType, u8)
    KF_EFFECT_TYPE_NONE = 0,
    KF_EFFECT_COLLISION_TARGET_ACTORS = 1,
    KF_EFFECT_COLLISION_TARGET_PLAYER = 2,
    KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER = 3,
    KF_EFFECT_COLLISION_TARGETS_MASK = 3,
    KF_EFFECT_USE_PLAYER_MAGIC = 0x10,
    KF_EFFECT_CLASS_20 = 0x20,
    KF_ACTOR_DAMAGE_CREDIT_PLAYER = 0x10,
    KF_ACTOR_DAMAGE_CREDIT_MASK = 0xf0,
    KF_EFFECT_FLOOR_DEFORM_TYPE = 0xf0,
    KF_EFFECT_SLOT_FREE = 0xff
KF_ENUM_END(KfEffectType)
KF_ENUM_FLAGS(KfEffectType, u8)

/* Other selector values request a fresh actor-cone query, not an actor index. */
KF_ENUM_BEGIN(KfEffectHomingMode, u8)
    KF_EFFECT_HOMING_WANDER = 0xff,
    KF_EFFECT_HOMING_PLAYER = 0xfe
KF_ENUM_END(KfEffectHomingMode)

KF_ENUM_BEGIN(KfEffectGroundBranchRole, u16)
    KF_EFFECT_GROUND_BRANCH_ROOT = 0,
    KF_EFFECT_GROUND_BRANCH_QUARTER_TURN = 1,
    KF_EFFECT_GROUND_BRANCH_THREE_QUARTER_TURN = 2,
    KF_EFFECT_GROUND_BRANCH_LEAF = 0xff
KF_ENUM_END(KfEffectGroundBranchRole)

enum {
    KF_EFFECT_GROUND_BRANCH_TIMER_DONE = 0xff
};

/* Kind-specific phases and update counters share byte 7. Intermediate
 * ages are advanced within their named ranges, with byte wrap at 0xff. */
KF_ENUM_BEGIN(KfEffectPhase, u8)
    KF_EFFECT_PHASE_INIT = 0,
    KF_EFFECT_FLOOR_DEFORM_ADVANCE = 0,
    KF_EFFECT_FLOOR_DEFORM_HOLD = 1,
    KF_EFFECT_FLOOR_DEFORM_REVERSE = 2,
    KF_EFFECT_PROJECTILE_TRAVEL = 0,
    KF_EFFECT_PROJECTILE_IMPACT_FIRST = 1,
    KF_EFFECT_PROJECTILE_DISSIPATE_FIRST = 50,
    KF_EFFECT_PROJECTILE_DISSIPATE_END = 60,
    KF_EFFECT_PROJECTILE_EMERGE_FIRST = 100,
    KF_EFFECT_PROJECTILE_EMERGE_LAST = 119,
    KF_EFFECT_PROJECTILE_FALL = 120,
    KF_EFFECT_PROJECTILE_SHRINK = 121,
    KF_EFFECT_PROJECTILE_LAUNCH_WRAP = 0xff,
    KF_EFFECT_GROUND_TRAIL_WAIT_FOR_PARENT = 0,
    KF_EFFECT_GROUND_TRAIL_SHRINK = 1,
    KF_EFFECT_HAZARD_RUNNING = 0,
    KF_EFFECT_HAZARD_RELEASE_REQUEST = 1,
    KF_EFFECT_HAZARD_RISE_FIRST = 10,
    KF_EFFECT_SHORT_SWING_PHASE_LIMIT = 40,
    KF_EFFECT_LONG_SWING_PHASE_LIMIT = 60,
    KF_EFFECT_ORBIT_PHASE_LIMIT = 40,
    KF_EFFECT_FIRE_BALL_IMPACT_END = 5,
    KF_EFFECT_PROJECTILE_IMPACT_END = 10,
    KF_EFFECT_MOONLIGHT_TRAVEL_LAST = 10,
    KF_EFFECT_MOONLIGHT_TRAIL_EMIT_PHASE = 2,
    KF_EFFECT_MOONLIGHT_IMPACT_FIRST = 20,
    KF_EFFECT_MOONLIGHT_IMPACT_LAST = 23,
    KF_EFFECT_RADIAL_BLAST_PHASE_END = 13,
    KF_EFFECT_HOMING_INITIAL_PHASE_LAST = 4,
    KF_EFFECT_HOMING_TRACKING_PHASE = 20,
    KF_EFFECT_LIGHTNING_IMPACT_PHASE_LAST = 9,
    KF_EFFECT_LIGHTNING_IMPACT_EMIT_FIRST = 3,
    KF_EFFECT_LIGHTNING_IMPACT_EMIT_SECOND = 5,
    KF_EFFECT_LIGHTNING_IMPACT_EMIT_LAST = 7,
    KF_EFFECT_LIGHTNING_BLAST_PHASE_LAST = 7,
    KF_EFFECT_GROUND_BRANCH_GROW_END = 16,
    KF_EFFECT_GROUND_BRANCH_ROOT_HOLD_BASE = 16,
    KF_EFFECT_GROUND_BRANCH_SIDE_HOLD_BASE = 26,
    KF_EFFECT_GROUND_BRANCH_LEAF_HOLD_BASE = 36,
    KF_EFFECT_GROUND_BRANCH_SHRINK_FIRST = 48,
    KF_EFFECT_GROUND_BRANCH_PHASE_END = 64,
    KF_EFFECT_GROUND_VISUAL_SHRINK_FIRST = 4,
    KF_EFFECT_GROUND_VISUAL_PHASE_END = 8,
    KF_EFFECT_ACTOR_SPAWNER_TRAVEL_FIRST = 17,
    KF_EFFECT_ACTOR_SPAWNER_TRAVEL_LAST = 40,
    KF_EFFECT_ACTOR_SPAWNER_WAIT_FIRST = 117,
    KF_EFFECT_ACTOR_SPAWNER_CREATE_PHASE = 132,
    KF_EFFECT_ACTOR_SPAWNER_SHRINK_FIRST = 149,
    KF_EFFECT_ACTOR_SPAWNER_PHASE_END = 165,
KF_ENUM_END(KfEffectPhase)
KF_ENUM_COUNTER(KfEffectPhase, u8)

/* Twenty emergence updates undo the initial downward placement offset. */
enum {
    KF_EFFECT_EMERGE_Y_STEP = 175,
    KF_EFFECT_EMERGE_DEPTH =
        (KF_ENUM_ENCODE(u8, KF_EFFECT_PROJECTILE_FALL) - KF_ENUM_ENCODE(u8, KF_EFFECT_PROJECTILE_EMERGE_FIRST)) * KF_EFFECT_EMERGE_Y_STEP,
    /* X/Z centers are quantized world coordinates, not Q8 fractions. */
    KF_EFFECT_ORBIT_CENTER_SHIFT = 8
};

/* The orbit helper keeps running at RELEASE_REQUEST; only the 3D helper
 * settles its pitch and enters the rise sequence on a zero crossing. */
enum {
    KF_EFFECT_SWING_PROBE_SHORT = 0,
    KF_EFFECT_SWING_PROBE_LONG = 1,
    KF_EFFECT_SWING_PROBE_COUNT = 2
};

typedef struct KfEffectDirectionWords {
    u16 x;
    u16 y;
    u16 z;
    u16 pad;
} KfEffectDirectionWords;

/* Effect kinds use these same eight bytes as a vector or unsigned state. */
typedef union KfEffectDirection {
    SVECTOR vector;
    KfEffectDirectionWords words;
} KfEffectDirection;

/* Each kind selects the interpretation of the shared storage. */
typedef union KfEffectVisualState {
    u16 animation_phase;
    u16 pulse_base_scale;
} KfEffectVisualState;

/* Moonlight projectiles initialize both bytes; their purpose is unresolved. */
typedef struct KfEffectControlBytes {
    u8 low;
    u8 high;
} KfEffectControlBytes;

typedef union KfEffectControl {
    u16 frames_remaining;
    u16 orbit_angle;
    u8 parent_effect_index;
    KfEffectHomingMode target_mode;
    KfEffectControlBytes bytes;
} KfEffectControl;

typedef union KfEffectPropagation {
    u16 generations_remaining;
    KfEffectGroundBranchRole branch;
} KfEffectPropagation;

/* Coordinates wrap modulo 256; the retail loads both steps as unsigned bytes. */
typedef struct KfFloorDeformSegment {
    u8 column;
    u8 row;
    u8 column_step;
    u8 row_step;
    u8 cell_count;
    u8 start_height;
    u8 end_height;
} KfFloorDeformSegment;

/*
 * Effect pool record (60-byte stride, pool in effect_pool_records). The pool
 * constructors establish the field layout: byte 0 is the type tag (0xff marks
 * a free slot), byte 1 the dispatch/magic_records index, byte 6 the owning id,
 * a VECTOR world position at 0x0c, and three SVECTOR-shaped 16-bit triples
 * (rotation, scale, direction) that individual effect kinds reuse. The default
 * scale is 0x1000 (1.0 fixed point); the direction triple is copied from the
 * constructor's SVECTOR argument.
 */
/* The animation-clip tag selects the bank. NONE is 0xff in both views;
 * shared visibility checks use the model view for that common sentinel. */
typedef union KfEffectRenderId {
    KfEffectBillboardId billboard;
    KfEffectModelId model;
} KfEffectRenderId;

typedef struct KfEffectRecord {
    KfEffectType type;    /* 0x00: targets, class and free sentinel */
    KfEffectKind kind;   /* 0x01 */
    KfEffectRenderId base_render_id;   /* 0x02: first sprite/model in the animation */
    KfEffectRenderId render_id;        /* 0x03 */
    KfAnimationClip animation_clip;   /* 0x04: 0xff selects a billboard instead of a model */
    KF_ENUM_STORAGE(KfAudioPlaybackResult, u8) sound_played;
    u8 id;               /* 0x06 */
    KfEffectPhase phase; /* 0x07: kind-specific lifecycle/age */
    KfEffectVisualState visual; /* 0x08: animation phase, or kind-10 pulse scale */
    u16 unknown_0a;      /* 0x0a */
    VECTOR position;     /* 0x0c */
    KfRotation rotation; /* 0x1c */
    u16 scale_x;         /* 0x24 */
    u16 scale_y;         /* 0x26 */
    u16 scale_z;         /* 0x28 */
    u16 unknown_2a;      /* 0x2a */
    KfEffectDirection direction; /* 0x2c */
    struct KfPoolRecord *animation_cache; /* 0x34 */
    KfEffectControl control; /* 0x38: countdown, orbit, parent, or homing selector */
    KfEffectPropagation propagation; /* 0x3a: kind-10 generations or kind-6 branch */
} KfEffectRecord;

/* Startup clears this whole object; selection derives the magic array from
 * the current-record slot by a fixed member offset. */
typedef struct KfEffectState {
    KfMagicTable magic;
    KfEffectRecord records[KF_EFFECT_CAPACITY];
    KfMagicRecord *current_magic;
    KfEffectRecord *current_record;
} KfEffectState;

extern SVECTOR effect_swing_probe_offsets[KF_EFFECT_SWING_PROBE_COUNT];
extern KfEffectState effect_state;
/* Consumer spellings are members, not separately owned globals. */
#define magic_records (effect_state.magic.entries)
#define effect_pool_records (effect_state.records)
#define current_effect_magic_record (effect_state.current_magic)
#define current_effect (effect_state.current_record)

/* The variadic tail is selected by kind. In strict checking, named groups
 * prevent integer/foreign-enum arguments from entering enum-valued slots.
 * Retail expands each group to the same original argument expressions. */
KF_ENUM_BEGIN(KfEffectSoundRequest, s32)
    KF_EFFECT_SOUND_SILENT = 0,
    KF_EFFECT_SOUND_PLAY = 1
KF_ENUM_END(KfEffectSoundRequest)

#if KF_MODERN_TYPES
constexpr KfEffectSoundRequest effect_sound_request(bool requested)
{
    return requested ? KF_EFFECT_SOUND_PLAY : KF_EFFECT_SOUND_SILENT;
}

class KfEffectBranchArguments { public: KfEffectGroundBranchRole role; };
class KfEffectRotationArguments { public: const SVECTOR *rotation; };
class KfEffectRotationSoundArguments {
public:
    const SVECTOR *rotation;
    KfEffectSoundRequest sound;
};
class KfEffectDurationSoundArguments {
public:
    s32 duration;
    KfEffectSoundRequest sound;
};
class KfEffectScatterArguments {
public:
    s32 generations;
    s32 duration;
    s32 scale;
};
class KfEffectSoundArguments { public: KfEffectSoundRequest sound; };
class KfEffectHomingArguments {
public:
    const SVECTOR *rotation;
    KfEffectHomingMode target;
    KfEffectSoundRequest sound;
};
class KfEffectParentArguments { public: s32 parent_index; };

#define KF_EFFECT_ARGS_BRANCH(role) KfEffectBranchArguments{role}
#define KF_EFFECT_ARGS_ROTATION(rotation) KfEffectRotationArguments{rotation}
#define KF_EFFECT_ARGS_ROTATION_SOUND(rotation, sound) \
    KfEffectRotationSoundArguments{rotation, sound}
#define KF_EFFECT_ARGS_DURATION_SOUND(duration, sound) \
    KfEffectDurationSoundArguments{duration, sound}
#define KF_EFFECT_ARGS_SCATTER(generations, duration, scale) \
    KfEffectScatterArguments{generations, duration, scale}
#define KF_EFFECT_ARGS_SOUND(sound) KfEffectSoundArguments{sound}
#define KF_EFFECT_ARGS_HOMING(rotation, target, sound) \
    KfEffectHomingArguments{rotation, target, sound}
#define KF_EFFECT_ARGS_PARENT(parent) KfEffectParentArguments{parent}
#else
#define effect_sound_request(requested) (requested)
#define KF_EFFECT_ARGS_BRANCH(role) (role)
#define KF_EFFECT_ARGS_ROTATION(rotation) (rotation)
#define KF_EFFECT_ARGS_ROTATION_SOUND(rotation, sound) (rotation), (sound)
#define KF_EFFECT_ARGS_DURATION_SOUND(duration, sound) (duration), (sound)
#define KF_EFFECT_ARGS_SCATTER(generations, duration, scale) \
    (generations), (duration), (scale)
#define KF_EFFECT_ARGS_SOUND(sound) (sound)
#define KF_EFFECT_ARGS_HOMING(rotation, target, sound) (rotation), (target), (sound)
#define KF_EFFECT_ARGS_PARENT(parent) (parent)
#endif

extern KfEffectRecord *effect_pool_find_free(void);
#if KF_MODERN_TYPES && !defined(KF_EFFECT_POOL_IMPLEMENTATION)
extern KfEffectRecord *effect_pool_construct(
    u8 id, KfEffectType type, KfEffectKind kind, const VECTOR *position,
    const SVECTOR *direction);
extern KfEffectRecord *effect_pool_construct(
    u8 id, KfEffectType type, KfEffectKind kind, const VECTOR *position,
    const SVECTOR *direction, KfEffectBranchArguments arguments);
extern KfEffectRecord *effect_pool_construct(
    u8 id, KfEffectType type, KfEffectKind kind, const VECTOR *position,
    const SVECTOR *direction, KfEffectRotationArguments arguments);
extern KfEffectRecord *effect_pool_construct(
    u8 id, KfEffectType type, KfEffectKind kind, const VECTOR *position,
    const SVECTOR *direction, KfEffectRotationSoundArguments arguments);
extern KfEffectRecord *effect_pool_construct(
    u8 id, KfEffectType type, KfEffectKind kind, const VECTOR *position,
    const SVECTOR *direction, KfEffectDurationSoundArguments arguments);
extern KfEffectRecord *effect_pool_construct(
    u8 id, KfEffectType type, KfEffectKind kind, const VECTOR *position,
    const SVECTOR *direction, KfEffectScatterArguments arguments);
extern KfEffectRecord *effect_pool_construct(
    u8 id, KfEffectType type, KfEffectKind kind, const VECTOR *position,
    const SVECTOR *direction, KfEffectSoundArguments arguments);
extern KfEffectRecord *effect_pool_construct(
    u8 id, KfEffectType type, KfEffectKind kind, const VECTOR *position,
    const SVECTOR *direction, KfEffectHomingArguments arguments);
extern KfEffectRecord *effect_pool_construct(
    u8 id, KfEffectType type, KfEffectKind kind, const VECTOR *position,
    const SVECTOR *direction, KfEffectParentArguments arguments);
#else
extern KfEffectRecord *effect_pool_construct(
    u8 id, KfEffectType type, KfEffectKind kind, const VECTOR *position,
    const SVECTOR *direction, ...);
#endif
extern KfEffectRecord *effect_pool_spawn_typed(
    u16 first_segment, u16 segment_count, u16 progress_per_update, u16 cell_stagger,
    s32 sweep_updates, s32 hold_countdown);
extern void effect_pool_set_current(KfEffectRecord *record);
extern void effect_pool_reset(void);
extern void effect_pool_sweep(void);
extern void effect_update_dispatch(void);
extern int effect_magic_power(KfEffectRecord *effect);
extern void effect_projectile_update_3d(SVECTOR *probe_offset, KF_ENUM_PARAM(KfEffectPhase, s32) phase_limit);
extern void effect_projectile_update_2d(s32 orbit_radius, KF_ENUM_PARAM(KfEffectPhase, s32) phase_limit);
extern void effect_floor_deform_line(s32 segment_index, s32 progress_start, s32 progress_step);
extern void effect_scatter_triple(KfEffectDirectionWords *values);
extern void effect_rotate_scale_offset_y(SVECTOR *offset, VECTOR *out, s16 angle, s32 scale);
extern void effect_spawn_ground_trail(u8 id, KfEffectRecord *record, s16 angle, s32 distance);
extern void effect_spawn_ground_branch(u8 id, KfEffectRecord *record, s16 angle_offset, KF_ENUM_PARAM(KfEffectGroundBranchRole, s32) branch_role);
extern u32 effect_map_collision(VECTOR *position, s32 radius);

#endif
