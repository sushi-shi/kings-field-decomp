#include <kf/address.h>
#include <kf/semantic_types.h>
#include <SYS/TYPES.H>
#include <LIBGTE.H>
#include <LIBGPU.H>

extern DRAWENV display_draw_environments[2];
extern DISPENV display_disp_environments[2];
extern KfRenderState render_state;

extern u32 func_80050544(u32 mode);
extern void InitGeom(void);
extern void SetGeomOffset(s32 ofx, s32 ofy);
extern void SetBackColor(s32 r, s32 g, s32 b);
extern void SetFarColor(s32 r, s32 g, s32 b);
extern void SetFogNear(s32 distance, s32 projection);
extern void func_8001bab8(s32 index);
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
    func_8001bab8(0);
    SetFarColor(0, 0, 0);
    render_state.fog_near_distance = 0x2af8;
    SetFogNear(0x2af8, 200);
    render_initialize();
}
