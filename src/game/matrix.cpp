#include <kf/game_graphics.h>

#include <kf/game_math.h>
#include <kf/game_render.h>
#include <kf/game.h>

void fog_interpolate_near(s32 start, s32 end, s32 ratio)
{
    s32 distance = (((end - start) * ratio) >> KF_FIXED12_BITS) + start;

    game_graphics_runtime.render_state.fog_near_distance = distance;
    SetFogNear(distance, KF_DEFAULT_PROJECTION_DISTANCE);
}

void fog_set_near(s32 distance)
{
    game_graphics_runtime.render_state.fog_near_distance = distance;
    SetFogNear(distance, KF_DEFAULT_PROJECTION_DISTANCE);
}
