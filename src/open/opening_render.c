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
void sprite_add_g4(const u16 *position, const CVECTOR *color0, const CVECTOR *color1,
                   const CVECTOR *color2, const CVECTOR *color3)
{
    POLY_G4 *prim = (POLY_G4 *)open_graphics_runtime.display_state.primitive_buffer->cursor;

    open_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_G4);
    SetPolyG4(prim);
    prim->x0 = position[0];
    prim->y0 = position[1];
    prim->x1 = position[0] + position[2];
    prim->y1 = position[1];
    prim->x2 = position[0];
    prim->y2 = position[1] + position[3];
    prim->x3 = position[0] + position[2];
    prim->y3 = position[1] + position[3];
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
