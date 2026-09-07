#include <kf/address.h>
#include <kf/open_opening_render.h>
#include <kf/open_render.h>

enum {
    ENDING_GRADIENT_OT_DEPTH = 0x3fef
};

ADDRESS(0x800137c4, 0x40)
void opening_render_frame(const VECTOR *position, const SVECTOR *rotation)
{
    render_set_view_transform(position, rotation);
    display_begin_frame();
    SetGeomScreen(KF_DEFAULT_PROJECTION_DISTANCE);
    opening_render_entities();
    display_present_frame();
}

ADDRESS(0x80013804, 0x198)
void sprite_add_g4(const KfScreenRect *position, const CVECTOR *color0, const CVECTOR *color1,
                   const CVECTOR *color2, const CVECTOR *color3)
{
    POLY_G4 *prim = (POLY_G4 *)open_graphics_runtime.display_state.primitive_buffer->cursor;

    open_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_G4);
    SetPolyG4(prim);
    prim->x0 = position->x;
    prim->y0 = position->y;
    prim->x1 = position->x + position->w;
    prim->y1 = position->y;
    prim->x2 = position->x;
    prim->y2 = position->y + position->h;
    prim->x3 = position->x + position->w;
    prim->y3 = position->y + position->h;
    prim->r0 = color0->r;
    prim->g0 = color0->g;
    prim->b0 = color0->b;
    prim->r1 = color1->r;
    prim->g1 = color1->g;
    prim->b1 = color1->b;
    prim->r2 = color2->r;
    prim->g2 = color2->g;
    prim->b2 = color2->b;
    prim->r3 = color3->r;
    prim->g3 = color3->g;
    prim->b3 = color3->b;
    AddPrim(&open_graphics_runtime.ordering_table[ENDING_GRADIENT_OT_DEPTH], prim);
}
