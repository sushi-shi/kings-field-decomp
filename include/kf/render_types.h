#ifndef KF_RENDER_TYPES_H
#define KF_RENDER_TYPES_H

/* Rendering layouts shared by GAME.EXE and OPEN.EXE. */

#include <kf/game_types.h>
#include <kf/enum.h>

/* Projected screen vertices in each independent runtime; extent remains WIP. */
enum { KF_PROJECTED_VERTEX_CAPACITY = 1000 };

/* Shared staggered-cylinder animation; pool ownership and mode codes differ. */
enum {
    KF_CYLINDER_TRANSITION_COUNT = 4,
    KF_CYLINDER_TRANSITION_TALL_SCALE = 0x2000,
    KF_CYLINDER_TRANSITION_SCALE_STEP = 0x100,
    KF_CYLINDER_TRANSITION_YAW_STEP = 0x200,
    KF_CYLINDER_TRANSITION_FRAMES = 48,
    KF_CYLINDER_TRANSITION_STAGGER_SHIFT = 3,
    KF_CYLINDER_TRANSITION_STAGGER_FRAMES = 1 << KF_CYLINDER_TRANSITION_STAGGER_SHIFT
};

/* Boosted sprites add half the signed GTE depth-cue factor before shading. */
KF_ENUM_BEGIN(KfSpriteDepthCueMode, s32)
    KF_SPRITE_DEPTH_CUE_NORMAL = 0,
    KF_SPRITE_DEPTH_CUE_BOOSTED = 1
KF_ENUM_END(KfSpriteDepthCueMode)

KF_ENUM_BEGIN(KfDisplayBuffer, u8)
    KF_DISPLAY_BUFFER_FIRST = 0,
    KF_DISPLAY_BUFFER_SECOND = 1,
    KF_DISPLAY_BUFFER_UNINITIALIZED = 0xff
KF_ENUM_END(KfDisplayBuffer)

/* Retail toggles with an equality, so the uninitialized value selects zero. */
#if KF_MODERN_TYPES
constexpr KfDisplayBuffer display_next_buffer(KfDisplayBuffer current)
{
    return static_cast<KfDisplayBuffer>(current == KF_DISPLAY_BUFFER_FIRST);
}
#else
#define display_next_buffer(current) ((current) == KF_DISPLAY_BUFFER_FIRST)
#endif

enum {
    KF_VRAM_WIDTH = 1024,
    KF_VRAM_HEIGHT = 512,
    KF_DISPLAY_WIDTH = 320,
    KF_DISPLAY_HEIGHT = 240,
    KF_DISPLAY_BUFFER_COUNT = 2,
    KF_ORDERING_TABLE_LENGTH = 0x4000,
    KF_ORDERING_TABLE_INDEX_MASK = KF_ORDERING_TABLE_LENGTH - 1,
    KF_SCENE_MIN_OT_DEPTH = 5,
    KF_MAP_OT_DEPTH_BIAS = 200,
    KF_GTE_DEPTH_TO_OT_SHIFT = 2,
    KF_TEXTURE_BASE_BRIGHTNESS = 128,
    KF_VIEW_QUADRANT_COUNT = 4,
    KF_CELL_WINDOW_YAW_COUNT = 16,
    KF_CELL_WINDOW_CELL_CAPACITY = 196,
    KF_CELL_WINDOW_YAW_SHIFT = 8,
    KF_DEFAULT_PROJECTION_DISTANCE = 200,
    KF_INITIAL_FOG_NEAR_DISTANCE = 11000,
    KF_RGB555_CHANNEL_MASK = 0x1f,
    KF_RGB555_GREEN_SHIFT = 5,
    KF_RGB555_BLUE_SHIFT = 10,
    KF_RGB555_STP = 0x8000
};

enum {
    KF_GPU_TEXTURE_4BIT = 0,
    KF_GPU_TEXTURE_8BIT = 1,
    KF_GPU_TEXTURE_16BIT = 2,
    KF_GPU_BLEND_AVERAGE = 0,
    KF_GPU_RESET_FULL = 0,
    KF_GPU_RESET_KEEP_DISPLAY = 3,
    KF_TEXTURE_LOWER_PAGE_Y = 256
};

enum {
    KF_TRANSITION_RECT_X = 32,
    KF_TRANSITION_RECT_WIDTH = 255,
    KF_TRANSITION_RECT_HEIGHT = 240,
    KF_TRANSITION_TPAGE_X = 576,
    KF_TRANSITION_CLUT_Y = 495,
    KF_TRANSITION_FADE_FRAMES = 18,
    KF_TRANSITION_FADE_STEP = 6,
    KF_TRANSITION_FADE_LIMIT = 255,
    KF_TRANSITION_OT_DEPTH = 4
};

/* Quad input coordinates; scrolling Y crosses the top edge of the screen. */
typedef struct KfScreenRect {
    u16 x;
    s16 y;
    u16 w;
    u16 h;
} KfScreenRect;

/* Byte descriptor consumed by sprite_add_ft4; odd bytes remain unclassified. */
enum {
    KF_QUAD_TEX_U = 0,
    KF_QUAD_TEX_V = 2,
    KF_QUAD_TEX_U_SPAN = 4,
    KF_QUAD_TEX_V_SPAN = 6,
    KF_QUAD_TEX_DESCRIPTOR_BYTES = 8
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

KF_ENUM_BEGIN(KfCellVisibility, u8)
    KF_CELL_WINDOW_HIDDEN = 0,
    KF_CELL_WINDOW_DISTANT = 1,
    KF_CELL_WINDOW_NEAR = 2
KF_ENUM_END(KfCellVisibility)

/* Per-yaw row-major cell classes shared by GAME and OPEN. */
typedef struct KfCellWindow {
    u16 width;
    u16 height;
    u16 origin_x;
    u16 origin_z;
    KfCellVisibility cells[KF_CELL_WINDOW_CELL_CAPACITY];
} KfCellWindow;

#endif
