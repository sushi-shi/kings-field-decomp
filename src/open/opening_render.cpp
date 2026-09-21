#include <kf/open/opening_render.h>
#include <kf/open/render.h>
#include <kf/lib/render_face.h>

enum {
    ENDING_GRADIENT_OT_DEPTH = 0x3fef
};

void opening_render_frame(const VECTOR *position_or_null, const SVECTOR *rotation_or_null)
{
    render_set_view_transform(open_graphics_runtime.render_state, position_or_null, rotation_or_null);
    display_begin_frame(open_graphics_runtime.display_state);
    opening_render_entities();
    display_present_frame(open_graphics_runtime.display_state);
}

void sprite_add_g4(const KfScreenRect *rectangle, const CVECTOR *color0, const CVECTOR *color1,
                   const CVECTOR *color2, const CVECTOR *color3)
{
    kf::DrawFace face {};
    const CVECTOR colors[] = {*color0, *color1, *color2, *color3};
    render_face_rectangle(&face, rectangle->x, rectangle->y,
        rectangle->x + rectangle->w, rectangle->y + rectangle->h);
    render_face_submit(&face, colors, kf::FaceShading::Gouraud, ENDING_GRADIENT_OT_DEPTH);
}
