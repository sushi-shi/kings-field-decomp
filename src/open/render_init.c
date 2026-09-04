#include <kf/address.h>
#include <kf/cd_file.h>
#include <kf/memory.h>
#include <kf/open_render.h>

/*
 * OPEN.EXE render initialisation. The two functions linked after this body
 * (display-environment setup at 0x80016adc and the primitive allocator at
 * 0x80016cb4) are not reconstructed, so this remains a separate WIP unit.
 * Shared state and the GAME homolog support the render-family interface, not
 * an original TU boundary. This variant loads B0\RTBL., budgets a larger
 * primitive buffer, keeps no light-matrix copy, and sets one texture page.
 */

RODATA(0x80012110, 0xc)

ADDRESS(0x80016908, 0x1d4)
void render_initialize(void)
{
    SVECTOR angles;
    u8 *buffer;

    display_state.buffer_index = 0xff;
    cd_file_load_into(&DAT_800439d8, "B0\\RTBL.");
    buffer = memory_allocate(0x4c2c0);
    display_state.asset_load_buffer = buffer;
    display_state.primitive_buffers[0].start = buffer;
    buffer += 0x26160;
    display_state.primitive_buffers[0].end = buffer;
    display_state.primitive_buffers[1].start = buffer;
    buffer += 0x26160;
    display_state.primitive_buffers[1].end = buffer;
    DAT_8006da3a = 0;
    angles.vx = 0;
    angles.vy = 0;
    angles.vz = 0;
    RotMatrix(&angles, (MATRIX *)&render_state.quadrant_matrices[0]);
    angles.vy = 0xc00;
    RotMatrix(&angles, (MATRIX *)&render_state.quadrant_matrices[3]);
    angles.vy = 0x800;
    RotMatrix(&angles, (MATRIX *)&render_state.quadrant_matrices[2]);
    angles.vy = 0x400;
    RotMatrix(&angles, (MATRIX *)&render_state.quadrant_matrices[1]);
    render_state.light_matrix.m[0][0] = 3800;
    render_state.light_matrix.m[0][1] = -2800;
    render_state.light_matrix.m[0][2] = 0;
    render_state.light_matrix.m[1][0] = -3000;
    render_state.light_matrix.m[1][1] = -3600;
    render_state.light_matrix.m[1][2] = -3400;
    render_state.light_matrix.m[2][0] = -1300;
    render_state.light_matrix.m[2][1] = 2700;
    render_state.light_matrix.m[2][2] = 800;
    MulMatrix0(
        (MATRIX *)&render_state.light_matrix,
        (MATRIX *)&render_state.quadrant_matrices[0],
        (MATRIX *)&light_quadrant_matrices[0]);
    MulMatrix0(
        (MATRIX *)&render_state.light_matrix,
        (MATRIX *)&render_state.quadrant_matrices[1],
        (MATRIX *)&light_quadrant_matrices[1]);
    MulMatrix0(
        (MATRIX *)&render_state.light_matrix,
        (MATRIX *)&render_state.quadrant_matrices[2],
        (MATRIX *)&light_quadrant_matrices[2]);
    MulMatrix0(
        (MATRIX *)&render_state.light_matrix,
        (MATRIX *)&render_state.quadrant_matrices[3],
        (MATRIX *)&light_quadrant_matrices[3]);
    DAT_8006da38 = GetTPage(1, 0, 0x340, 0);
    DAT_8006da36 = 0x7a00;
}
