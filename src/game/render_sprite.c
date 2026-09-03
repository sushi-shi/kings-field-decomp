#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/semantic_types.h>

/*
 * Screen-space textured-sprite emitters.  Both build a POLY_FT4 quad from a
 * 12-byte sprite descriptor (texture U/V origin and span, then screen origin
 * and span) and link it into the frame's ordering table.  func_8001e480 draws
 * the quad in raw screen space for the HUD-gauge pass; render_enqueue_sprite first
 * projects the quad's four corners through the GTE and depth-cues its colour so
 * the notification and billboard sprites sort into the 3D scene.
 *
 * WIP: the shared 12-byte descriptor is modelled locally as KfSpriteQuad until
 * its producers (the HUD gauge table, the DAT_80055afc sprite records, and the
 * notification descriptors) are reconstructed and the type can move into the
 * shared header.  The tint/clut/tpage carriers (DAT_80095058..DAT_8009505f) are
 * the floor-item render descriptor's leading fields, reached by their
 * individual identities until that object is modelled.
 *
 * Codegen residue (func_8001e480, structurally exact -- the POLY_FT4 field
 * layout, the primitive-buffer guard, and the ordering-table insert all match).
 * gcc-2.5.7's post-reload scheduler fills the load-delay slots after the screen
 * X/Y reads with the clut/tpage global loads, hoisting them ahead of their
 * stores; retail leaves those slots as nops and keeps each clut/tpage load
 * beside its store.  Neither -O2 nor -O2 -fno-schedule-insns2 reproduces both
 * retail's prologue hoist and the un-hoisted body, so this is the open
 * compiler-attribution scheduling residue, not a source defect.
 */

extern KfDisplayState display_state;

extern u16 DAT_80095058;
extern u16 DAT_8009505a;
extern u8 DAT_8009505c;
extern u8 DAT_8009505d;
extern u8 DAT_8009505e;

/* One screen sprite descriptor: texture rect then screen rect. */
typedef struct KfSpriteQuad {
    u8 u;      /* +0: texture U origin */
    u8 v;      /* +1: texture V origin */
    u8 u_span; /* +2: texture U extent */
    u8 v_span; /* +3: texture V extent */
    u16 x;     /* +4: screen X origin */
    u16 y;     /* +6: screen Y origin */
    u16 w;     /* +8: screen width */
    u16 h;     /* +10: screen height */
} KfSpriteQuad;

ADDRESS(0x8001e480, 0x16c)
void func_8001e480(KfSpriteQuad *sprite)
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
