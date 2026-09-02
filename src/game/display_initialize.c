#include <kf/address.h>
#include <kf/semantic_types.h>

extern DRAWENV display_draw_environments[2];
extern DISPENV display_disp_environments[2];
extern KfRenderState render_state;

extern u32 func_80050544(u32 mode);
extern void lighting_set_active_color_matrix(s32 index);
extern void render_initialize(void);

ADDRESS(0x8001bb94, 0x14c)
void func_8001bb94(void)
{
    func_80050544(3);
    InitGeom();
    SetGeomOffset(160, 120);
    SetDefDrawEnv(&display_draw_environments[0], 0, 0, 320, 240);
    SetDefDispEnv(&display_disp_environments[0], 0, 240, 320, 240);
    SetDefDrawEnv(&display_draw_environments[1], 0, 240, 320, 240);
    SetDefDispEnv(&display_disp_environments[1], 0, 0, 320, 240);
    display_draw_environments[0].dtd = display_draw_environments[1].dtd = 1;
    display_draw_environments[0].isbg = 1;
    display_draw_environments[1].isbg = 1;
    display_draw_environments[0].r0 = 0;
    display_draw_environments[0].g0 = 0;
    display_draw_environments[0].b0 = 0;
    display_draw_environments[1].r0 = 0;
    display_draw_environments[1].g0 = 0;
    display_draw_environments[1].b0 = 0;
    PutDispEnv(&display_disp_environments[0]);
    SetBackColor(60, 60, 60);
    lighting_set_active_color_matrix(0);
    SetFarColor(0, 0, 0);
    render_state.fog_near_distance = 0x2af8;
    SetFogNear(0x2af8, 200);
    render_initialize();
}
