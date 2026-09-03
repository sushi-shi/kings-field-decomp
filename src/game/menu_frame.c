#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

DATA(0x80057e88, 0x4)
POLY_FT4 *current_poly_ft4 = 0;

/*
 * Begin a menu frame: flip the double buffer, re-anchor the active primitive
 * buffer and ordering table for the new buffer index, clear the ordering
 * table, and rewind the primitive-buffer cursor. This is display_begin_frame
 * without the three gameplay per-frame counters that only the in-game path
 * resets, so the menu loop starts a clean ordering table without disturbing
 * them.
 */
ADDRESS(0x8002abb4, 0x80)
void menu_frame_begin(void)
{
    display_state.buffer_index = display_state.buffer_index == 0;
    display_state.primitive_buffer =
        &display_state.primitive_buffers[display_state.buffer_index];
    display_state.ordering_table =
        display_state.ordering_tables[display_state.buffer_index].entries;
    ClearOTagR(display_state.ordering_table, 0x4000);
    display_state.primitive_buffer->cursor =
        display_state.primitive_buffer->start;
}

/*
 * Menu/hub copy of the frame flush: wait for the GPU and vertical retrace,
 * swap in the draw/display environments for the active buffer, then hand the
 * ordering table to the GPU. Byte-identical to display_present_frame; the hub
 * and item panels call this local copy.
 */
ADDRESS(0x8002ac34, 0x98)
void menu_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&display_draw_environments[display_state.buffer_index]);
    PutDispEnv(&display_disp_environments[display_state.buffer_index]);
    DrawOTag(display_state.ordering_table + 0x3fff);
}

ADDRESS(0x8002accc, 0x50)
void primitive_buffer_begin_poly_ft4(void)
{
    SetPolyFT4(current_poly_ft4);
    current_poly_ft4->r0 = 0x60;
    current_poly_ft4->g0 = 0x60;
    current_poly_ft4->b0 = 0x60;
}

ADDRESS(0x8002ad1c, 0x50)
void primitive_buffer_commit_poly_ft4(s32 depth)
{
    depth <<= 2;
    AddPrim(
        (u32 *)((u8 *)display_state.ordering_table + depth),
        current_poly_ft4);
    current_poly_ft4++;
    display_state.primitive_buffer->cursor = (u8 *)current_poly_ft4;
}
