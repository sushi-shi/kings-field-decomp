#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfDisplayState display_state;

ADDRESS(0x80014314, 0x1c0)
void sprite_add_ft4(u16 *position, u8 *texcoords, u16 tpage, u16 clut,
                   u8 *color, u16 ot_index)
{
    POLY_FT4 *prim = (POLY_FT4 *)display_state.primitive_buffer->cursor;

    display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
    SetPolyFT4(prim);
    prim->clut = clut;
    prim->tpage = tpage;
    prim->x0 = position[0];
    prim->y0 = position[1];
    prim->x1 = position[0] + position[2];
    prim->y1 = position[1];
    prim->x2 = position[0];
    prim->y2 = position[1] + position[3];
    prim->x3 = position[0] + position[2];
    prim->y3 = position[1] + position[3];
    prim->u0 = texcoords[0];
    prim->v0 = texcoords[2];
    prim->u1 = texcoords[0] + texcoords[4];
    prim->v1 = texcoords[2];
    prim->u2 = texcoords[0];
    prim->v2 = texcoords[2] + texcoords[6];
    prim->u3 = texcoords[0] + texcoords[4];
    prim->v3 = texcoords[2] + texcoords[6];
    prim->r0 = color[0];
    prim->g0 = color[1];
    prim->b0 = color[2];
    AddPrim(&display_state.ordering_table[ot_index & 0x3fff], prim);
}
