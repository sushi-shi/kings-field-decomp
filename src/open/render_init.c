#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * OPEN.EXE render initialisation.  The translation unit continues in
 * src/open/render.c; the two unlabelled helpers linked between them (display
 * environment setup at 0x80016adc and the primitive allocator at 0x80016cb4)
 * are not reconstructed yet, so the run is split into two units.  Compared
 * with GAME.EXE this snapshot loads B0\RTBL., budgets a larger primitive
 * buffer, keeps no light-matrix copy, and sets one texture page.
 */

extern void *memory_allocate(s32 size);
extern u32 func_8001615c(void *destination, char *path);

extern KfDisplayStateOpen display_state;
extern KfRenderStateOpen render_state;
extern MATRIX light_quadrant_matrices[4];
extern u16 DAT_8006da3a;
extern u16 DAT_8006da38;
extern u16 DAT_8006da36;
extern u32 DAT_800439d8;

RODATA(0x80012110, 0xc)

ADDRESS(0x80016908, 0x1d4)
void render_initialize(void)
{
    SVECTOR angles;
    u8 *buffer;

    display_state.buffer_index = 0xff;
    func_8001615c(&DAT_800439d8, "B0\\RTBL.");
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
