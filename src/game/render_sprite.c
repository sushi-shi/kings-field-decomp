#include <kf/game_graphics.h>
#include <kf/address.h>
#include <psyq/sdk.h>
#include <kf/game_render.h>

/* Emit a HUD quad in screen space using the current material. */

enum { HUD_SPRITE_OT_DEPTH = 1 };

ADDRESS(0x8001e480, 0x16c)
void render_screen_sprite(KfSpriteQuad *sprite)
{
    POLY_FT4 *prim;

    prim = (POLY_FT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
    if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
        return;
    }
    SetPolyFT4(prim);
    prim->x0 = prim->x2 = sprite->x;
    prim->x1 = prim->x3 = sprite->x + sprite->w;
    prim->y0 = prim->y1 = sprite->y;
    prim->y2 = prim->y3 = sprite->y + sprite->h;
    prim->clut = game_graphics_runtime.active_render_clut;
    prim->tpage = game_graphics_runtime.active_render_tpage;
    prim->u0 = prim->u2 = sprite->u;
    prim->u1 = prim->u3 = sprite->u + sprite->u_span;
    prim->v0 = prim->v1 = sprite->v;
    prim->v2 = prim->v3 = sprite->v + sprite->v_span;
    setRGB0(prim,
        game_graphics_runtime.active_render_color.r,
        game_graphics_runtime.active_render_color.g,
        game_graphics_runtime.active_render_color.b);
    AddPrim(&game_graphics_runtime.display_state.ordering_table[HUD_SPRITE_OT_DEPTH], prim);
}
