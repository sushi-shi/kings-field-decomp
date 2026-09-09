#include <kf/address.h>
#include <kf/psyq_pad.h>
#include <kf/open_render.h>
#include <LIBETC.H>

enum {
    VRAM_VIEW_PAN_STEP = 4
};

ADDRESS(0x8001a82c, 0x248)
void display_adjust_vram_view(void)
{
    RECT saved;
    DISPENV *display;
    u32 buttons;

    open_graphics_runtime.display_draw_environments[0].isbg = 0;
    open_graphics_runtime.display_draw_environments[1].isbg = 0;
    open_graphics_runtime.display_state.buffer_index = display_next_buffer(open_graphics_runtime.display_state.buffer_index);
    DrawSync(0);
    VSync(0);
    PutDispEnv(&open_graphics_runtime.display_disp_environments[KF_ENUM_ENCODE(u8, open_graphics_runtime.display_state.buffer_index)]);
    open_graphics_runtime.display_draw_environments[KF_ENUM_ENCODE(u8, open_graphics_runtime.display_state.buffer_index)].dfe = 0;
    PutDrawEnv(&open_graphics_runtime.display_draw_environments[KF_ENUM_ENCODE(u8, open_graphics_runtime.display_state.buffer_index)]);
    display = &open_graphics_runtime.display_disp_environments[KF_ENUM_ENCODE(u8, open_graphics_runtime.display_state.buffer_index)];
    saved = display->disp;

    while (PadRead(1) & PADh) {
    }
    for (;;) {
        buttons = PadRead(1);
        if (buttons & PADLright) {
            display->disp.x += VRAM_VIEW_PAN_STEP;
        }
        if (buttons & PADLleft) {
            display->disp.x -= VRAM_VIEW_PAN_STEP;
        }
        if (buttons & PADLup) {
            display->disp.y -= VRAM_VIEW_PAN_STEP;
        }
        if (buttons & PADLdown) {
            display->disp.y += VRAM_VIEW_PAN_STEP;
        }
        if (buttons & PADh) {
            break;
        }
        display->disp.x &= KF_VRAM_WIDTH - 1;
        display->disp.y &= KF_VRAM_HEIGHT - 1;
        VSync(0);
        PutDispEnv(display);
    }
    while (PadRead(1) & PADh) {
    }
    display->disp = saved;
    open_graphics_runtime.display_draw_environments[KF_ENUM_ENCODE(u8, open_graphics_runtime.display_state.buffer_index)].dfe = 1;
    open_graphics_runtime.display_draw_environments[0].isbg = 1;
    open_graphics_runtime.display_draw_environments[1].isbg = 1;
}
