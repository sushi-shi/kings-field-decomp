#ifndef KF_RENDER_TYPES_H
#define KF_RENDER_TYPES_H

/* Rendering layouts shared by GAME.EXE and OPEN.EXE. */

#include <kf/game_types.h>

/* Texture rectangle followed by its screen-space or projected rectangle. */
typedef struct KfSpriteQuad {
    u8 u;
    u8 v;
    u8 u_span;
    u8 v_span;
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} KfSpriteQuad;

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
