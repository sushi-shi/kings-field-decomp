#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Begin a menu frame: flip the double buffer, re-anchor the active primitive
 * buffer and ordering table for the new buffer index, clear the ordering
 * table, and rewind the primitive-buffer cursor.  This is display_begin_frame
 * without the three gameplay per-frame counters that only the in-game path
 * resets, so the menu loop starts a clean ordering table without disturbing
 * them.
 */
ADDRESS(0x8002abb4, 0x80)
void menu_frame_begin(void)
{
    display_state.buffer_index = display_state.buffer_index == 0;
    display_state.primitive_buffer = &display_state.primitive_buffers[display_state.buffer_index];
    display_state.ordering_table =
        display_state.ordering_tables[display_state.buffer_index].entries;
    ClearOTagR(display_state.ordering_table, 0x4000);
    display_state.primitive_buffer->cursor = display_state.primitive_buffer->start;
}
