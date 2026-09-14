#include <kf/lib/address.h>
#include <kf/open/opening_render.h>
#include <kf/open/render.h>

enum {
    ENDING_GRADIENT_OT_DEPTH = 0x3fef
};

ADDRESS(0x800137c4, 0x40)
void opening_render_frame(const VECTOR *position_or_null, const SVECTOR *rotation_or_null)
{
    render_set_view_transform(position_or_null, rotation_or_null);
    display_begin_frame();
    SetGeomScreen(KF_DEFAULT_PROJECTION_DISTANCE);
    opening_render_entities();
    display_present_frame();
}

ADDRESS(0x80013804, 0x198)
void sprite_add_g4(const KfScreenRect *rectangle, const CVECTOR *color0, const CVECTOR *color1,
                   const CVECTOR *color2, const CVECTOR *color3)
{
    POLY_G4 *prim = (POLY_G4 *)open_graphics_runtime.display_state.primitive_buffer->cursor;

    open_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_G4);
    SetPolyG4(prim);
    setXYWH(prim, rectangle->x, rectangle->y, rectangle->w, rectangle->h);
    setRGB0(prim, color0->r, color0->g, color0->b);
    setRGB1(prim, color1->r, color1->g, color1->b);
    setRGB2(prim, color2->r, color2->g, color2->b);
    setRGB3(prim, color3->r, color3->g, color3->b);
    AddPrim((void *)(&open_graphics_runtime.ordering_table[ENDING_GRADIENT_OT_DEPTH]), (void *)prim);
}
