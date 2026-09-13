#ifndef KF_OPEN_RENDER_H
#define KF_OPEN_RENDER_H

/* OPEN.EXE display, render, and TMD state shared across render units. */

#include <kf/enum.h>
#include <kf/game_math.h>
#include <kf/item.h>
#include <kf/overlay.h>
#include <kf/render_types.h>
#include <kf/tmd.h>

KF_ENUM_BEGIN(KfOpenColorPreset, s32)
    KF_OPEN_COLOR_DEFAULT = 0,
    KF_OPEN_COLOR_BLACK = 1,
    KF_OPEN_COLOR_WHITE = 2,
    KF_OPEN_COLOR_ENDING_MIDPOINT = 3,
    KF_OPEN_COLOR_ENDING_GREEN = 4
KF_ENUM_END(KfOpenColorPreset)

enum {
    KF_OPEN_PROJECTED_VERTEX_CAPACITY = 1000,
    KF_OPEN_COLOR_PRESET_COUNT = 5,
    KF_OPEN_TMD_SLOT_COUNT = 2,
};

/* OPEN.EXE display subobject; the active OT pointer follows this record. */
typedef struct KfDisplayStateOpen {
    KfDisplayBuffer buffer_index;
    u8 unknown_01[3];
    void *asset_load_buffer;
    KfPrimitiveBuffer primitive_buffers[KF_DISPLAY_BUFFER_COUNT];
    KfPrimitiveBuffer *primitive_buffer;
    KfOrderingTable ordering_tables[KF_DISPLAY_BUFFER_COUNT];
} KfDisplayStateOpen;

/* Two registered TMD slots and the selected asset. */
typedef struct KfTmdStateOpen {
    KfTmdHeader *slots[KF_OPEN_TMD_SLOT_COUNT];
    KfTmdHeader *current_asset;
} KfTmdStateOpen;

/* OPEN.EXE view state lacks GAME's two extra light matrices. */
typedef struct KfRenderStateOpen {
    MATRIX view_matrix;
    MATRIX pitch_matrix;
    MATRIX light_matrix;
    s32 fog_near_distance;
    VECTOR view_position;
    SVECTOR view_rotation;
    struct KfVecXZs view_cell;
    MATRIX quadrant_matrices[KF_VIEW_QUADRANT_COUNT];
} KfRenderStateOpen;

/* Texture selectors followed by the SDK color used for projected sprites. */
typedef struct KfSpriteMaterial {
    u16 clut;
    u16 tpage;
    CVECTOR color;
} KfSpriteMaterial;

/* Retail traversal derives items from material.tpage + 22 bytes. */
typedef struct KfFloorItemStateOpen {
    KfSpriteMaterial material;
    u8 unknown_08[6];
    u16 texture_clut;
    u16 texture_tpage;
    u16 count;
    u8 unknown_14[4];
    KfFloorItem items[KF_FLOOR_ITEM_CAPACITY];
} KfFloorItemStateOpen;

/* Cleared as one region by opening_run. Unclassified intervals stay opaque. */
typedef struct KfGraphicsRuntimeOpen {
    KfDisplayStateOpen display_state;
    u32 *ordering_table;
    DRAWENV display_draw_environments[KF_DISPLAY_BUFFER_COUNT];
    DISPENV display_disp_environments[KF_DISPLAY_BUFFER_COUNT];
    u8 unknown_20108[8];
    KfTmdStateOpen tmd_state;
    u8 unknown_2011c[4];
    SVECTOR *current_tmd_vertices;
    u8 unknown_20124[0x14];
    KfScreenVertex tmd_projected_vertices[KF_OPEN_PROJECTED_VERTEX_CAPACITY];
    u8 unknown_22078[0x1f68];
    KfFloorItemStateOpen floor_item_state;
    u32 DAT_8006e040;
    u32 DAT_8006e044;
    KfRenderStateOpen render_state;
    MATRIX light_quadrant_matrices[KF_VIEW_QUADRANT_COUNT];
    const KfCellWindow *active_cell_window;
    s16 tmd_projection_shift;
    u8 unknown_24786[2];
} KfGraphicsRuntimeOpen;
extern KfGraphicsRuntimeOpen open_graphics_runtime;

extern MATRIX color_matrix_table[KF_OPEN_COLOR_PRESET_COUNT];
extern KfSpriteQuad floor_item_sprites[KF_FLOOR_ITEM_SPRITE_COUNT];
extern MATRIX floor_item_light_matrix;
extern SVECTOR render_sprite_light_normal;
extern CVECTOR map_textured_primitive_color;

extern KfCellWindow render_cell_windows[KF_CELL_WINDOW_YAW_COUNT];
extern u32 primitive_allocation_count;

extern void display_initialize(KfOpenMode mode);
extern void lighting_set_active_color_matrix(KfOpenColorPreset preset);
extern void matrix_interpolate(
    const MATRIX *from, const MATRIX *to, MATRIX *output, s32 blend);
extern void lighting_set_color_matrix(
    const MATRIX *from, const MATRIX *to, s32 blend);
extern void lighting_set_light_matrix(
    const MATRIX *from, const MATRIX *to, s32 blend);
extern void fog_interpolate_near(s32 start, s32 end, s32 ratio);
extern void fog_set_near(s32 distance);
extern void color_lerp_cvector(
    const CVECTOR *from, const CVECTOR *to, CVECTOR *output, s32 blend);
extern u16 color_lerp_rgb555(u16 color0, u16 color1, s32 blend);
extern void *primitive_buffer_allocate(u16 byte_count);
extern void render_initialize(void);
extern void display_begin_frame(void);
extern void display_present_frame(void);
extern void sprite_add_f4(
    const KfScreenRect *position, const CVECTOR *color, u16 ot_index);
extern void sprite_add_ft4(const KfScreenRect *position, u8 *texcoords, u32 tpage, u32 clut,
                           const CVECTOR *color, u16 ot_index);
extern void opening_fade_in(void);
extern void render_set_view_transform(
    const VECTOR *position, const SVECTOR *rotation);
extern void tmd_project_vertices_perspective_right(s32 count);
extern void tmd_project_vertices_shift(s32 count, u8 shift);
extern void tmd_transform_vertices(s32 count);
extern void render_enqueue_tmd(u16 object_index, s16 depth_bias);
extern void render_enqueue_unlit_triangles(u16 object_index, s16 depth_bias);
extern void render_enqueue_sprite(
    KfSpriteQuad *sprite, s16 depth_bias, KfSpriteDepthCueMode depth_cue_mode);
extern void render_floor_item(KfFloorItem *item);
extern void render_map_cell(s32 col, s32 row, KfCellVisibility cell);
extern void render_enqueue_map(u16 object_index);

#endif
