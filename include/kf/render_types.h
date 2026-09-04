#ifndef KF_RENDER_TYPES_H
#define KF_RENDER_TYPES_H

/* Display layouts shared by GAME.EXE and OPEN.EXE. */

#include <kf/game_types.h>

/* One primitive allocation interval and its current write cursor. */
typedef struct KfPrimitiveBuffer {
    u8 *start;
    u8 *end;
    u8 *cursor;
} KfPrimitiveBuffer;

/* One reversed ordering table of 0x4000 entries (ClearOTagR/DrawOTag). */
typedef struct KfOrderingTable {
    u32 entries[0x4000];
} KfOrderingTable;

#endif
