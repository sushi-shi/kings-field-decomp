#ifndef KF_OPEN_RENDER_H
#define KF_OPEN_RENDER_H

/* OPEN.EXE display, render, and TMD state shared across render units. */

#include <kf/game_math.h>
#include <kf/item.h>
#include <kf/render_types.h>
#include <kf/tmd.h>

/* OPEN.EXE display subobject; the active OT pointer follows this record. */
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

/* Cleared as one region by opening_run. Unclassified intervals stay opaque. */
typedef struct KfGraphicsRuntimeOpen {
    KfDisplayStateOpen display_state;
    u32 *ordering_table;
    DRAWENV display_draw_environments[2];
    DISPENV display_disp_environments[2];
    u8 unknown_20108[8];
    KfTmdStateOpen tmd_state;
    u8 unknown_2011c[4];
    SVECTOR *current_tmd_vertices;
    u8 unknown_20124[0x14];
    KfScreenVertex tmd_projected_vertices[1000];
    u8 unknown_22078[0x1f68];
    KfFloorItemStateOpen floor_item_state;
    u32 DAT_8006e040;
    u32 DAT_8006e044;
    KfRenderStateOpen render_state;
    MATRIX light_quadrant_matrices[4];
    const KfCellWindow *active_cell_window;
    s16 tmd_projection_shift;
    u8 unknown_24786[2];
} KfGraphicsRuntimeOpen;
extern KfGraphicsRuntimeOpen open_graphics_runtime;
#define KF_OPEN_GRAPHICS_OFFSET_CHECK(member, offset) \
    typedef char check_##member[ \
        ((unsigned long)&((KfGraphicsRuntimeOpen *)0)->member == (offset)) ? 1 : -1]
KF_OPEN_GRAPHICS_OFFSET_CHECK(display_state, 0x0);
KF_OPEN_GRAPHICS_OFFSET_CHECK(ordering_table, 0x20024);
KF_OPEN_GRAPHICS_OFFSET_CHECK(display_draw_environments, 0x20028);
KF_OPEN_GRAPHICS_OFFSET_CHECK(display_disp_environments, 0x200e0);
KF_OPEN_GRAPHICS_OFFSET_CHECK(unknown_20108, 0x20108);
KF_OPEN_GRAPHICS_OFFSET_CHECK(tmd_state, 0x20110);
KF_OPEN_GRAPHICS_OFFSET_CHECK(unknown_2011c, 0x2011c);
KF_OPEN_GRAPHICS_OFFSET_CHECK(current_tmd_vertices, 0x20120);
KF_OPEN_GRAPHICS_OFFSET_CHECK(unknown_20124, 0x20124);
KF_OPEN_GRAPHICS_OFFSET_CHECK(tmd_projected_vertices, 0x20138);
KF_OPEN_GRAPHICS_OFFSET_CHECK(unknown_22078, 0x22078);
KF_OPEN_GRAPHICS_OFFSET_CHECK(floor_item_state, 0x23fe0);
KF_OPEN_GRAPHICS_OFFSET_CHECK(DAT_8006e040, 0x245f8);
KF_OPEN_GRAPHICS_OFFSET_CHECK(DAT_8006e044, 0x245fc);
KF_OPEN_GRAPHICS_OFFSET_CHECK(render_state, 0x24600);
KF_OPEN_GRAPHICS_OFFSET_CHECK(light_quadrant_matrices, 0x24700);
KF_OPEN_GRAPHICS_OFFSET_CHECK(active_cell_window, 0x24780);
KF_OPEN_GRAPHICS_OFFSET_CHECK(tmd_projection_shift, 0x24784);
KF_OPEN_GRAPHICS_OFFSET_CHECK(unknown_24786, 0x24786);
#undef KF_OPEN_GRAPHICS_OFFSET_CHECK
typedef char check_runtime_size[sizeof(KfGraphicsRuntimeOpen) == 0x24788 ? 1 : -1];

extern MATRIX color_matrix_table[5];
extern KfSpriteQuad floor_item_sprites[7];
extern MATRIX floor_item_light_matrix;
extern SVECTOR render_sprite_light_normal;
extern CVECTOR map_textured_primitive_color;

extern KfCellWindow render_cell_windows[16];
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
extern void sprite_add_ft4(u16 *position, u8 *texcoords, u32 tpage, u32 clut,
                           u8 *color, u16 ot_index);
extern void opening_fade_in(void);
extern void render_set_view_transform(
    const VECTOR *position, const SVECTOR *rotation);
extern void tmd_project_vertices_perspective_right(s32 count);
extern void tmd_project_vertices_shift(s32 count, u8 shift);
extern void tmd_transform_vertices(s32 count);
extern void render_enqueue_tmd(u16 object_index, s16 depth_bias);
extern void render_enqueue_unlit_triangles(u16 object_index, s16 depth_bias);
extern void render_enqueue_sprite(KfSpriteQuad *sprite, s16 depth_bias, s32 flag);
extern void render_floor_item(KfFloorItem *item);
extern void render_map_cell(s32 col, s32 row, u8 cell);
extern void render_enqueue_map(u16 object_index);

#endif
