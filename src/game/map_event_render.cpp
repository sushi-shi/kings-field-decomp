#include <kf/lib/null.h>
#include <kf/game/graphics.h>

#include <kf/game/asset.h>
#include <kf/game/render.h>
#include <kf/lib/geometry_types.h>

void render_map_event(KfMapEvent *event, const MATRIX *lights)
{
    SVECTOR screen;
    MATRIX model;
    MATRIX composed;
    u16 asset;
    KfTmdObject *object;

    vector_set_xyz(screen,
        event->reference_position.vx - game_graphics_runtime.render_state.view_position.vx,
        event->reference_position.vy - game_graphics_runtime.render_state.view_position.vy,
        event->reference_position.vz - game_graphics_runtime.render_state.view_position.vz);
    kf::render_place_model(composed, game_graphics_runtime.render_state.view_matrix, screen);
    kf::matrix_set_rotation_xyz(event->rotation, model);
    kf::matrix_multiply_rotation(game_graphics_runtime.render_state.view_matrix, model, composed);
    asset = event->model_index + KF_ASSET_MAP_EVENT_FIRST;
    asset_registry_select(asset);
    object = tmd_get_object(0);
    if (render_bind_animated_instance(
            &event->animation_cache, asset, event->animation_clip, event->animation_phase,
            object->vertex_count) == NULL) {
        tmd_select_object_vertices(0);
        tmd_project_vertices(tmd_get_object(0)->vertex_count, &composed, game_graphics_runtime.render_state.projection);
    } else {
        tmd_project_vertices(object->vertex_count, &composed, game_graphics_runtime.render_state.projection);
    }
    render_enqueue_tmd(0, 0, lights);
}
