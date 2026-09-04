#ifndef KF_MAGIC_H
#define KF_MAGIC_H

/*
 * game.magic (0x8003a244-0x8003a7dc): magic-record loading and cast dispatch.
 * Effect-pool lifecycle declarations live in game_effect.h.
 */

#include <kf/audio.h>

/* Runtime-loaded spell definition used by player and effect code. */
typedef struct KfMagicRecord {
    u8 learned;
    u8 charge_rate;
    SoundRef sounds[2];
    u16 damage_components[4];
    u16 mp_cost;
    u8 unknown_12[0x02];
} KfMagicRecord;

extern void magic_load_records(const u32 *source);
extern void magic_cast(void);

#endif
