#include <kf/address.h>
#include <kf/semantic_types.h>
#include <SYS/TYPES.H>
#include <LIBGTE.H>
#include <LIBGPU.H>

extern KfDisplayState display_state;
extern DRAWENV display_draw_environments[2];
extern DISPENV display_disp_environments[2];
extern u32 DAT_800a0768;
extern u32 DAT_8009569c;
extern u32 DAT_80095698;

ADDRESS(0x8001bfb8, 0x98)
void display_begin_frame(void)
{
    display_state.buffer_index = display_state.buffer_index == 0;
    display_state.primitive_buffer = &display_state.primitive_buffers[display_state.buffer_index];
    display_state.ordering_table =
        display_state.ordering_tables[display_state.buffer_index].entries;
    ClearOTagR(display_state.ordering_table, 0x4000);
    display_state.primitive_buffer->cursor = display_state.primitive_buffer->start;
    DAT_800a0768 = 0;
    DAT_8009569c = 0;
    DAT_80095698 = 0;
}

ADDRESS(0x8001c050, 0x98)
void display_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&display_draw_environments[display_state.buffer_index]);
    PutDispEnv(&display_disp_environments[display_state.buffer_index]);
    DrawOTag(display_state.ordering_table + 0x3fff);
}
