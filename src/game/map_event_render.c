#include <kf/address.h>
#include <kf/game_asset.h>
#include <kf/game_render.h>
#include <kf/psyq.h>

/*
 * Map-event model emitter invoked by the frame renderer's pool sweep
 * (render_entities) for each of the seven live map events.  It carries the
 * event's world position into the view, orients the model from the event's
 * rotation vector composed onto the view matrix, and enqueues the asset that
 * follows the event's model index.
 */

/*
 * The world position is taken from the low halves of the event's reference and
 * y coordinates. The eight-byte render-rotation view overlaps the rotation
 * field and its neighbours, ending immediately before animation_cache.
 */
ADDRESS(0x8001f0c4, 0x154)
void render_map_event(KfMapEvent *event)
{
    SVECTOR screen;
    MATRIX model;
    MATRIX composed;
    long flag;
    u16 asset;
    KfTmdObject *object;

    SetRotMatrix((MATRIX *)&render_state.view_matrix);
    SetTransMatrix((MATRIX *)&render_state.view_matrix);
    screen.vx = (u16)event->reference_x - (u16)render_state.view_position.vx;
    screen.vy = (u16)event->position_y - (u16)render_state.view_position.vy;
    screen.vz = (u16)event->reference_z - (u16)render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&composed.t, &flag);
    RotMatrix((SVECTOR *)&event->rotation_x, &model);
    MulMatrix0((MATRIX *)&render_state.view_matrix, &model, &composed);
    SetRotMatrix(&composed);
    SetTransMatrix(&composed);
    asset = event->model_index + KF_ASSET_MAP_EVENT_FIRST;
    asset_registry_select(asset);
    object = tmd_get_object(0);
    if (render_bind_animated_instance(
            &event->animation_cache, asset, event->animation_clip, event->animation_phase,
            object->vertex_count) == 0) {
        tmd_select_object_vertices(0);
        tmd_project_vertices(tmd_get_object(0)->vertex_count);
    } else {
        tmd_project_vertices(object->vertex_count);
    }
    render_enqueue_tmd(0, 0);
}
