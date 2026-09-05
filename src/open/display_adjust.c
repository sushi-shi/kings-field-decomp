#include <kf/address.h>
#include <kf/open_render.h>
#include <LIBETC.H>

ADDRESS(0x8001a82c, 0x248)
void display_adjust_vram_view(void)
{
    RECT saved;
    DISPENV *display;
    u32 buttons;

    open_graphics_runtime.display_draw_environments[0].isbg = 0;
    open_graphics_runtime.display_draw_environments[1].isbg = 0;
    open_graphics_runtime.display_state.buffer_index = open_graphics_runtime.display_state.buffer_index == 0;
    DrawSync(0);
    VSync(0);
    PutDispEnv(&open_graphics_runtime.display_disp_environments[open_graphics_runtime.display_state.buffer_index]);
    open_graphics_runtime.display_draw_environments[open_graphics_runtime.display_state.buffer_index].dfe = 0;
    PutDrawEnv(&open_graphics_runtime.display_draw_environments[open_graphics_runtime.display_state.buffer_index]);
    display = &open_graphics_runtime.display_disp_environments[open_graphics_runtime.display_state.buffer_index];
    saved = display->disp;

    while (PadRead(1) & PADh) {
    }
    for (;;) {
        buttons = PadRead(1);
        if (buttons & PADLright) {
            display->disp.x += 4;
        }
        if (buttons & PADLleft) {
            display->disp.x -= 4;
        }
        if (buttons & PADLup) {
            display->disp.y -= 4;
        }
        if (buttons & PADLdown) {
            display->disp.y += 4;
        }
        if (buttons & PADh) {
            break;
        }
        display->disp.x &= 0x3ff;
        display->disp.y &= 0x1ff;
        VSync(0);
        PutDispEnv(display);
    }
    while (PadRead(1) & PADh) {
    }
    display->disp = saved;
    open_graphics_runtime.display_draw_environments[open_graphics_runtime.display_state.buffer_index].dfe = 1;
    open_graphics_runtime.display_draw_environments[0].isbg = 1;
    open_graphics_runtime.display_draw_environments[1].isbg = 1;
}
