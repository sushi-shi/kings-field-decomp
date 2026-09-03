#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfDisplayState display_state;
extern DRAWENV display_draw_environments[2];
extern DISPENV display_disp_environments[2];

/*
 * Menu/hub copy of the frame flush: wait for the GPU and vertical retrace,
 * swap in the draw/display environments for the active buffer, then hand the
 * ordering table to the GPU.  Byte-identical to display_present_frame; the
 * hub and item panels call this local copy.
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
