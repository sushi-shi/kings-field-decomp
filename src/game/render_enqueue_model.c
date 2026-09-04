#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/* Global material clut/tpage set by the caller (render_actor). */
extern u16 DAT_80095058;   /* clut */
extern u16 DAT_8009505a;   /* tpage */
extern u8 DAT_80057b5f;    /* last primitive code */
extern CVECTOR DAT_80057b5c;   /* base colour fed to NormalColorDpq */

/*
 * Emit one prepared TMD object as flat/Gouraud shaded textured primitives using
 * the shared global material colour.  Textured triangles (0x24) and quads
 * (0x2c) take a single face normal (NormalColorDpq); Gouraud textured triangles
 * (0x34) and quads (0x3c) take a per-vertex normal (NormalColorDpq3).  Each
 * packet is back-face clipped against the projected screen vertices and sorted
 * into the 3D ordering table at the averaged depth biased by the caller's tag.
 *
 * Residue: the graphics-context aggregate wall documented for render_enqueue_map
 * -- tmd_state.current_asset, DAT_800911b0 and display_state are one object in
 * the original, reached through a single base register; kept as separate
 * curated identities here, so per-access address materialisation diverges.
 */
ADDRESS(0x8001d730, 0x6e8)
void render_enqueue_model(u16 object_index, s16 depth_bias)
{
    KfTmdObject *object;
    u8 *payload;
    u8 *normals;
    u8 *packet;
    KfScreenVertex *vertices;
    s32 remaining;
    u32 header;
    u32 type;
    s32 otz;
    KfScreenVertex *va;
    KfScreenVertex *vb;
    KfScreenVertex *vc;
    KfScreenVertex *vd;

    object = tmd_get_object(object_index);
    payload = tmd_state.current_asset;
    remaining = object->primitive_count;
    packet = payload + object->primitive_offset + 12;
    normals = payload + object->normal_offset + 12;
    vertices = DAT_800911b0;
    if (remaining == 0) {
        return;
    }
    remaining--;
    do {
        header = *(u32 *)packet;
        type = header >> 24;
        if (type == 0x34) {
            KfTmdGt3 *gt = (KfTmdGt3 *)(packet + 4);

            va = (KfScreenVertex *)((u8 *)vertices + gt->v0);
            vb = (KfScreenVertex *)((u8 *)vertices + gt->v1);
            vc = (KfScreenVertex *)((u8 *)vertices + gt->v2);
            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_GT3 *prim = (POLY_GT3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_GT3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyGT3(prim);
                DAT_80057b5f = prim->code;
                prim->clut = DAT_80095058;
                prim->tpage = DAT_8009505a;
                *(long *)&prim->x0 = *(long *)&va->sxy;
                *(long *)&prim->x1 = *(long *)&vb->sxy;
                *(long *)&prim->x2 = *(long *)&vc->sxy;
                *(u16 *)&prim->u0 = *(u16 *)&gt->tu0;
                *(u16 *)&prim->u1 = *(u16 *)&gt->tu1;
                *(u16 *)&prim->u2 = *(u16 *)&gt->tu2;
                NormalColorDpq3((SVECTOR *)(normals + gt->n0), (SVECTOR *)(normals + gt->n1),
                                (SVECTOR *)(normals + gt->n2), &DAT_80057b5c, va->p2,
                                (CVECTOR *)&prim->r0, (CVECTOR *)&prim->r1, (CVECTOR *)&prim->r2);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> 2;
                if (otz + depth_bias > 4) {
                    AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                }
            }
        } else if (type < 0x35) {
            if (type == 0x24) {
                KfTmdFt3 *ft = (KfTmdFt3 *)(packet + 4);

                va = (KfScreenVertex *)((u8 *)vertices + ft->v0);
                vb = (KfScreenVertex *)((u8 *)vertices + ft->v1);
                vc = (KfScreenVertex *)((u8 *)vertices + ft->v2);
                if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                    POLY_FT3 *prim = (POLY_FT3 *)display_state.primitive_buffer->cursor;

                    display_state.primitive_buffer->cursor += sizeof(POLY_FT3);
                    if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                        return;
                    }
                    SetPolyFT3(prim);
                    DAT_80057b5f = prim->code;
                    prim->clut = DAT_80095058;
                    prim->tpage = DAT_8009505a;
                    *(long *)&prim->x0 = *(long *)&va->sxy;
                    *(long *)&prim->x1 = *(long *)&vb->sxy;
                    *(long *)&prim->x2 = *(long *)&vc->sxy;
                    *(u16 *)&prim->u0 = *(u16 *)&ft->tu0;
                    *(u16 *)&prim->u1 = *(u16 *)&ft->tu1;
                    *(u16 *)&prim->u2 = *(u16 *)&ft->tu2;
                    NormalColorDpq((SVECTOR *)(normals + ft->n0), &DAT_80057b5c,
                                   (va->p2 + vb->p2 + vc->p2) / 3, (CVECTOR *)&prim->r0);
                    otz = (va->sz + vb->sz + vc->sz) / 3 >> 2;
                    if (otz + depth_bias > 4) {
                        AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                    }
                }
            } else if (type == 0x2c) {
                KfTmdFt4 *ft = (KfTmdFt4 *)(packet + 4);

                va = (KfScreenVertex *)((u8 *)vertices + ft->v0);
                vb = (KfScreenVertex *)((u8 *)vertices + ft->v1);
                vc = (KfScreenVertex *)((u8 *)vertices + ft->v2);
                if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                    POLY_FT4 *prim = (POLY_FT4 *)display_state.primitive_buffer->cursor;

                    display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
                    vd = (KfScreenVertex *)((u8 *)vertices + ft->v3);
                    if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                        return;
                    }
                    SetPolyFT4(prim);
                    DAT_80057b5f = prim->code;
                    prim->clut = DAT_80095058;
                    prim->tpage = DAT_8009505a;
                    *(long *)&prim->x0 = *(long *)&va->sxy;
                    *(long *)&prim->x1 = *(long *)&vb->sxy;
                    *(long *)&prim->x2 = *(long *)&vc->sxy;
                    *(long *)&prim->x3 = *(long *)&vd->sxy;
                    *(u16 *)&prim->u0 = *(u16 *)&ft->tu0;
                    *(u16 *)&prim->u1 = *(u16 *)&ft->tu1;
                    *(u16 *)&prim->u2 = *(u16 *)&ft->tu2;
                    *(u16 *)&prim->u3 = *(u16 *)&ft->tu3;
                    NormalColorDpq((SVECTOR *)(normals + ft->n0), &DAT_80057b5c,
                                   (va->p2 + vc->p2 + vb->p2 + vd->p2) >> 2, (CVECTOR *)&prim->r0);
                    otz = (va->sz + vb->sz + vc->sz + vd->sz) >> 4;
                    if (otz + depth_bias > 4) {
                        AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                    }
                }
            }
        } else if (type == 0x3c) {
            KfTmdGt4 *gt = (KfTmdGt4 *)(packet + 4);

            va = (KfScreenVertex *)((u8 *)vertices + gt->v0);
            vb = (KfScreenVertex *)((u8 *)vertices + gt->v1);
            vc = (KfScreenVertex *)((u8 *)vertices + gt->v2);
            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_GT4 *prim = (POLY_GT4 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_GT4);
                vd = (KfScreenVertex *)((u8 *)vertices + gt->v3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyGT4(prim);
                DAT_80057b5f = prim->code;
                prim->clut = DAT_80095058;
                prim->tpage = DAT_8009505a;
                *(long *)&prim->x0 = *(long *)&va->sxy;
                *(long *)&prim->x1 = *(long *)&vb->sxy;
                *(long *)&prim->x2 = *(long *)&vc->sxy;
                *(long *)&prim->x3 = *(long *)&vd->sxy;
                *(u16 *)&prim->u0 = *(u16 *)&gt->tu0;
                *(u16 *)&prim->u1 = *(u16 *)&gt->tu1;
                *(u16 *)&prim->u2 = *(u16 *)&gt->tu2;
                *(u16 *)&prim->u3 = *(u16 *)&gt->tu3;
                NormalColorDpq3((SVECTOR *)(normals + gt->n0), (SVECTOR *)(normals + gt->n1),
                                (SVECTOR *)(normals + gt->n2), &DAT_80057b5c, va->p2,
                                (CVECTOR *)&prim->r0, (CVECTOR *)&prim->r1, (CVECTOR *)&prim->r2);
                NormalColorDpq((SVECTOR *)(normals + gt->n3), &DAT_80057b5c, va->p2,
                               (CVECTOR *)&prim->r3);
                otz = (va->sz + vc->sz + vb->sz + vd->sz) >> 4;
                if (otz + depth_bias > 4) {
                    AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                }
            }
        }
        packet += 4 + ((header >> 6) & 0x3fc);
    } while (remaining-- != 0);
}
