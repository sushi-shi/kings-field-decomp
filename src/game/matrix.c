#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_math.h>
#include <kf/game_render.h>
#include <kf/game.h>

/*
 * matrix_interpolate, lighting_set_color_matrix, and lighting_set_light_matrix
 * are identical in GAME.EXE and OPEN.EXE and live in src/shared/matrix_lerp.c.
 */

ADDRESS(0x800203bc, 0x44)
void fog_interpolate_near(s32 start, s32 end, s32 ratio)
{
    s32 distance = (((end - start) * ratio) >> KF_FIXED12_BITS) + start;

    game_graphics_runtime.render_state.fog_near_distance = distance;
    SetFogNear(distance, KF_DEFAULT_PROJECTION_DISTANCE);
}

ADDRESS(0x80020400, 0x28)
void fog_set_near(s32 distance)
{
    game_graphics_runtime.render_state.fog_near_distance = distance;
    SetFogNear(distance, KF_DEFAULT_PROJECTION_DISTANCE);
}
