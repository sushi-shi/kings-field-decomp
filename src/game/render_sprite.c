#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/game_render.h>

/* Emit a HUD quad in screen space using the current material. */

ADDRESS(0x8001e480, 0x16c)
void render_screen_sprite(KfSpriteQuad *sprite)
{
    POLY_FT4 *prim;

    prim = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
    if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
        return;
    }
    SetPolyFT4(prim);
    prim->x0 = prim->x2 = sprite->x;
    prim->x1 = prim->x3 = sprite->x + sprite->w;
    prim->y0 = prim->y1 = sprite->y;
    prim->y2 = prim->y3 = sprite->y + sprite->h;
    prim->clut = active_render_clut;
    prim->tpage = active_render_tpage;
    prim->u0 = prim->u2 = sprite->u;
    prim->u1 = prim->u3 = sprite->u + sprite->u_span;
    prim->v0 = prim->v1 = sprite->v;
    prim->v2 = prim->v3 = sprite->v + sprite->v_span;
    prim->r0 = active_render_red;
    prim->g0 = active_render_green;
    prim->b0 = active_render_blue;
    AddPrim(&display_state.ordering_table[1], prim);
}
