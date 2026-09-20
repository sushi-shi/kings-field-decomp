#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <kf/lib/overlay.h>
#include <kf/lib/resource_file.h>
#include <kf/lib/item.h>
#include <kf/lib/memory.h>
#include <kf/open/render.h>
#include <kf/open/resources.h>
#include <kf/lib/graphics.h>

enum {
    DISPLAY_ASSET_BUFFER_BYTES = 2 * 0x26160,
    FLOOR_ITEM_TPAGE_X = 832,
    FLOOR_ITEM_PALETTE_Y = 488
};

MATRIX color_matrix_table[KF_OPEN_COLOR_PRESET_COUNT] = {
    {{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},
    {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},
    {{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},
    {{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},
    {{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},
};

MATRIX floor_item_light_matrix = {
    {{0, 0, KF_FIXED12_ONE}, {0, 0, KF_FIXED12_ONE}, {0, 0, 0}}, {0, 0, 0},
};

KfGraphicsRuntimeOpen open_graphics_runtime;

void render_initialize(void)
{
    SVECTOR angles;
    u8 *buffer;

    open_graphics_runtime.display_state.buffer_index = KF_DISPLAY_BUFFER_UNINITIALIZED;
    if (resource_file_load_into(opening_cell_storage.rtbl_sectors,
            sizeof opening_cell_storage.rtbl_sectors, "B0/RTBL.") != KF_RESOURCE_LOADED)
        exit(1);
    buffer = (u8 *)memory_allocate(DISPLAY_ASSET_BUFFER_BYTES);
    open_graphics_runtime.display_state.asset_load_buffer = buffer;
    open_graphics_runtime.display_state.asset_load_capacity = DISPLAY_ASSET_BUFFER_BYTES;
    open_graphics_runtime.floor_item_state.count = 0;
    setVector(&angles, 0, 0, 0);
    kf::matrix_set_rotation_xyz(angles, open_graphics_runtime.render_state.quadrant_matrices[0]);
    angles.vy = KF_ANGLE_THREE_QUARTER_TURN;
    kf::matrix_set_rotation_xyz(angles, open_graphics_runtime.render_state.quadrant_matrices[3]);
    angles.vy = KF_ANGLE_HALF_TURN;
    kf::matrix_set_rotation_xyz(angles, open_graphics_runtime.render_state.quadrant_matrices[2]);
    angles.vy = KF_ANGLE_QUARTER_TURN;
    kf::matrix_set_rotation_xyz(angles, open_graphics_runtime.render_state.quadrant_matrices[1]);
    open_graphics_runtime.render_state.light_matrix.m[0][0] = 3800;
    open_graphics_runtime.render_state.light_matrix.m[0][1] = -2800;
    open_graphics_runtime.render_state.light_matrix.m[0][2] = 0;
    open_graphics_runtime.render_state.light_matrix.m[1][0] = -3000;
    open_graphics_runtime.render_state.light_matrix.m[1][1] = -3600;
    open_graphics_runtime.render_state.light_matrix.m[1][2] = -3400;
    open_graphics_runtime.render_state.light_matrix.m[2][0] = -1300;
    open_graphics_runtime.render_state.light_matrix.m[2][1] = 2700;
    open_graphics_runtime.render_state.light_matrix.m[2][2] = 800;
    kf::matrix_multiply_rotation(open_graphics_runtime.render_state.light_matrix, open_graphics_runtime.render_state.quadrant_matrices[0], open_graphics_runtime.light_quadrant_matrices[0]);
    kf::matrix_multiply_rotation(open_graphics_runtime.render_state.light_matrix, open_graphics_runtime.render_state.quadrant_matrices[1], open_graphics_runtime.light_quadrant_matrices[1]);
    kf::matrix_multiply_rotation(open_graphics_runtime.render_state.light_matrix, open_graphics_runtime.render_state.quadrant_matrices[2], open_graphics_runtime.light_quadrant_matrices[2]);
    kf::matrix_multiply_rotation(open_graphics_runtime.render_state.light_matrix, open_graphics_runtime.render_state.quadrant_matrices[3], open_graphics_runtime.light_quadrant_matrices[3]);
    open_graphics_runtime.floor_item_state.texture = {kf::SurfaceKind::Texture,
        {FLOOR_ITEM_TPAGE_X, 0, 0, FLOOR_ITEM_PALETTE_Y, kf::TextureFormat::Indexed8},
        kf::BlendMode::average};
}

void display_initialize(KfOverlayMode overlay_mode)
{
    open_graphics_runtime.display_state.frame_style = {};
    open_graphics_runtime.render_state.projection = {};
    // A new intro starts black; the ending retains the game's transition image.
    if (overlay_mode == KF_OVERLAY_MODE_INTRO) {
        kf::FaceList blank {};
        kf::host_present_faces(&blank);
    }
    open_graphics_runtime.render_state.lighting.ambient = {0, 0, 0};
    lighting_set_active_color_matrix(KF_OPEN_COLOR_DEFAULT);
    open_graphics_runtime.render_state.lighting.fog = {0, 0, 0};
    open_graphics_runtime.render_state.fog_near_distance = KF_INITIAL_FOG_NEAR_DISTANCE;
    open_graphics_runtime.render_state.projection.fog_near = KF_INITIAL_FOG_NEAR_DISTANCE;
    open_graphics_runtime.tmd_projection_shift = KF_TMD_DEFAULT_PERSPECTIVE_SHIFT;
    render_initialize();
}

void render_init_reset_module_state(void)
{
    kf::restore_initial_value<color_matrix_table>();
    kf::restore_initial_value<floor_item_light_matrix>();
    kf::restore_initial_value<open_graphics_runtime>();
}
