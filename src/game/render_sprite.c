#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/game_render.h>
#include <kf/game.h>

/*
 * Screen-space textured-sprite emitters.  Both build a POLY_FT4 quad from a
 * 12-byte sprite descriptor (texture U/V origin and span, then screen origin
 * and span) and link it into the frame's ordering table.  render_screen_sprite draws
 * the quad in raw screen space for the HUD-gauge pass; render_enqueue_sprite first
 * projects the quad's four corners through the GTE and depth-cues its colour so
 * the notification and billboard sprites sort into the 3D scene.
 *
 * The shared 12-byte KfSpriteQuad layout is used by the HUD gauge table, the
 * DAT_80055afc sprite records, and the notification descriptors.  The
 * tint/clut/tpage carriers (DAT_80095058..DAT_8009505f) are
 * the floor-item render descriptor's leading fields, reached by their
 * individual identities until that object is modelled.
 *
 * Codegen residue (render_screen_sprite, structurally exact -- the POLY_FT4 field
 * layout, the primitive-buffer guard, and the ordering-table insert all match).
 * gcc-2.5.7's post-reload scheduler fills the load-delay slots after the screen
 * X/Y reads with the clut/tpage global loads, hoisting them ahead of their
 * stores; retail leaves those slots as nops and keeps each clut/tpage load
 * beside its store.  Neither -O2 nor -O2 -fno-schedule-insns2 reproduces both
 * retail's prologue hoist and the un-hoisted body, so this is the open
 * compiler-attribution scheduling residue, not a source defect.
 */

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
    prim->clut = DAT_80095058;
    prim->tpage = DAT_8009505a;
    prim->u0 = prim->u2 = sprite->u;
    prim->u1 = prim->u3 = sprite->u + sprite->u_span;
    prim->v0 = prim->v1 = sprite->v;
    prim->v2 = prim->v3 = sprite->v + sprite->v_span;
    prim->r0 = DAT_8009505c;
    prim->g0 = DAT_8009505d;
    prim->b0 = DAT_8009505e;
    AddPrim(&display_state.ordering_table[1], prim);
}
