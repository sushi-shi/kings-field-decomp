#include <kf/game_graphics.h>

#include <kf/game_render.h>

void sprite_add_ft4(const KfScreenRect *rectangle, u8 *texcoords, u32 tpage, u32 clut,
                   const CVECTOR *color, u16 ot_index)
{
    POLY_FT4 *prim = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
    SetPolyFT4(prim);
    prim->clut = clut;
    prim->tpage = tpage;
    setXYWH(prim, rectangle->x, rectangle->y, rectangle->w, rectangle->h);
    setUVWH(prim, texcoords[KF_QUAD_TEX_U], texcoords[KF_QUAD_TEX_V],
            texcoords[KF_QUAD_TEX_U_SPAN], texcoords[KF_QUAD_TEX_V_SPAN]);
    setRGB0(prim, color->r, color->g, color->b);
    AddPrim(
        (void *)(&game_graphics_runtime.display_state.ordering_table[ot_index & KF_ORDERING_TABLE_INDEX_MASK]),
        (void *)prim);
}
