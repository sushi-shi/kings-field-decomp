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
    u8 unknown_02;       /* 0x02 */
    u8 unknown_03;       /* 0x03 */
    u8 unknown_04;       /* 0x04 */
    u8 unknown_05;       /* 0x05 */
    u8 id;               /* 0x06 */
    u8 unknown_07;       /* 0x07 */
    u16 unknown_08;      /* 0x08 */
    u16 unknown_0a;      /* 0x0a */
    VECTOR position;     /* 0x0c */
    u16 rotation_x;      /* 0x1c */
    u16 rotation_y;      /* 0x1e */
    u16 rotation_z;      /* 0x20 */
    u16 unknown_22;      /* 0x22 */
    u16 scale_x;         /* 0x24 */
    u16 scale_y;         /* 0x26 */
    u16 scale_z;         /* 0x28 */
    u16 unknown_2a;      /* 0x2a */
    u16 direction_x;     /* 0x2c */
    u16 direction_y;     /* 0x2e */
    u16 direction_z;     /* 0x30 */
    u16 unknown_32;      /* 0x32 */
    struct KfPoolRecord *animation_cache; /* 0x34 */
    u16 unknown_38;      /* 0x38 */
    u16 unknown_3a;      /* 0x3a */
} KfEffectRecord;

/* Rendering view of the same 60-byte effect-pool record. The renderer reads
 * the low halfwords of the VECTOR position and interprets kind-specific header
 * bytes as sprite selectors. */
typedef struct KfEffectRenderView {
    u8 unknown_00[3];
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
extern KfEffectRecord effect_pool_records[48];
extern KfMagicRecord *current_effect_magic_record;
extern KfEffectRecord *current_effect;

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
extern u32 effect_map_collision(VECTOR *position, s32 radius);

#endif
