#include <kf/address.h>
#include <kf/open_opening_render.h>
#include <kf/open_render.h>

ADDRESS(0x800137c4, 0x40)
void opening_render_frame(const VECTOR *position, const SVECTOR *rotation)
{
    render_set_view_transform(position, rotation);
    display_begin_frame();
    SetGeomScreen(200);
    opening_render_entities();
    display_present_frame();
}

ADDRESS(0x80013804, 0x198)
void sprite_add_g4(const u16 *position, const u8 *color0, const u8 *color1,
                   const u8 *color2, const u8 *color3)
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
    prim->r0 = color0[0];
    prim->g0 = color0[1];
    prim->b0 = color0[2];
    prim->r1 = color1[0];
    prim->g1 = color1[1];
    prim->b1 = color1[2];
    prim->r2 = color2[0];
    prim->g2 = color2[1];
    prim->b2 = color2[2];
    prim->r3 = color3[0];
    prim->g3 = color3[1];
    prim->b3 = color3[2];
    AddPrim(&open_graphics_runtime.ordering_table[0x3fef], prim);
}
