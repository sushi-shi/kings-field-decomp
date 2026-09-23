#ifndef KF_OPEN_RENDER_H
#define KF_OPEN_RENDER_H

#include <kf/lib/enum.h>
#include <kf/lib/cd_file.h>
#include <kf/lib/math.h>
#include <kf/lib/item.h>
#include <kf/lib/map_data.h>
#include <kf/lib/overlay.h>
#include <kf/lib/render_types.h>
#include <kf/lib/tmd.h>

enum class KfOpenColorPreset : s32 {
    KF_OPEN_COLOR_DEFAULT = 0,
    KF_OPEN_COLOR_BLACK = 1,
    KF_OPEN_COLOR_WHITE = 2,
    KF_OPEN_COLOR_ENDING_MIDPOINT = 3,
    KF_OPEN_COLOR_ENDING_GREEN = 4
}; using enum KfOpenColorPreset;

enum {
    KF_OPEN_COLOR_PRESET_COUNT = 5,
    KF_OPEN_TMD_SLOT_COUNT = 2
};

typedef struct KfDisplayStateOpen {
    KfDisplayBuffer buffer_index;
    u8 unknown_01[3];
    u8 *asset_load_buffer;
    KfPrimitiveBuffer primitive_buffers[KF_DISPLAY_BUFFER_COUNT];
    KfPrimitiveBuffer *primitive_buffer;
    KfOrderingTable ordering_tables[KF_DISPLAY_BUFFER_COUNT];
} KfDisplayStateOpen;

typedef struct KfTmdStateOpen {
    KfTmdHeader *slots[KF_OPEN_TMD_SLOT_COUNT];
    KfTmdHeader *current_asset;
} KfTmdStateOpen;

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

typedef struct KfSpriteMaterial {
    u16 clut;
    u16 tpage;
    CVECTOR color;
} KfSpriteMaterial;

typedef struct KfFloorItemStateOpen {
    KfSpriteMaterial material;
    u8 unknown_08[6];
    u16 texture_clut;
    u16 texture_tpage;
    u16 count;
    u8 unknown_14[4];
    KfFloorItem items[KF_FLOOR_ITEM_CAPACITY];
} KfFloorItemStateOpen;

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
    KfScreenVertex tmd_projected_vertices[KF_PROJECTED_VERTEX_CAPACITY];
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

typedef struct KfOpeningSceneCells {
    KfCellWindow windows[KF_CELL_WINDOW_YAW_COUNT];
    u8 unknown_cc0[0x30];
    KfMapGrid collision_flags;
} KfOpeningSceneCells;
typedef union KfOpeningCellStorage {
    u32 rtbl_sectors[2][KF_CD_SECTOR_WORDS];
    KfOpeningSceneCells scene;
} KfOpeningCellStorage;
extern KfOpeningCellStorage opening_cell_storage;
extern u32 primitive_allocation_count;

extern void display_initialize(KfOverlayMode overlay_mode);
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
    const KfScreenRect *rectangle, const CVECTOR *color, u16 ot_index);
extern void sprite_add_ft4(const KfScreenRect *rectangle, u8 *texcoords, u32 tpage, u32 clut,
                           const CVECTOR *color, u16 ot_index);
extern void opening_fade_in(void);
extern void render_set_view_transform(
    const VECTOR *position_or_null, const SVECTOR *rotation_or_null);
extern void tmd_project_vertices_perspective_right(s32 count);
extern void tmd_project_vertices_shift(s32 count, u8 shift);
extern void tmd_transform_vertices(s32 count);
extern void render_enqueue_tmd(u16 object_index, s16 depth_bias);
extern void render_enqueue_unlit_triangles(u16 object_index, s16 depth_bias);
extern void render_enqueue_sprite(
    KfSpriteQuad *sprite, s16 depth_bias, KfSpriteDepthCueMode depth_cue_mode);
extern void render_floor_item(KfFloorItem *item);
extern void render_map_cell(s32 col, s32 row, KfCellVisibility visibility);
extern void render_enqueue_map(u16 object_index);

#endif
