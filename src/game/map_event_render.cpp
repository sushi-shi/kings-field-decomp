#include <kf/null.h>
#include <kf/game_graphics.h>

#include <kf/game_asset.h>
#include <kf/game_render.h>
#include <psyq/sdk.h>

void render_map_event(KfMapEvent *event)
{
    SVECTOR screen;
    MATRIX model;
    MATRIX composed;
    long flag;
    u16 asset;
    KfTmdObject *object;

    SetRotMatrix(&game_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&game_graphics_runtime.render_state.view_matrix);
    setVector(&screen,
        event->reference_position.vx - game_graphics_runtime.render_state.view_position.vx,
        event->reference_position.vy - game_graphics_runtime.render_state.view_position.vy,
        event->reference_position.vz - game_graphics_runtime.render_state.view_position.vz);
    RotTrans(&screen, (VECTOR *)&composed.t, &flag);
    RotMatrix(&event->rotation, &model);
    MulMatrix0(&game_graphics_runtime.render_state.view_matrix, &model, &composed);
    SetRotMatrix(&composed);
    SetTransMatrix(&composed);
    asset = event->model_index + KF_ASSET_MAP_EVENT_FIRST;
    asset_registry_select(asset);
    object = tmd_get_object(0);
    if (render_bind_animated_instance(
            &event->animation_cache, asset, event->animation_clip, event->animation_phase,
            object->vertex_count) == NULL) {
        tmd_select_object_vertices(0);
        tmd_project_vertices(tmd_get_object(0)->vertex_count);
    } else {
        tmd_project_vertices(object->vertex_count);
    }
    render_enqueue_tmd(0, 0);
}
