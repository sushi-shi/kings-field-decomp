#ifndef KF_MAGIC_H
#define KF_MAGIC_H

/*
 * game.magic (0x8003a244-0x8003a7dc): the magic/effect-pool subsystem. Exports
 * the effect-pool reset/sweep, the magic record loader, and the cast dispatch.
 */

#include <kf/semantic_types.h>

extern void effect_pool_reset(void);
extern void magic_load_records(const u32 *source);
extern void magic_cast(void);
extern void effect_pool_sweep(void);

#endif
