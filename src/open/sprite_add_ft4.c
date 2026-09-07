#include <kf/address.h>
#include <kf/open_render.h>

ADDRESS(0x8001399c, 0x1c0)
void sprite_add_ft4(const KfScreenRect *position, u8 *texcoords, u32 tpage, u32 clut,
                    const CVECTOR *color, u16 ot_index)
{
    POLY_FT4 *prim = (POLY_FT4 *)open_graphics_runtime.display_state.primitive_buffer->cursor;

    open_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
    SetPolyFT4(prim);
    prim->clut = clut;
    prim->tpage = tpage;
    prim->x0 = position->x;
    prim->y0 = position->y;
    prim->x1 = position->x + position->w;
    prim->y1 = position->y;
    prim->x2 = position->x;
    prim->y2 = position->y + position->h;
    prim->x3 = position->x + position->w;
    prim->y3 = position->y + position->h;
    prim->u0 = texcoords[0];
    prim->v0 = texcoords[2];
    prim->u1 = texcoords[0] + texcoords[4];
    prim->v1 = texcoords[2];
    prim->u2 = texcoords[0];
    prim->v2 = texcoords[2] + texcoords[6];
    prim->u3 = texcoords[0] + texcoords[4];
    prim->v3 = texcoords[2] + texcoords[6];
    prim->r0 = color->r;
    prim->g0 = color->g;
    prim->b0 = color->b;
    AddPrim(
        &open_graphics_runtime.ordering_table[ot_index & KF_ORDERING_TABLE_INDEX_MASK],
        prim);
}

ADDRESS(0x80013b5c, 0x114)
void sprite_add_f4(
    const KfScreenRect *position, const CVECTOR *color, u16 ot_index)
{
    POLY_F4 *prim = (POLY_F4 *)open_graphics_runtime.display_state.primitive_buffer->cursor;

    open_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_F4);
    SetPolyF4(prim);
    prim->x0 = position->x;
    prim->y0 = position->y;
    prim->x1 = position->x + position->w;
    prim->y1 = position->y;
    prim->x2 = position->x;
    prim->y2 = position->y + position->h;
    prim->x3 = position->x + position->w;
    prim->y3 = position->y + position->h;
    prim->r0 = color->r;
    prim->g0 = color->g;
    prim->b0 = color->b;
    AddPrim(
        &open_graphics_runtime.ordering_table[ot_index & KF_ORDERING_TABLE_INDEX_MASK],
        prim);
}
