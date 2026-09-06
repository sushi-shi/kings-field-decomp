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
enum {
    KF_EFFECT_KIND_GROUND_BRANCH = 6,
    KF_EFFECT_KIND_ACTOR_SPAWNER = 9,
    KF_EFFECT_KIND_SCATTER_PROJECTILE = 10,
    KF_EFFECT_KIND_DARKNESS_PROJECTILE = 11,
    KF_EFFECT_KIND_CURSE_PROJECTILE = 12,
    KF_EFFECT_KIND_EMERGING_PROJECTILE = 13,
    KF_EFFECT_KIND_SWINGING_HAZARD_SHORT = 15,
    KF_EFFECT_KIND_SWINGING_HAZARD_LONG = 16,
    KF_EFFECT_KIND_ORBITING_PROJECTILE = 17,
    KF_EFFECT_KIND_RADIAL_BLAST = 18,
    KF_EFFECT_KIND_GROUND_TRAIL = 19,
    KF_EFFECT_KIND_HOMING_PROJECTILE = 20,
    KF_EFFECT_KIND_FLOOR_DEFORMATION = 52
};

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
enum {
    KF_EFFECT_HOMING_WANDER = 0xff,
    KF_EFFECT_HOMING_PLAYER = 0xfe
};

enum {
    KF_EFFECT_GROUND_BRANCH_ROOT = 0,
    KF_EFFECT_GROUND_BRANCH_QUARTER_TURN = 1,
    KF_EFFECT_GROUND_BRANCH_THREE_QUARTER_TURN = 2,
    KF_EFFECT_GROUND_BRANCH_LEAF = 0xff,
    KF_EFFECT_GROUND_BRANCH_TIMER_DONE = 0xff
};

enum {
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

/* Kind 36 initializes both bytes; their behavioral purpose is unresolved. */
typedef struct KfEffectControlBytes {
    u8 low;
    u8 high;
} KfEffectControlBytes;

typedef union KfEffectControl {
    u16 frames_remaining;
    u16 orbit_angle;
    u8 parent_effect_index;
    u8 target_mode;
    KfEffectControlBytes bytes;
} KfEffectControl;

typedef union KfEffectPropagation {
    u16 generations_remaining;
    u16 branch;
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
    u8 kind;             /* 0x01 */
    u8 base_render_id;   /* 0x02: first sprite/model in the animation */
    u8 render_id;        /* 0x03 */
    u8 animation_clip;   /* 0x04: 0xff selects a billboard instead of a model */
    u8 sound_played;     /* 0x05: spatial sound accepted within audible range */
    u8 id;               /* 0x06 */
    u8 phase;            /* 0x07: kind-specific lifecycle/age */
    KfEffectVisualState visual; /* 0x08: animation phase, or kind-10 pulse scale */
    u16 unknown_0a;      /* 0x0a */
    VECTOR position;     /* 0x0c */
    SVECTOR rotation;    /* 0x1c */
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
KF_EFFECT_OFFSET_CHECK(rotation, rotation, 0x1c);
KF_EFFECT_OFFSET_CHECK(rotation_y, rotation.vy, 0x1e);
KF_EFFECT_OFFSET_CHECK(rotation_z, rotation.vz, 0x20);
KF_EFFECT_OFFSET_CHECK(rotation_pad, rotation.pad, 0x22);
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

/* Rendering view of the same 60-byte effect-pool record. The renderer reads
 * the low halfwords of the VECTOR position and interprets kind-specific header
 * bytes as sprite selectors. */
typedef struct KfEffectRenderView {
    u8 type;
    u8 kind;
    u8 base_render_id;
    u8 sprite_id;
    u8 mode;
    u8 unknown_05[3];
    u16 asset_variant;
    u8 unknown_0a[2];
    u16 position_x;
    u16 unknown_0e;
    u16 position_y;
    u16 unknown_12;
    u16 position_z;
    u8 unknown_16[6];
    struct KfEulerAngles rotation;
    u8 unknown_22[2];
    s16 scale_x;
    s16 scale_y;
    s16 scale_z;
    u8 unknown_2a[10];
    struct KfPoolRecord *animation_cache;
    u8 unknown_38[4];
} KfEffectRenderView;

extern SVECTOR effect_swing_probe_offsets[KF_EFFECT_SWING_PROBE_COUNT];
extern KfEffectState effect_state;
/* Consumer spellings are members, not separately owned globals. */
#define magic_records (effect_state.magic)
#define effect_pool_records (effect_state.records)
#define current_effect_magic_record (effect_state.current_magic)
#define current_effect (effect_state.current_record)

extern KfEffectRecord *effect_pool_find_free(void);
extern KfEffectRecord *effect_pool_construct(
    u8 id, u8 type, u8 kind, const VECTOR *position,
    const SVECTOR *direction, ...);
extern KfEffectRecord *effect_pool_spawn_typed(
    u16 first_segment, u16 segment_count, u16 progress_per_update, u16 cell_stagger,
    s32 sweep_updates, s32 hold_countdown);
extern void effect_pool_set_current(KfEffectRecord *record);
extern void effect_pool_reset(void);
extern void effect_pool_sweep(void);
extern void effect_update_dispatch(void);
extern void effect_projectile_update_3d(SVECTOR *probe_offset, s32 frame_limit);
extern void effect_projectile_update_2d(s32 speed, s32 frame_limit);
extern u32 effect_map_collision(VECTOR *position, s32 radius);

#endif
