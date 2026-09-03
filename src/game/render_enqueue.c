#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Two polygon-emission enqueuers that sit at the tail of the render subsystem's
 * primitive pipeline.
 *
 * render_enqueue_map is the lit map-geometry path: it walks a prepared TMD object's
 * FT3/FT4 primitive packets, back-face-clips each against the projected screen
 * vertices, and up-converts them into Gouraud-textured GT3/GT4 GPU primitives
 * whose per-vertex colours come from one flat face normal (NormalColorCol) that
 * is then depth-cued per vertex (DpqColor).  It is the emitter reached by the
 * per-cell wall/floor routine render_map_cell.
 *
 * render_enqueue_sprite is the projected textured-sprite enqueuer: it projects a
 * KfSpriteQuad's four corners through the GTE (RotTransPers4), builds a
 * depth-cued POLY_FT4, and sorts it into the 3D scene's ordering table.  It is
 * reached by the floor-item and actor-billboard emitters and by the frame
 * renderer's notification-sprite pass.  The sort depth is the projected anchor
 * point (RotTransPers of the origin), not the corner batch.
 *
 * Residues (both structurally exact -- call set, referents, widths, control
 * flow all match; see docs/patterns/source-shapes-gcc257.md):
 *
 *  - render_enqueue_sprite hits the same post-reload-scheduler wall as its banked
 *    sibling render_screen_sprite (render_sprite.c): gcc-2.5.7 fills the screen-XY
 *    load-delay slots with the clut/tpage global loads and saves the anchor
 *    depth in the RotTransPers4 delay slot a beat differently than retail,
 *    which spends one extra callee-saved register.  Neither -O2 nor
 *    -fno-schedule-insns2 reproduces both retail schedules at once.
 *
 *  - render_enqueue_map references four curated identities -- tmd_state.current_asset,
 *    the projected-vertex buffer DAT_800911b0, and display_state.ordering_table
 *    -- that the retail code reaches through ONE base register: it holds
 *    &tmd_state.current_asset and forms the vertex buffer as base+488 and the
 *    ordering-table pointer as (buffer-756), with a single relocation for the
 *    whole span.  gcc can only share a base across accesses that are one source
 *    object, so the original was a single graphics-context aggregate that the
 *    curation split into display_state, tmd_state and DAT_800911b0 (the same
 *    unified object also carries the sprite material DAT_80095058 at
 *    buffer+16040, reached that way by render_enqueue_model/render_enqueue_tmd).  Modelling
 *    that aggregate is high-ripple (display_state and tmd_state are shared by
 *    many banked units), so this source keeps the individual identities; the
 *    resulting per-access address materialisation is the dominant divergence
 *    and cannot close until the aggregate is modelled.
 */

/*
 * Flat-shading colour scratch: a CVECTOR at 0x80057b60 whose command byte
 * (0x80057b63) carries the current primitive code into NormalColorCol, and the
 * light direction normal at 0x80057b64 fed to NormalColorDpq.
 */

/*
 * Screen sprite / floor-item render descriptor leading fields, reached by their
 * individual identities until the whole object is modelled (see render_sprite.c).
 */

extern KfTmdObject *tmd_get_object(u16 index);
extern long NormalClip(long sxy0, long sxy1, long sxy2);
extern void NormalColorCol(SVECTOR *normal, CVECTOR *in, CVECTOR *out);
extern void DpqColor(CVECTOR *in, long dqp, CVECTOR *out);

/*
 * Emits one prepared TMD object as lit Gouraud-textured primitives.  Each FT3
 * (mode 0x24) or FT4 (mode 0x2c) packet indexes the projection scratch buffer
 * for its screen vertices, is back-face clipped, and drives a GT3/GT4 packet
 * whose colours are the face-normal shade depth-cued at each vertex.  The
 * ordering-table slot is the averaged screen depth biased by 200.
 */
ADDRESS(0x8001de18, 0x418)
void render_enqueue_map(u16 object_index)
{
    KfTmdObject *object;
    u8 *payload;
    u8 *normals;
    u8 *packet;
    KfScreenVertex *vertices;
    s32 remaining;
    u32 header;
    CVECTOR shade;
    POLY_GT4 *prim;
    KfScreenVertex *va;
    KfScreenVertex *vb;
    KfScreenVertex *vc;
    KfScreenVertex *vd;
    s32 otz;

    object = tmd_get_object(object_index);
    payload = tmd_state.current_asset;
    normals = payload + object->normal_offset + 12;
    tmd_project_vertices(object->vertex_count);
    packet = payload + object->primitive_offset + 12;
    remaining = object->primitive_count;
    vertices = DAT_800911b0;
    if (remaining == 0) {
        return;
    }
    remaining--;
    do {
        header = *(u32 *)packet;
        packet += 4;
        if ((header >> 24) == 0x24) {
            KfTmdFt3 *ft3 = (KfTmdFt3 *)packet;

            va = (KfScreenVertex *)((u8 *)vertices + ft3->v0);
            vb = (KfScreenVertex *)((u8 *)vertices + ft3->v1);
            vc = (KfScreenVertex *)((u8 *)vertices + ft3->v2);
            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_GT3 *gt3 = (POLY_GT3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_GT3);
                if (display_state.primitive_buffer->cursor <=
                    display_state.primitive_buffer->end) {
                    SetPolyGT3(gt3);
                    gt3->clut = ft3->cba;
                    gt3->tpage = ft3->tsb;
                    *(long *)&gt3->x0 = *(long *)&va->sxy;
                    *(long *)&gt3->x1 = *(long *)&vb->sxy;
                    *(long *)&gt3->x2 = *(long *)&vc->sxy;
                    *(u16 *)&gt3->u0 = *(u16 *)&ft3->tu0;
                    *(u16 *)&gt3->u1 = *(u16 *)&ft3->tu1;
                    *(u16 *)&gt3->u2 = *(u16 *)&ft3->tu2;
                    DAT_80057b63 = gt3->code;
                    NormalColorCol((SVECTOR *)(normals + ft3->n0),
                                   (CVECTOR *)(&DAT_80057b63 - 3), &shade);
                    DpqColor(&shade, va->p2, (CVECTOR *)&gt3->r0);
                    DpqColor(&shade, vb->p2, (CVECTOR *)&gt3->r1);
                    DpqColor(&shade, vc->p2, (CVECTOR *)&gt3->r2);
                    otz = ((va->sz + vb->sz + vc->sz) / 3 >> 2) + 200;
                    if (otz < 16384) {
                        AddPrim(&display_state.ordering_table[otz & 0x3fff], gt3);
                    }
                }
            }
        } else if ((header >> 24) == 0x2c) {
            KfTmdFt4 *ft4 = (KfTmdFt4 *)packet;

            va = (KfScreenVertex *)((u8 *)vertices + ft4->v0);
            vb = (KfScreenVertex *)((u8 *)vertices + ft4->v1);
            vc = (KfScreenVertex *)((u8 *)vertices + ft4->v2);
            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                prim = (POLY_GT4 *)display_state.primitive_buffer->cursor;
                display_state.primitive_buffer->cursor += sizeof(POLY_GT4);
                if (display_state.primitive_buffer->cursor <=
                    display_state.primitive_buffer->end) {
                    SetPolyGT4(prim);
                    vd = (KfScreenVertex *)((u8 *)vertices + ft4->v3);
                    prim->clut = ft4->cba;
                    prim->tpage = ft4->tsb;
                    *(long *)&prim->x0 = *(long *)&va->sxy;
                    *(long *)&prim->x1 = *(long *)&vb->sxy;
                    *(long *)&prim->x2 = *(long *)&vc->sxy;
                    *(long *)&prim->x3 = *(long *)&vd->sxy;
                    *(u16 *)&prim->u0 = *(u16 *)&ft4->tu0;
                    *(u16 *)&prim->u1 = *(u16 *)&ft4->tu1;
                    *(u16 *)&prim->u2 = *(u16 *)&ft4->tu2;
                    *(u16 *)&prim->u3 = *(u16 *)&ft4->tu3;
                    DAT_80057b63 = prim->code;
                    NormalColorCol((SVECTOR *)(normals + ft4->n0),
                                   (CVECTOR *)(&DAT_80057b63 - 3), &shade);
                    DpqColor(&shade, va->p2, (CVECTOR *)&prim->r0);
                    DpqColor(&shade, vb->p2, (CVECTOR *)&prim->r1);
                    DpqColor(&shade, vc->p2, (CVECTOR *)&prim->r2);
                    DpqColor(&shade, vd->p2, (CVECTOR *)&prim->r3);
                    otz = ((va->sz + vb->sz + vc->sz + vd->sz) >> 4) + 200;
                    if (otz < 16384) {
                        AddPrim(&display_state.ordering_table[otz & 0x3fff], prim);
                    }
                }
            }
        }
        packet += (header >> 6) & 0x3fc;
    } while (remaining-- != 0);
}

/*
 * Enqueues one screen sprite as a depth-sorted textured quad.  The quad's four
 * corners are projected through the GTE, the primitive's colour is the light
 * normal shade depth-cued by the projected perspective term (optionally scaled
 * by 1.5 when flag selects it), and it sorts into the ordering table at the
 * projected depth biased by the caller's screen_scale.
 */
ADDRESS(0x8001e230, 0x250)
void render_enqueue_sprite(KfSpriteQuad *sprite, s16 screen_scale, s32 flag)
{
    SVECTOR corners[4];
    SVECTOR anchor;
    DVECTOR anchor_sxy;
    long p;
    long clip_flag;
    long sxy0;
    long sxy1;
    long sxy2;
    long sxy3;
    POLY_FT4 *prim;
    s32 otz;

    corners[0].vx = corners[2].vx = sprite->x;
    corners[1].vx = corners[3].vx = sprite->x + sprite->w;
    corners[0].vy = corners[1].vy = sprite->y;
    corners[2].vy = corners[3].vy = sprite->y + sprite->h;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;
    anchor.vx = 0;
    anchor.vy = 0;
    anchor.vz = 0;
    otz = RotTransPers(&anchor, (long *)&anchor_sxy, &p, &clip_flag);
    RotTransPers4(&corners[0], &corners[1], &corners[2], &corners[3],
                  &sxy0, &sxy1, &sxy2, &sxy3, &p, &clip_flag);

    prim = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
    if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
        return;
    }
    SetPolyFT4(prim);
    prim->clut = DAT_80095058;
    *(long *)&prim->x0 = sxy0;
    *(long *)&prim->x1 = sxy1;
    *(long *)&prim->x2 = sxy2;
    prim->tpage = DAT_8009505a;
    *(long *)&prim->x3 = sxy3;
    prim->u0 = prim->u2 = sprite->u;
    prim->u1 = prim->u3 = sprite->u + sprite->u_span;
    prim->v0 = prim->v1 = sprite->v;
    prim->v2 = prim->v3 = sprite->v + sprite->v_span;
    DAT_8009505f = prim->code;
    if (flag == 1) {
        p += p >> 1;
    }
    NormalColorDpq(&DAT_80057b64, (CVECTOR *)(&DAT_80095058 + 2), p,
                   (CVECTOR *)&prim->r0);
    otz += screen_scale;
    if (otz >= 5) {
        AddPrim(&display_state.ordering_table[otz & 0x3fff], prim);
    }
}
