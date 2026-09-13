#include <kf/address.h>
#include <kf/open_render.h>

ADDRESS(0x8001399c, 0x1c0)
void sprite_add_ft4(const KfScreenRect *rectangle, u8 *texcoords, u32 tpage, u32 clut,
                    const CVECTOR *color, u16 ot_index)
{
    POLY_FT4 *prim = (POLY_FT4 *)open_graphics_runtime.display_state.primitive_buffer->cursor;

    open_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
    SetPolyFT4(prim);
    prim->clut = clut;
    prim->tpage = tpage;
    setXYWH(prim, rectangle->x, rectangle->y, rectangle->w, rectangle->h);
    setUVWH(prim, texcoords[KF_QUAD_TEX_U], texcoords[KF_QUAD_TEX_V],
            texcoords[KF_QUAD_TEX_U_SPAN], texcoords[KF_QUAD_TEX_V_SPAN]);
    setRGB0(prim, color->r, color->g, color->b);
    AddPrim(
        (void *)(&open_graphics_runtime.ordering_table[ot_index & KF_ORDERING_TABLE_INDEX_MASK]),
        (void *)prim);
}

ADDRESS(0x80013b5c, 0x114)
void sprite_add_f4(
    const KfScreenRect *rectangle, const CVECTOR *color, u16 ot_index)
{
    POLY_F4 *prim = (POLY_F4 *)open_graphics_runtime.display_state.primitive_buffer->cursor;

    open_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_F4);
    SetPolyF4(prim);
    setXYWH(prim, rectangle->x, rectangle->y, rectangle->w, rectangle->h);
    setRGB0(prim, color->r, color->g, color->b);
    AddPrim(
        (void *)(&open_graphics_runtime.ordering_table[ot_index & KF_ORDERING_TABLE_INDEX_MASK]),
        (void *)prim);
}
