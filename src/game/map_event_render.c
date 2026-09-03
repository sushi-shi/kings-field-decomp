#include <kf/address.h>
#include <kf/semantic_types.h>

/*
 * Map-event model emitter invoked by the frame renderer's pool sweep
 * (render_entities) for each of the seven live map events.  It carries the
 * event's world position into the view, orients the model from the event's
 * rotation vector composed onto the view matrix, and enqueues the asset that
 * follows the event's variant id.
 */

extern KfRenderState render_state;

extern void asset_registry_select(u16 index);
extern KfTmdObject *tmd_get_object(u16 index);
extern void tmd_select_object_vertices(u16 object_index);
extern void tmd_project_vertices(s32 count);
extern u16 *render_bind_animated_instance(void *anchor, u16 asset, u16 tag, u16 variant, u16 count);
extern void render_enqueue_tmd(u16 arg0, s16 arg1);

/*
 * The world position is taken from the low halves of the event's reference and
 * y coordinates; the render-rotation vector overlaps the event's rotation field
 * and its neighbours, and the visibility anchor sits inside the trailing bytes,
 * so both are reached through views of KfMapEvent until that block is modelled.
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
    RotMatrix((SVECTOR *)&event->unknown_34, &model);
    MulMatrix0((MATRIX *)&render_state.view_matrix, &model, &composed);
    SetRotMatrix(&composed);
    SetTransMatrix(&composed);
    asset = event->variant + 10;
    asset_registry_select(asset);
    object = tmd_get_object(0);
    if (render_bind_animated_instance(
            &event->unknown_3a[2], asset, event->unknown_0f, event->rotation_phase,
            object->vertex_count) == 0) {
        tmd_select_object_vertices(0);
        tmd_project_vertices(tmd_get_object(0)->vertex_count);
    } else {
        tmd_project_vertices(object->vertex_count);
    }
    render_enqueue_tmd(0, 0);
}
