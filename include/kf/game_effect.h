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

/* Each kind selects the interpretation; all alternatives retain u16 storage. */
typedef union KfEffectVisualState {
    u16 animation_phase;
    u16 pulse_base_scale;
} KfEffectVisualState;

typedef union KfEffectControl {
    u16 frames_remaining;
    u16 orbit_angle;
    u16 parent_effect_index;
    u16 target_mode;
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
    s16 rotation_x;      /* 0x1c */
    s16 rotation_y;      /* 0x1e */
    s16 rotation_z;      /* 0x20 */
    u16 rotation_pad;    /* 0x22: copied SVECTOR fourth lane */
    u16 scale_x;         /* 0x24 */
    u16 scale_y;         /* 0x26 */
    u16 scale_z;         /* 0x28 */
    u16 unknown_2a;      /* 0x2a */
    u16 direction_x;     /* 0x2c */
    u16 direction_y;     /* 0x2e */
    u16 direction_z;     /* 0x30 */
    u16 direction_pad;   /* 0x32: copied SVECTOR fourth lane */
    struct KfPoolRecord *animation_cache; /* 0x34 */
    KfEffectControl control; /* 0x38: countdown, orbit, parent, or homing selector */
    KfEffectPropagation propagation; /* 0x3a: kind-10 generations or kind-6 branch */
} KfEffectRecord;

typedef char check_effect_record_size[sizeof(KfEffectRecord) == 0x3c ? 1 : -1];
#define KF_EFFECT_OFFSET_CHECK(member, offset) \
    typedef char check_effect_##member[ \
        ((unsigned long)&((KfEffectRecord *)0)->member == (offset)) ? 1 : -1]
KF_EFFECT_OFFSET_CHECK(rotation_x, 0x1c);
KF_EFFECT_OFFSET_CHECK(rotation_y, 0x1e);
KF_EFFECT_OFFSET_CHECK(rotation_z, 0x20);
KF_EFFECT_OFFSET_CHECK(visual, 0x08);
KF_EFFECT_OFFSET_CHECK(control, 0x38);
KF_EFFECT_OFFSET_CHECK(propagation, 0x3a);
#undef KF_EFFECT_OFFSET_CHECK

/* Startup clears this whole object; selection derives the magic array from
 * the current-record slot by a fixed member offset. */
typedef struct KfEffectState {
    KfMagicRecord magic[24];
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

extern SVECTOR effect_projectile_velocities[2];
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
    u16 rotation_x, u16 rotation_y, u16 rotation_z, u16 direction_y,
    s32 position_x, s32 position_y);
extern void effect_pool_set_current(KfEffectRecord *record);
extern void effect_pool_reset(void);
extern void effect_pool_sweep(void);
extern void effect_update_dispatch(void);
extern void effect_projectile_update_3d(SVECTOR *velocity, s32 frame_limit);
extern void effect_projectile_update_2d(s32 speed, s32 frame_limit);
extern u32 effect_map_collision(VECTOR *position, s32 radius);

#endif
