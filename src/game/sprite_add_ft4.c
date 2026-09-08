#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_render.h>

ADDRESS(0x80014314, 0x1c0)
void sprite_add_ft4(const KfScreenRect *position, u8 *texcoords, u32 tpage, u32 clut,
                   const CVECTOR *color, u16 ot_index)
{
    POLY_FT4 *prim = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

    game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
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
    prim->u0 = texcoords[KF_QUAD_TEX_U];
    prim->v0 = texcoords[KF_QUAD_TEX_V];
    prim->u1 = texcoords[KF_QUAD_TEX_U] + texcoords[KF_QUAD_TEX_U_SPAN];
    prim->v1 = texcoords[KF_QUAD_TEX_V];
    prim->u2 = texcoords[KF_QUAD_TEX_U];
    prim->v2 = texcoords[KF_QUAD_TEX_V] + texcoords[KF_QUAD_TEX_V_SPAN];
    prim->u3 = texcoords[KF_QUAD_TEX_U] + texcoords[KF_QUAD_TEX_U_SPAN];
    prim->v3 = texcoords[KF_QUAD_TEX_V] + texcoords[KF_QUAD_TEX_V_SPAN];
    prim->r0 = color->r;
    prim->g0 = color->g;
    prim->b0 = color->b;
    AddPrim(
        &game_graphics_runtime.display_state.ordering_table[ot_index & KF_ORDERING_TABLE_INDEX_MASK],
        prim);
}
