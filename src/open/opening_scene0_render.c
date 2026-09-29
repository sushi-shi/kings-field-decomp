#include <kf/open/render.h>
#include <kf/open/scene0.h>

void opening_scene0_render_frame(
    const VECTOR *position_or_null, const SVECTOR *rotation_or_null)
{
    render_set_view_transform(position_or_null, rotation_or_null);
    display_begin_frame();
    SetGeomScreen(KF_DEFAULT_PROJECTION_DISTANCE);
    opening_render_map_cells();
    opening_render_entities_and_items();
    display_present_frame();
}
