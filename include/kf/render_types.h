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

/* Per-yaw row-major visible-cell window shared by GAME and OPEN. */
typedef struct KfCellWindow {
    u16 width;
    u16 height;
    u16 origin_x;
    u16 origin_z;
    u8 cells[196];
} KfCellWindow;

#endif
