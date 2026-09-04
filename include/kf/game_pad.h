#ifndef KF_GAME_PAD_H
#define KF_GAME_PAD_H

/* GAME.EXE controller initialization and polling interface. */

#include <kf/game_types.h>

extern u32 pad_initialize(s32 identifier);
/* Retail callers pass an ignored identifier to this K&R-style boundary. */
extern u32 pad_read();
extern void pad_stop(void);

#endif
