#ifndef KF_GAME_EFFECT_H
#define KF_GAME_EFFECT_H

/*
 * GAME.EXE effect-pool lifecycle, construction, collision, and dispatch.
 * The general constructor has five fixed arguments followed by slots whose
 * meanings depend on KIND; callers must supply only the slots used by that
 * effect kind.
 */

#include <kf/semantic_types.h>

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
