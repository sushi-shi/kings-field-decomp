#include <kf/address.h>
#include <kf/cd_file.h>
#include <kf/item.h>
#include <kf/memory.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>

enum {
    PRIMITIVE_BUFFER_BYTES = 0x26160
};

DATA(0x80035944, 0xa0)
MATRIX color_matrix_table[5] = {
    {{{2000, 700, 4000}, {2000, 700, 4000}, {2000, 700, 4000}}, {0, 0, 0}},
    {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, {0, 0, 0}},
    {{{4095, 4095, 4095}, {4095, 4095, 4095}, {4095, 4095, 4095}}, {0, 0, 0}},
    {{{170, 682, 682}, {170, 341, 341}, {0, 0, 0}}, {0, 0, 0}},
    {{{0, 0, 0}, {375, 375, 375}, {0, 0, 0}}, {0, 0, 0}},
};

DATA(0x80049a48, 0x24788)
KfGraphicsRuntimeOpen open_graphics_runtime;

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
void lighting_set_active_color_matrix(s32 index)
{
    SetColorMatrix(&color_matrix_table[index]);
}

ADDRESS(0x80016908, 0x1d4)
void render_initialize(void)
{
    SVECTOR angles;
    u8 *buffer;

    open_graphics_runtime.display_state.buffer_index = KF_DISPLAY_BUFFER_UNINITIALIZED;
    cd_file_load_into(render_cell_windows, "B0\\RTBL.");
    buffer = memory_allocate(KF_DISPLAY_BUFFER_COUNT * PRIMITIVE_BUFFER_BYTES);
    open_graphics_runtime.display_state.asset_load_buffer = buffer;
    open_graphics_runtime.display_state.primitive_buffers[0].start = buffer;
    buffer += PRIMITIVE_BUFFER_BYTES;
    open_graphics_runtime.display_state.primitive_buffers[0].end = buffer;
    open_graphics_runtime.display_state.primitive_buffers[1].start = buffer;
    buffer += PRIMITIVE_BUFFER_BYTES;
    open_graphics_runtime.display_state.primitive_buffers[1].end = buffer;
    open_graphics_runtime.floor_item_state.count = 0;
    angles.vx = 0;
    angles.vy = 0;
    angles.vz = 0;
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
    open_graphics_runtime.floor_item_state.texture_tpage = GetTPage(1, 0, 0x340, 0);
    open_graphics_runtime.floor_item_state.texture_clut = 0x7a00;
}

ADDRESS(0x80016adc, 0x1d8)
void display_initialize(s32 mode)
{
    s32 framebuffer_height;
    s32 lower_buffer_y = KF_DISPLAY_HEIGHT;
    DRAWENV *first_draw;
    DRAWENV *second_draw;

    if (mode == 0xfe) {
        ResetGraph(3);
    } else {
        ResetGraph(0);
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
    open_graphics_runtime.display_draw_environments[0].r0 = 0;
    open_graphics_runtime.display_draw_environments[0].g0 = 0;
    open_graphics_runtime.display_draw_environments[0].b0 = 0;
    open_graphics_runtime.display_draw_environments[1].r0 = 0;
    open_graphics_runtime.display_draw_environments[1].g0 = 0;
    open_graphics_runtime.display_draw_environments[1].b0 = 0;
    first_draw = &open_graphics_runtime.display_draw_environments[0];
    second_draw = &open_graphics_runtime.display_draw_environments[1];
    if (mode == 0xfe) {
        PutDispEnv(&open_graphics_runtime.display_disp_environments[0]);
        SetDispMask(1);
    } else {
        first_draw->dfe = 0;
        second_draw->dfe = 0;
        PutDrawEnv(first_draw);
        PutDrawEnv(second_draw);
        first_draw->dfe = 1;
        second_draw->dfe = 1;
        SetDispMask(0);
    }
    SetBackColor(0, 0, 0);
    lighting_set_active_color_matrix(0);
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
