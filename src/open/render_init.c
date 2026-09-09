#include <kf/address.h>
#include <kf/psyq_libc.h>
#include <kf/overlay.h>
#include <kf/cd_file.h>
#include <kf/item.h>
#include <kf/memory.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>

enum {
    PRIMITIVE_BUFFER_BYTES = 0x26160,
    FLOOR_ITEM_TPAGE_X = 832,
    FLOOR_ITEM_CLUT = 0x7a00
};

DATA(0x80035944, 0xa0)
MATRIX color_matrix_table[KF_OPEN_COLOR_PRESET_COUNT] = {
    {{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},
    {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},
    {{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},
    {{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},
    {{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},
};

DATA(0x80049a48, 0x24788)
KfGraphicsRuntimeOpen open_graphics_runtime;

DATA(0x80075928, 0x4)
u32 primitive_allocation_count;

/*
 * OPEN.EXE render initialisation. The colour-preset selector, display setup,
 * and primitive allocator share the same matrix/display state and direct
 * initialization flow. Shared state and the GAME homolog support this WIP
 * render-family unit, not an original TU boundary. This variant loads
 * B0\RTBL., budgets a larger primitive buffer, keeps no light-matrix copy,
 * and sets one texture page.
 */

RODATA(0x80012110, 0x28)

ADDRESS(0x800168dc, 0x2c)
void lighting_set_active_color_matrix(KfOpenColorPreset preset)
{
    SetColorMatrix(&color_matrix_table[KF_ENUM_ENCODE(s32, preset)]);
}

ADDRESS(0x80016908, 0x1d4)
void render_initialize(void)
{
    SVECTOR angles;
    u8 *buffer;

    open_graphics_runtime.display_state.buffer_index = KF_DISPLAY_BUFFER_UNINITIALIZED;
    cd_file_load_into(render_cell_windows, "B0\\RTBL.");
    buffer = (u8 *)memory_allocate(KF_DISPLAY_BUFFER_COUNT * PRIMITIVE_BUFFER_BYTES);
    open_graphics_runtime.display_state.asset_load_buffer = buffer;
    open_graphics_runtime.display_state.primitive_buffers[0].start = buffer;
    buffer += PRIMITIVE_BUFFER_BYTES;
    open_graphics_runtime.display_state.primitive_buffers[0].end = buffer;
    open_graphics_runtime.display_state.primitive_buffers[1].start = buffer;
    buffer += PRIMITIVE_BUFFER_BYTES;
    open_graphics_runtime.display_state.primitive_buffers[1].end = buffer;
    open_graphics_runtime.floor_item_state.count = 0;
    setVector(&angles, 0, 0, 0);
    RotMatrix(&angles, &open_graphics_runtime.render_state.quadrant_matrices[0]);
    angles.vy = KF_ANGLE_THREE_QUARTER_TURN;
    RotMatrix(&angles, &open_graphics_runtime.render_state.quadrant_matrices[3]);
    angles.vy = KF_ANGLE_HALF_TURN;
    RotMatrix(&angles, &open_graphics_runtime.render_state.quadrant_matrices[2]);
    angles.vy = KF_ANGLE_QUARTER_TURN;
    RotMatrix(&angles, &open_graphics_runtime.render_state.quadrant_matrices[1]);
    open_graphics_runtime.render_state.light_matrix.m[0][0] = 3800;
    open_graphics_runtime.render_state.light_matrix.m[0][1] = -2800;
    open_graphics_runtime.render_state.light_matrix.m[0][2] = 0;
    open_graphics_runtime.render_state.light_matrix.m[1][0] = -3000;
    open_graphics_runtime.render_state.light_matrix.m[1][1] = -3600;
    open_graphics_runtime.render_state.light_matrix.m[1][2] = -3400;
    open_graphics_runtime.render_state.light_matrix.m[2][0] = -1300;
    open_graphics_runtime.render_state.light_matrix.m[2][1] = 2700;
    open_graphics_runtime.render_state.light_matrix.m[2][2] = 800;
    MulMatrix0(
        &open_graphics_runtime.render_state.light_matrix,
        &open_graphics_runtime.render_state.quadrant_matrices[0],
        &open_graphics_runtime.light_quadrant_matrices[0]);
    MulMatrix0(
        &open_graphics_runtime.render_state.light_matrix,
        &open_graphics_runtime.render_state.quadrant_matrices[1],
        &open_graphics_runtime.light_quadrant_matrices[1]);
    MulMatrix0(
        &open_graphics_runtime.render_state.light_matrix,
        &open_graphics_runtime.render_state.quadrant_matrices[2],
        &open_graphics_runtime.light_quadrant_matrices[2]);
    MulMatrix0(
        &open_graphics_runtime.render_state.light_matrix,
        &open_graphics_runtime.render_state.quadrant_matrices[3],
        &open_graphics_runtime.light_quadrant_matrices[3]);
    open_graphics_runtime.floor_item_state.texture_tpage = GetTPage(
        KF_GPU_TEXTURE_8BIT, KF_GPU_BLEND_AVERAGE,
        FLOOR_ITEM_TPAGE_X, 0);
    open_graphics_runtime.floor_item_state.texture_clut = FLOOR_ITEM_CLUT;
}

ADDRESS(0x80016adc, 0x1d8)
void display_initialize(KfOpenMode mode)
{
    s32 framebuffer_height;
    s16 lower_buffer_y = KF_DISPLAY_HEIGHT;

    if (mode == KF_OPEN_MODE_ENDING) {
        ResetGraph(KF_GPU_RESET_KEEP_DISPLAY);
    } else {
        ResetGraph(KF_GPU_RESET_FULL);
    }
    framebuffer_height = KF_DISPLAY_HEIGHT;
    InitGeom();
    SetGeomOffset(KF_DISPLAY_WIDTH / 2, KF_DISPLAY_HEIGHT / 2);
    SetDefDrawEnv(
        &open_graphics_runtime.display_draw_environments[0], 0, 0,
        KF_DISPLAY_WIDTH, framebuffer_height);
    SetDefDispEnv(
        &open_graphics_runtime.display_disp_environments[0],
        0,
        lower_buffer_y,
        KF_DISPLAY_WIDTH,
        framebuffer_height);
    SetDefDrawEnv(
        &open_graphics_runtime.display_draw_environments[1],
        0,
        lower_buffer_y,
        KF_DISPLAY_WIDTH,
        framebuffer_height);
    SetDefDispEnv(
        &open_graphics_runtime.display_disp_environments[1], 0, 0,
        KF_DISPLAY_WIDTH, framebuffer_height);
    open_graphics_runtime.display_draw_environments[0].dtd = open_graphics_runtime.display_draw_environments[1].dtd = 1;
    open_graphics_runtime.display_draw_environments[0].isbg = 1;
    open_graphics_runtime.display_draw_environments[1].isbg = 1;
    setRGB0(&open_graphics_runtime.display_draw_environments[0], 0, 0, 0);
    setRGB0(&open_graphics_runtime.display_draw_environments[1], 0, 0, 0);
    if (mode == KF_OPEN_MODE_ENDING) {
        PutDispEnv(&open_graphics_runtime.display_disp_environments[0]);
        SetDispMask(1);
    } else {
        open_graphics_runtime.display_draw_environments[0].dfe = 0;
        open_graphics_runtime.display_draw_environments[1].dfe = 0;
        PutDrawEnv(&open_graphics_runtime.display_draw_environments[0]);
        PutDrawEnv(&open_graphics_runtime.display_draw_environments[1]);
        open_graphics_runtime.display_draw_environments[0].dfe = 1;
        open_graphics_runtime.display_draw_environments[1].dfe = 1;
        SetDispMask(0);
    }
    SetBackColor(0, 0, 0);
    lighting_set_active_color_matrix(KF_OPEN_COLOR_DEFAULT);
    SetFarColor(0, 0, 0);
    open_graphics_runtime.render_state.fog_near_distance = KF_INITIAL_FOG_NEAR_DISTANCE;
    SetFogNear(KF_INITIAL_FOG_NEAR_DISTANCE, KF_DEFAULT_PROJECTION_DISTANCE);
    open_graphics_runtime.tmd_projection_shift = KF_TMD_DEFAULT_PERSPECTIVE_SHIFT;
    render_initialize();
}

ADDRESS(0x80016cb4, 0x84)
void *primitive_buffer_allocate(u16 byte_count)
{
    u8 *allocation = open_graphics_runtime.display_state.primitive_buffer->cursor;

    open_graphics_runtime.display_state.primitive_buffer->cursor += byte_count;
    if (open_graphics_runtime.display_state.primitive_buffer->cursor >
        open_graphics_runtime.display_state.primitive_buffer->end) {
        for (;;) {
            printf("primitive over fllow!!!\n");
        }
    }
    primitive_allocation_count++;
    return allocation;
}
