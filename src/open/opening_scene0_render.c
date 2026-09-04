#include <kf/address.h>
#include <kf/open_render.h>
#include <kf/open_scene0.h>

ADDRESS(0x800194d8, 0x48)
void opening_scene0_render_frame(
    const VECTOR *position, const SVECTOR *rotation)
{
    render_set_view_transform(position, rotation);
    display_begin_frame();
    SetGeomScreen(200);
    opening_render_map_cells();
    opening_render_entities_and_items();
    display_present_frame();
}
