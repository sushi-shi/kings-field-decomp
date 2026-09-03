#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Two shared menu ordering-table primitives: double-buffered POLY_FT4 quads
 * that the surrounding panels prime and these helpers link into the current
 * frame's ordering table at a fixed depth.  BSS; the quad bodies are filled
 * elsewhere, so both helpers only select the active buffer's copy and enqueue
 * it.  Both are shared by the magic, list, save and load panels.
 */

/* Link the shared mid-depth menu quad at ordering-table slot 500. */
ADDRESS(0x80027e58, 0x48)
void menu_add_marker_quad(void)
{
    AddPrim(display_state.ordering_table + 500,
            &DAT_80058228[display_state.buffer_index]);
}

/* Link the shared front menu quad at ordering-table slot 0. */
ADDRESS(0x80027ea0, 0x44)
void menu_add_frame_quad(void)
{
    AddPrim(display_state.ordering_table,
            &DAT_80058278[display_state.buffer_index]);
}
