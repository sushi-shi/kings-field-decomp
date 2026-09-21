#ifndef KF_OPEN_RENDER_H
#define KF_OPEN_RENDER_H

#include <kf/lib/graphics.h>

#include <kf/lib/enum.h>
#include <kf/lib/resource_file.h>
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

typedef struct KfTmdStateOpen {
    KfTmdResource slots[KF_OPEN_TMD_SLOT_COUNT];
    KfTmdResource current_asset;
} KfTmdStateOpen;

typedef struct KfSpriteMaterial {
    kf::FaceMaterial surface;
    CVECTOR color;
} KfSpriteMaterial;

typedef struct KfFloorItemStateOpen {
    KfSpriteMaterial material;
    u8 unknown_08[6];
    kf::FaceMaterial texture;
    u16 count;
    u8 unknown_14[4];
    KfFloorItem items[KF_FLOOR_ITEM_CAPACITY];
} KfFloorItemStateOpen;

typedef struct KfGraphicsRuntimeOpen {
    KfDisplayState display_state;
    u8 unknown_20108[8];
    KfTmdStateOpen tmd_state;
    u8 unknown_2011c[4];
    SVECTOR *current_tmd_vertices;
    u8 unknown_20124[0x14];
    KfScreenVertex tmd_projected_vertices[KF_PROJECTED_VERTEX_CAPACITY];
    u8 unknown_22078[0x1f68];
    KfFloorItemStateOpen floor_item_state;
    KfRenderState render_state;
    MATRIX light_quadrant_matrices[KF_VIEW_QUADRANT_COUNT];
    const KfCellWindow *active_cell_window;
    s16 tmd_projection_shift;
    u8 unknown_24786[2];
} KfGraphicsRuntimeOpen;
extern KfGraphicsRuntimeOpen open_graphics_runtime;

inline KfScreenVertex *tmd_projected_vertex(u16 index)
{
    return &open_graphics_runtime.tmd_projected_vertices[index];
}

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
    u32 rtbl_sectors[2][512];
    KfOpeningSceneCells scene;
} KfOpeningCellStorage;
extern KfOpeningCellStorage opening_cell_storage;

extern void display_initialize(KfOverlayMode overlay_mode);
extern void lighting_set_active_color_matrix(KfOpenColorPreset preset);
extern void render_initialize(void);
extern void opening_fade_in(void);
extern void tmd_project_vertices_perspective_right(s32 count, const MATRIX *model, const kf::Projection &projection);
extern void render_enqueue_tmd(u16 object_index, s16 depth_bias, const MATRIX *lights);
extern void render_enqueue_unlit_triangles(u16 object_index, s16 depth_bias);
extern void render_enqueue_sprite(KfSpriteQuad *sprite, s16 depth_bias, KfSpriteDepthCueMode depth_cue_mode, const MATRIX *lights, const MATRIX *model, const kf::Projection &projection);
extern void render_map_cell(s32 col, s32 row, KfCellVisibility visibility);
extern void render_enqueue_map(u16 object_index, const MATRIX *lights, const MATRIX *model, const kf::Projection &projection);

extern void tmd_project_vertices(s32 count, const MATRIX *model, const kf::Projection &projection);

inline KfTmdContext tmd_context()
{
    return {open_graphics_runtime.tmd_state.slots, open_graphics_runtime.tmd_state.current_asset,
        open_graphics_runtime.current_tmd_vertices, open_graphics_runtime.tmd_projected_vertices};
}

inline KfFloorItemStorage floor_item_storage()
{
    return {open_graphics_runtime.floor_item_state.count, open_graphics_runtime.floor_item_state.items};
}

#endif
