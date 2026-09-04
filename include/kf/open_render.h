#ifndef KF_OPEN_RENDER_H
#define KF_OPEN_RENDER_H

/* OPEN.EXE display, render, and TMD state shared across render units. */

#include <kf/game_math.h>
#include <kf/item.h>
#include <kf/render_types.h>
#include <kf/tmd.h>

/* OPEN.EXE display state; ordering_table is a separate global. */
typedef struct KfDisplayStateOpen {
    u8 buffer_index;
    u8 unknown_01[3];
    void *asset_load_buffer;
    KfPrimitiveBuffer primitive_buffers[2];
    KfPrimitiveBuffer *primitive_buffer;
    KfOrderingTable ordering_tables[2];
} KfDisplayStateOpen;

/* Two registered TMD slots and the selected asset. */
typedef struct KfTmdStateOpen {
    u8 *slots[2];
    void *current_asset;
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
    MATRIX quadrant_matrices[4];
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
    KfFloorItem items[64];
} KfFloorItemStateOpen;

extern KfDisplayStateOpen display_state;
extern KfRenderStateOpen render_state;
extern KfTmdStateOpen tmd_state;
extern DRAWENV display_draw_environments[2];
extern DISPENV display_disp_environments[2];
extern u32 *ordering_table;
extern MATRIX light_quadrant_matrices[4];
extern MATRIX color_matrix_table[5];
extern KfSpriteQuad floor_item_sprites[7];
extern MATRIX floor_item_light_matrix;
extern SVECTOR render_sprite_light_normal;
extern KfFloorItemStateOpen floor_item_state;

extern KfCellWindow render_cell_windows[16];
extern const KfCellWindow *active_cell_window;
extern u32 DAT_8006e040;
extern u32 DAT_8006e044;
extern s16 tmd_projection_shift;
extern u32 primitive_allocation_count;

extern void display_initialize(s32 mode);
extern void lighting_set_active_color_matrix(s32 index);
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
    const u16 *position, const u8 *color, u16 ot_index);
extern void sprite_add_ft4(u16 *position, u8 *texcoords, u16 tpage, u16 clut,
                           u8 *color, u16 ot_index);
extern void opening_fade_in(void);
extern void render_set_view_transform(
    const VECTOR *position, const SVECTOR *rotation);
extern void tmd_project_vertices_perspective_right(s32 count);
extern void tmd_project_vertices_shift(s32 count, u8 shift);
extern void tmd_transform_vertices(s32 count);
extern void func_8001764c(u16 object_index, s16 depth_bias);
extern void render_enqueue_unlit_triangles(u16 object_index, s16 depth_bias);
extern void render_enqueue_sprite(KfSpriteQuad *sprite, s16 depth_bias, s32 flag);
extern void render_floor_item(KfFloorItem *item);
extern void render_map_cell(s32 col, s32 row, u8 cell);
extern void func_800185e8(u16 object_index);

#endif
