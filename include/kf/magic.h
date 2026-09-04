#ifndef KF_MAGIC_H
#define KF_MAGIC_H

/*
 * game.magic (0x8003a244-0x8003a7dc): magic-record loading and cast dispatch.
 * Effect-pool lifecycle declarations live in game_effect.h.
 */

#include <kf/semantic_types.h>

extern void magic_load_records(const u32 *source);
extern void magic_cast(void);

#endif
