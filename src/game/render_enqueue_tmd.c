#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

DATA(0x80057b58, 0x4)
CVECTOR tmd_textured_primitive_color = {0x80, 0x80, 0x80, 0};

#define VTX(off) ((KfScreenVertex *)((u8 *)vertices + (off)))

/*
 * Emit one prepared TMD object across the full primitive-type set: flat (F3/F4),
 * flat textured (FT3/FT4), Gouraud (G3/G4) and Gouraud textured (GT3/GT4),
 * each with a semi-transparent variant.  Untextured packets shade from their own
 * packet colour; textured packets take tmd_textured_primitive_color and the
 * packet's own CLUT/tpage.  Every packet is back-face clipped against the
 * projected screen vertices and sorted at the averaged depth biased by the tag.
 *
 * Residue: the graphics-context aggregate wall documented for render_enqueue_map
 * -- tmd_state.current_asset, DAT_800911b0 and display_state are one object in
 * the original reached through a single base register; kept as separate curated
 * identities here, so per-access address materialisation diverges.
 */
RODATA(0x8001222c, 0x74)

ADDRESS(0x8001c7f8, 0xf38)
void render_enqueue_tmd(u16 object_index, s16 depth_bias)
{
    KfTmdObject *object;
    u8 *payload;
    u8 *normals;
    u8 *packet;
    KfScreenVertex *vertices;
    s32 remaining;
    u32 header;
    s32 otz;

    object = tmd_get_object(object_index);
    payload = tmd_state.current_asset;
    remaining = object->primitive_count;
    packet = payload + object->primitive_offset + 12;
    normals = payload + object->normal_offset + 12;
    vertices = DAT_800911b0;
    if (remaining == 0) {
        return;
    }
    do {
        remaining--;
        header = *(u32 *)packet;
        switch (header >> 24) {
        case 0x20:
        case 0x22: {
            KfTmdF3 *p = (KfTmdF3 *)(packet + 4);
            KfScreenVertex *va = VTX(p->v0);
            KfScreenVertex *vb = VTX(p->v1);
            KfScreenVertex *vc = VTX(p->v2);

            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_F3 *prim = (POLY_F3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_F3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyF3(prim);
                if ((header >> 24) == 0x22) {
                    SetSemiTrans(prim, 1);
                }
                *(long *)&prim->x0 = *(long *)&va->sxy;
                *(long *)&prim->x1 = *(long *)&vb->sxy;
                *(long *)&prim->x2 = *(long *)&vc->sxy;
                NormalColorDpq((SVECTOR *)(normals + p->n0), (CVECTOR *)&p->r,
                               (va->p2 + vb->p2 + vc->p2) / 3, (CVECTOR *)&prim->r0);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> 2;
                if (otz + depth_bias > 4) {
                    AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                }
            }
            break;
        }
        case 0x24: {
            KfTmdFt3 *p = (KfTmdFt3 *)(packet + 4);
            KfScreenVertex *va = VTX(p->v0);
            KfScreenVertex *vb = VTX(p->v1);
            KfScreenVertex *vc = VTX(p->v2);

            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_FT3 *prim = (POLY_FT3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_FT3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyFT3(prim);
                tmd_textured_primitive_color.cd = prim->code;
                prim->clut = p->cba;
                prim->tpage = p->tsb;
                *(long *)&prim->x0 = *(long *)&va->sxy;
                *(long *)&prim->x1 = *(long *)&vb->sxy;
                *(long *)&prim->x2 = *(long *)&vc->sxy;
                *(u16 *)&prim->u0 = *(u16 *)&p->tu0;
                *(u16 *)&prim->u1 = *(u16 *)&p->tu1;
                *(u16 *)&prim->u2 = *(u16 *)&p->tu2;
                NormalColorDpq((SVECTOR *)(normals + p->n0), &tmd_textured_primitive_color,
                               (va->p2 + vb->p2 + vc->p2) / 3, (CVECTOR *)&prim->r0);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> 2;
                if (otz + depth_bias > 4) {
                    AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                }
            }
            break;
        }
        case 0x28:
        case 0x2a: {
            KfTmdF4 *p = (KfTmdF4 *)(packet + 4);
            KfScreenVertex *va = VTX(p->v0);
            KfScreenVertex *vb = VTX(p->v1);
            KfScreenVertex *vc = VTX(p->v2);
            KfScreenVertex *vd;

            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_F4 *prim = (POLY_F4 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_F4);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyF4(prim);
                if ((header >> 24) == 0x2a) {
                    SetSemiTrans(prim, 1);
                }
                vd = VTX(p->v3);
                *(long *)&prim->x0 = *(long *)&va->sxy;
                *(long *)&prim->x1 = *(long *)&vb->sxy;
                *(long *)&prim->x2 = *(long *)&vc->sxy;
                *(long *)&prim->x3 = *(long *)&vd->sxy;
                NormalColorDpq((SVECTOR *)(normals + p->n0), (CVECTOR *)&p->r,
                               (va->p2 + vb->p2 + vc->p2 + vd->p2) >> 2, (CVECTOR *)&prim->r0);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> 4;
                if (otz + depth_bias > 4) {
                    AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                }
            }
            break;
        }
        case 0x2c: {
            KfTmdFt4 *p = (KfTmdFt4 *)(packet + 4);
            KfScreenVertex *va = VTX(p->v0);
            KfScreenVertex *vb = VTX(p->v1);
            KfScreenVertex *vc = VTX(p->v2);
            KfScreenVertex *vd;

            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_FT4 *prim = (POLY_FT4 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
                vd = VTX(p->v3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyFT4(prim);
                tmd_textured_primitive_color.cd = prim->code;
                prim->clut = p->cba;
                prim->tpage = p->tsb;
                *(long *)&prim->x0 = *(long *)&va->sxy;
                *(long *)&prim->x1 = *(long *)&vb->sxy;
                *(long *)&prim->x2 = *(long *)&vc->sxy;
                *(long *)&prim->x3 = *(long *)&vd->sxy;
                *(u16 *)&prim->u0 = *(u16 *)&p->tu0;
                *(u16 *)&prim->u1 = *(u16 *)&p->tu1;
                *(u16 *)&prim->u2 = *(u16 *)&p->tu2;
                *(u16 *)&prim->u3 = *(u16 *)&p->tu3;
                NormalColorDpq((SVECTOR *)(normals + p->n0), &tmd_textured_primitive_color,
                               (va->p2 + vc->p2 + vb->p2 + vd->p2) >> 2, (CVECTOR *)&prim->r0);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> 4;
                if (otz + depth_bias > 4) {
                    AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                }
            }
            break;
        }
        case 0x30: {
            KfTmdG3 *p = (KfTmdG3 *)(packet + 4);
            KfScreenVertex *va = VTX(p->v0);
            KfScreenVertex *vb = VTX(p->v1);
            KfScreenVertex *vc = VTX(p->v2);

            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_G3 *prim = (POLY_G3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_G3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyG3(prim);
                *(long *)&prim->x0 = *(long *)&va->sxy;
                *(long *)&prim->x1 = *(long *)&vb->sxy;
                *(long *)&prim->x2 = *(long *)&vc->sxy;
                NormalColorDpq3((SVECTOR *)(normals + p->n0), (SVECTOR *)(normals + p->n1),
                                (SVECTOR *)(normals + p->n2), (CVECTOR *)&p->r, va->p2,
                                (CVECTOR *)&prim->r0, (CVECTOR *)&prim->r1, (CVECTOR *)&prim->r2);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> 2;
                if (otz + depth_bias > 4) {
                    AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                }
            }
            break;
        }
        case 0x32: {
            KfTmdG3 *p = (KfTmdG3 *)(packet + 4);
            KfScreenVertex *va = VTX(p->v0);
            KfScreenVertex *vb = VTX(p->v1);
            KfScreenVertex *vc = VTX(p->v2);

            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_G3 *prim = (POLY_G3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_G3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyG3(prim);
                SetSemiTrans(prim, 1);
                *(long *)&prim->x0 = *(long *)&va->sxy;
                *(long *)&prim->x1 = *(long *)&vb->sxy;
                *(long *)&prim->x2 = *(long *)&vc->sxy;
                NormalColorCol3((SVECTOR *)(normals + p->n0), (SVECTOR *)(normals + p->n1),
                                (SVECTOR *)(normals + p->n2), (CVECTOR *)&p->r,
                                (CVECTOR *)&prim->r0, (CVECTOR *)&prim->r1, (CVECTOR *)&prim->r2);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> 2;
                if (otz + depth_bias > 4) {
                    AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                }
            }
            break;
        }
        case 0x34: {
            KfTmdGt3 *p = (KfTmdGt3 *)(packet + 4);
            KfScreenVertex *va = VTX(p->v0);
            KfScreenVertex *vb = VTX(p->v1);
            KfScreenVertex *vc = VTX(p->v2);

            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_GT3 *prim = (POLY_GT3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_GT3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyGT3(prim);
                tmd_textured_primitive_color.cd = prim->code;
                prim->clut = p->cba;
                prim->tpage = p->tsb;
                *(long *)&prim->x0 = *(long *)&va->sxy;
                *(long *)&prim->x1 = *(long *)&vb->sxy;
                *(long *)&prim->x2 = *(long *)&vc->sxy;
                *(u16 *)&prim->u0 = *(u16 *)&p->tu0;
                *(u16 *)&prim->u1 = *(u16 *)&p->tu1;
                *(u16 *)&prim->u2 = *(u16 *)&p->tu2;
                NormalColorDpq3((SVECTOR *)(normals + p->n0), (SVECTOR *)(normals + p->n1),
                                (SVECTOR *)(normals + p->n2), &tmd_textured_primitive_color, va->p2,
                                (CVECTOR *)&prim->r0, (CVECTOR *)&prim->r1, (CVECTOR *)&prim->r2);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> 2;
                if (otz + depth_bias > 4) {
                    AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                }
            }
            break;
        }
        case 0x38: {
            KfTmdG4 *p = (KfTmdG4 *)(packet + 4);
            KfScreenVertex *va = VTX(p->v0);
            KfScreenVertex *vb = VTX(p->v1);
            KfScreenVertex *vc = VTX(p->v2);
            KfScreenVertex *vd;

            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_G4 *prim = (POLY_G4 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_G4);
                vd = VTX(p->v3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyG4(prim);
                *(long *)&prim->x0 = *(long *)&va->sxy;
                *(long *)&prim->x1 = *(long *)&vb->sxy;
                *(long *)&prim->x2 = *(long *)&vc->sxy;
                *(long *)&prim->x3 = *(long *)&vd->sxy;
                NormalColorDpq3((SVECTOR *)(normals + p->n0), (SVECTOR *)(normals + p->n1),
                                (SVECTOR *)(normals + p->n2), (CVECTOR *)&p->r, va->p2,
                                (CVECTOR *)&prim->r0, (CVECTOR *)&prim->r1, (CVECTOR *)&prim->r2);
                NormalColorDpq((SVECTOR *)(normals + p->n3), (CVECTOR *)&p->r, va->p2,
                               (CVECTOR *)&prim->r3);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> 4;
                if (otz + depth_bias > 4) {
                    AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                }
            }
            break;
        }
        case 0x3a: {
            KfTmdG4 *p = (KfTmdG4 *)(packet + 4);
            KfScreenVertex *va = VTX(p->v0);
            KfScreenVertex *vb = VTX(p->v1);
            KfScreenVertex *vc = VTX(p->v2);
            KfScreenVertex *vd;

            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_G4 *prim = (POLY_G4 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_G4);
                vd = VTX(p->v3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyG4(prim);
                SetSemiTrans(prim, 1);
                *(long *)&prim->x0 = *(long *)&va->sxy;
                *(long *)&prim->x1 = *(long *)&vb->sxy;
                *(long *)&prim->x2 = *(long *)&vc->sxy;
                *(long *)&prim->x3 = *(long *)&vd->sxy;
                NormalColorDpq((SVECTOR *)(normals + p->n0), (CVECTOR *)&p->r, va->p2, (CVECTOR *)&prim->r0);
                NormalColorDpq((SVECTOR *)(normals + p->n1), (CVECTOR *)&p->r, va->p2, (CVECTOR *)&prim->r1);
                NormalColorDpq((SVECTOR *)(normals + p->n2), (CVECTOR *)&p->r, va->p2, (CVECTOR *)&prim->r2);
                NormalColorDpq((SVECTOR *)(normals + p->n3), (CVECTOR *)&p->r, va->p2, (CVECTOR *)&prim->r3);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> 4;
                if (otz + depth_bias > 4) {
                    AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                }
            }
            break;
        }
        case 0x3c: {
            KfTmdGt4 *p = (KfTmdGt4 *)(packet + 4);
            KfScreenVertex *va = VTX(p->v0);
            KfScreenVertex *vb = VTX(p->v1);
            KfScreenVertex *vc = VTX(p->v2);
            KfScreenVertex *vd;

            if (NormalClip(*(long *)&va->sxy, *(long *)&vb->sxy, *(long *)&vc->sxy) > 0) {
                POLY_GT4 *prim = (POLY_GT4 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_GT4);
                vd = VTX(p->v3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyGT4(prim);
                tmd_textured_primitive_color.cd = prim->code;
                prim->clut = p->cba;
                prim->tpage = p->tsb;
                *(long *)&prim->x0 = *(long *)&va->sxy;
                *(long *)&prim->x1 = *(long *)&vb->sxy;
                *(long *)&prim->x2 = *(long *)&vc->sxy;
                *(long *)&prim->x3 = *(long *)&vd->sxy;
                *(u16 *)&prim->u0 = *(u16 *)&p->tu0;
                *(u16 *)&prim->u1 = *(u16 *)&p->tu1;
                *(u16 *)&prim->u2 = *(u16 *)&p->tu2;
                *(u16 *)&prim->u3 = *(u16 *)&p->tu3;
                NormalColorDpq3((SVECTOR *)(normals + p->n0), (SVECTOR *)(normals + p->n1),
                                (SVECTOR *)(normals + p->n2), &tmd_textured_primitive_color, va->p2,
                                (CVECTOR *)&prim->r0, (CVECTOR *)&prim->r1, (CVECTOR *)&prim->r2);
                NormalColorDpq((SVECTOR *)(normals + p->n3), &tmd_textured_primitive_color, va->p2,
                               (CVECTOR *)&prim->r3);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> 4;
                if (otz + depth_bias > 4) {
                    AddPrim(&display_state.ordering_table[(otz + depth_bias) & 0x3fff], prim);
                }
            }
            break;
        }
        }
        packet += 4 + ((header >> 6) & 0x3fc);
    } while (remaining != 0);
}
