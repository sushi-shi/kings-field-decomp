#ifndef KF_GAME_EFFECT_H
#define KF_GAME_EFFECT_H

/*
 * GAME.EXE effect-pool lifecycle, construction, collision, and dispatch.
 * The general constructor has five fixed arguments followed by slots whose
 * meanings depend on KIND; callers must supply only the slots used by that
 * effect kind.
 */

#include <kf/game_math.h>
#include <kf/magic.h>

struct KfPoolRecord;

enum {
    KF_EFFECT_CAPACITY = 48,
    KF_EFFECT_SLOT_FREE = 0xff
};

/* Behavioral identities; other kind/resource IDs remain unresolved. */
KF_ENUM_BEGIN(KfEffectKind, u8)
    KF_EFFECT_KIND_LIGHTNING_BOLT = KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHTNING_BOLT),
    KF_EFFECT_KIND_FIRE_BALL = KF_ENUM_ENCODE(u8, KF_MAGIC_FIRE_BALL),
    KF_EFFECT_KIND_GROUND_BRANCH = KF_ENUM_ENCODE(u8, KF_MAGIC_FIRE_WALL),
    KF_EFFECT_KIND_WIND_CUTTER = KF_ENUM_ENCODE(u8, KF_MAGIC_WIND_CUTTER),
    KF_EFFECT_KIND_LIGHT_NEEDLE = KF_ENUM_ENCODE(u8, KF_MAGIC_LIGHT_NEEDLE),
    KF_EFFECT_KIND_ACTOR_SPAWNER = 9,
    KF_EFFECT_KIND_SCATTER_PROJECTILE = 10,
    KF_EFFECT_KIND_DARKNESS_PROJECTILE = 11,
    KF_EFFECT_KIND_CURSE_PROJECTILE = 12,
    KF_EFFECT_KIND_EMERGING_PROJECTILE = 13,
    KF_EFFECT_KIND_MAP_EMITTER_PROJECTILE = 14,
    KF_EFFECT_KIND_SWINGING_HAZARD_SHORT = 15,
    KF_EFFECT_KIND_SWINGING_HAZARD_LONG = 16,
    KF_EFFECT_KIND_ORBITING_PROJECTILE = 17,
    KF_EFFECT_KIND_RADIAL_BLAST = 18,
    KF_EFFECT_KIND_GROUND_TRAIL = 19,
    KF_EFFECT_KIND_HOMING_PROJECTILE = 20,
    KF_EFFECT_KIND_WARP_SHIMMER = 21,
    KF_EFFECT_KIND_PHYSICAL_PROJECTILE = 22,
    KF_EFFECT_KIND_LIGHTNING_IMPACT = 32,
    KF_EFFECT_KIND_LIGHTNING_RADIAL_BLAST = 33,
    KF_EFFECT_KIND_GROUND_BRANCH_VISUAL = 34,
    KF_EFFECT_KIND_MOONLIGHT_PROJECTILE = 36,
    KF_EFFECT_KIND_LIGHTNING_BOLT_ALTERNATE = 23,
    KF_EFFECT_KIND_HOMING_PROJECTILE_ALTERNATE = 24,
    KF_EFFECT_KIND_LIGHTNING_IMPACT_ALTERNATE = 41,
    KF_EFFECT_KIND_LIGHTNING_RADIAL_BLAST_ALTERNATE = 42,
    KF_EFFECT_KIND_RADIAL_BLAST_ALTERNATE = 44,
    KF_EFFECT_KIND_MAP_SWITCH = 48,
    KF_EFFECT_KIND_FLOOR_DEFORMATION = 52
KF_ENUM_END(KfEffectKind)

/* Resource identities follow their effect use; billboard IDs may advance by frame. */
enum {
    KF_EFFECT_BILLBOARD_FIRE_BALL = 0,
    KF_EFFECT_BILLBOARD_WIND_CUTTER = 5,
    KF_EFFECT_BILLBOARD_LIGHTNING_BOLT = 6,
    KF_EFFECT_BILLBOARD_DARKNESS_PROJECTILE = 8,
    KF_EFFECT_BILLBOARD_SCATTER_PROJECTILE = 9,
    KF_EFFECT_BILLBOARD_CURSE_PROJECTILE = 10,
    KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT = 11,
    KF_EFFECT_BILLBOARD_GROUND_TRAIL = 14,
    KF_EFFECT_BILLBOARD_LIGHTNING_BOLT_ALTERNATE = 17,
    KF_EFFECT_BILLBOARD_LIGHTNING_IMPACT_ALTERNATE = 19,
    KF_EFFECT_BILLBOARD_SPRITE_COUNT = 22
};

enum {
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
    KF_EFFECT_MODEL_RADIAL_BLAST_ALTERNATE = 17
};

enum { KF_EFFECT_ANIMATION_FIRST_CLIP = 0 };

/* Normalized packed codes retain word arithmetic before the byte API boundary. */
typedef KF_ENUM_PROMOTED(KfEffectKind) KfEffectKindArgument;

/* Low type bits select actors/player after the separate terrain checks.
 * The power bit alone does not imply the actor-damage player-credit class. */
enum {
    KF_EFFECT_COLLISION_TARGETS_MASK = 3,
    KF_EFFECT_COLLISION_TARGET_ACTORS = 1,
    KF_EFFECT_COLLISION_TARGET_PLAYER = 2,
    KF_EFFECT_COLLISION_TARGET_ACTORS_AND_PLAYER = 3,
    KF_EFFECT_USE_PLAYER_MAGIC = 0x10
};

enum {
    KF_EFFECT_ANIMATION_BILLBOARD = 0xff,
    KF_EFFECT_RENDER_NONE = 0xff
};

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

enum {
    KF_EFFECT_FLOOR_DEFORM_TYPE = 0xf0,
    KF_EFFECT_FLOOR_DEFORM_ADVANCE = 0,
    KF_EFFECT_FLOOR_DEFORM_HOLD = 1,
    KF_EFFECT_FLOOR_DEFORM_REVERSE = 2
};

/* Shared projectile dispatcher phases; the phase byte also counts updates. */
enum {
    KF_EFFECT_PROJECTILE_TRAVEL = 0,
    KF_EFFECT_PROJECTILE_IMPACT_FIRST = 1,
    KF_EFFECT_PROJECTILE_DISSIPATE_FIRST = 50,
    KF_EFFECT_PROJECTILE_DISSIPATE_END = 60,
    KF_EFFECT_PROJECTILE_EMERGE_FIRST = 100,
    KF_EFFECT_PROJECTILE_EMERGE_LAST = 119,
    KF_EFFECT_PROJECTILE_FALL = 120,
    KF_EFFECT_PROJECTILE_SHRINK = 121,
    KF_EFFECT_PROJECTILE_LAUNCH_WRAP = 0xff
};

/* Twenty emergence updates undo the initial downward placement offset. */
enum {
    KF_EFFECT_EMERGE_Y_STEP = 175,
    KF_EFFECT_EMERGE_DEPTH =
        (KF_EFFECT_PROJECTILE_FALL - KF_EFFECT_PROJECTILE_EMERGE_FIRST) * KF_EFFECT_EMERGE_Y_STEP,
    /* X/Z centers are quantized world coordinates, not Q8 fractions. */
    KF_EFFECT_ORBIT_CENTER_SHIFT = 8
};

enum {
    KF_EFFECT_GROUND_TRAIL_WAIT_FOR_PARENT = 0,
    KF_EFFECT_GROUND_TRAIL_SHRINK = 1
};

/* The orbit helper keeps running at RELEASE_REQUEST; only the 3D helper
 * settles its pitch and enters the rise sequence on a zero crossing. */
enum {
    KF_EFFECT_HAZARD_RUNNING = 0,
    KF_EFFECT_HAZARD_RELEASE_REQUEST = 1,
    KF_EFFECT_HAZARD_RISE_FIRST = 10,
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
typedef struct KfEffectRecord {
    u8 type;             /* 0x00 */
    KfEffectKind kind;   /* 0x01 */
    u8 base_render_id;   /* 0x02: first sprite/model in the animation */
    u8 render_id;        /* 0x03 */
    u8 animation_clip;   /* 0x04: 0xff selects a billboard instead of a model */
    u8 sound_played;     /* 0x05: spatial sound accepted within audible range */
    u8 id;               /* 0x06 */
    u8 phase;            /* 0x07: kind-specific lifecycle/age */
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

#define KF_EFFECT_OFFSET_CHECK(label, member, offset) \
    typedef char check_effect_##label[ \
        ((unsigned long)&((KfEffectRecord *)0)->member == (offset)) ? 1 : -1]
typedef char check_effect_record_size[sizeof(KfEffectRecord) == 60 ? 1 : -1];
typedef char check_effect_rotation_size[sizeof(KfRotation) == 8 ? 1 : -1];
KF_EFFECT_OFFSET_CHECK(position, position, 0x0c);
KF_EFFECT_OFFSET_CHECK(rotation, rotation, 0x1c);
KF_EFFECT_OFFSET_CHECK(rotation_vector, rotation.vector, 0x1c);
KF_EFFECT_OFFSET_CHECK(rotation_angles, rotation.angles, 0x1c);
KF_EFFECT_OFFSET_CHECK(rotation_angle_y, rotation.angles.y, 0x1e);
KF_EFFECT_OFFSET_CHECK(rotation_angle_z, rotation.angles.z, 0x20);
KF_EFFECT_OFFSET_CHECK(rotation_y, rotation.vector.vy, 0x1e);
KF_EFFECT_OFFSET_CHECK(rotation_z, rotation.vector.vz, 0x20);
KF_EFFECT_OFFSET_CHECK(rotation_pad, rotation.vector.pad, 0x22);
KF_EFFECT_OFFSET_CHECK(direction, direction, 0x2c);
KF_EFFECT_OFFSET_CHECK(direction_pad, direction.words.pad, 0x32);
KF_EFFECT_OFFSET_CHECK(visual, visual, 0x08);
KF_EFFECT_OFFSET_CHECK(control, control, 0x38);
KF_EFFECT_OFFSET_CHECK(control_high_byte, control.bytes.high, 0x39);
KF_EFFECT_OFFSET_CHECK(propagation, propagation, 0x3a);
#undef KF_EFFECT_OFFSET_CHECK

/* Startup clears this whole object; selection derives the magic array from
 * the current-record slot by a fixed member offset. */
typedef struct KfEffectState {
    KfMagicRecord magic[KF_MAGIC_RECORD_COUNT];
    KfEffectRecord records[KF_EFFECT_CAPACITY];
    KfMagicRecord *current_magic;
    KfEffectRecord *current_record;
} KfEffectState;

extern SVECTOR effect_swing_probe_offsets[KF_EFFECT_SWING_PROBE_COUNT];
extern KfEffectState effect_state;
/* Consumer spellings are members, not separately owned globals. */
#define magic_records (effect_state.magic)
#define effect_pool_records (effect_state.records)
#define current_effect_magic_record (effect_state.current_magic)
#define current_effect (effect_state.current_record)

extern KfEffectRecord *effect_pool_find_free(void);
extern KfEffectRecord *effect_pool_construct(
    u8 id, u8 type, KfEffectKind kind, const VECTOR *position,
    const SVECTOR *direction, ...);
extern KfEffectRecord *effect_pool_spawn_typed(
    u16 first_segment, u16 segment_count, u16 progress_per_update, u16 cell_stagger,
    s32 sweep_updates, s32 hold_countdown);
extern void effect_pool_set_current(KfEffectRecord *record);
extern void effect_pool_reset(void);
extern void effect_pool_sweep(void);
extern void effect_update_dispatch(void);
extern int effect_magic_power(KfEffectRecord *effect);
extern void effect_projectile_update_3d(SVECTOR *probe_offset, s32 phase_limit);
extern void effect_projectile_update_2d(s32 orbit_radius, s32 phase_limit);
extern void effect_floor_deform_line(s32 segment_index, s32 progress_start, s32 progress_step);
extern void effect_scatter_triple(KfEffectDirectionWords *values);
extern void effect_rotate_scale_offset_y(SVECTOR *offset, VECTOR *out, s16 angle, s32 scale);
extern void effect_spawn_ground_trail(u8 id, KfEffectRecord *record, s16 angle, s32 distance);
extern void effect_spawn_ground_branch(u8 id, KfEffectRecord *record, s16 angle_offset, KF_ENUM_PARAM(KfEffectGroundBranchRole, s32) branch_role);
extern u32 effect_map_collision(VECTOR *position, s32 radius);

#endif
