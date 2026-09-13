#include <kf/address.h>
#include <kf/open_render.h>
#include <kf/open_scene0.h>

ADDRESS(0x800194d8, 0x48)
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
