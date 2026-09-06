#ifndef KF_RENDER_TYPES_H
#define KF_RENDER_TYPES_H

/* Rendering layouts shared by GAME.EXE and OPEN.EXE. */

#include <kf/game_types.h>

enum {
    KF_VRAM_WIDTH = 1024,
    KF_VRAM_HEIGHT = 512,
    KF_DISPLAY_WIDTH = 320,
    KF_DISPLAY_HEIGHT = 240,
    KF_DISPLAY_BUFFER_COUNT = 2,
    KF_DISPLAY_BUFFER_UNINITIALIZED = 0xff,
    KF_ORDERING_TABLE_LENGTH = 0x4000,
    KF_ORDERING_TABLE_INDEX_MASK = KF_ORDERING_TABLE_LENGTH - 1,
    KF_VIEW_QUADRANT_COUNT = 4,
    KF_CELL_WINDOW_YAW_COUNT = 16,
    KF_CELL_WINDOW_YAW_SHIFT = 8,
    KF_DEFAULT_PROJECTION_DISTANCE = 200,
    KF_INITIAL_FOG_NEAR_DISTANCE = 11000,
    KF_RGB555_CHANNEL_MASK = 0x1f,
    KF_RGB555_GREEN_SHIFT = 5,
    KF_RGB555_BLUE_SHIFT = 10,
    KF_RGB555_STP = 0x8000
};

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

/* One reversed ordering table (ClearOTagR/DrawOTag). */
typedef struct KfOrderingTable {
    u32 entries[KF_ORDERING_TABLE_LENGTH];
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
