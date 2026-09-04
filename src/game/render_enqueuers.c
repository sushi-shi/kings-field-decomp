#include <kf/address.h>
#include <kf/psyq.h>
#include <kf/game_render.h>
#include <kf/tmd.h>

DATA(0x80057b58, 0x4)
CVECTOR tmd_textured_primitive_color = {0x80, 0x80, 0x80, 0};

#define VTX(off) ((KfScreenVertex *)((u8 *)vertices + (off)))

RODATA(0x8001222c, 0x74)

ADDRESS(0x8001c7f8, 0xf38)
void render_enqueue_tmd(u16 object_index, s16 depth_bias)
{
    KfTmdObject *object;
    u8 *payload;
    u8 *normals;
    u8 *packet;
    KfScreenVertex *vertices;
    u32 remaining;
    u32 header;
    s32 otz;

    object = tmd_get_object(object_index);
    payload = tmd_state.current_asset;
    remaining = object->primitive_count;
    packet = payload + object->primitive_offset + 12;
    normals = payload + object->normal_offset + 12;
    vertices = tmd_projected_vertices;
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

            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                POLY_F3 *prim = (POLY_F3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_F3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyF3(prim);
                if ((header >> 24) == 0x22) {
                    SetSemiTrans(prim, 1);
                }
                *(long *)&prim->x0 = va->sxy;
                *(long *)&prim->x1 = vb->sxy;
                *(long *)&prim->x2 = vc->sxy;
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

            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                POLY_FT3 *prim = (POLY_FT3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_FT3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyFT3(prim);
                tmd_textured_primitive_color.cd = prim->code;
                prim->clut = p->cba;
                prim->tpage = p->tsb;
                *(long *)&prim->x0 = va->sxy;
                *(long *)&prim->x1 = vb->sxy;
                *(long *)&prim->x2 = vc->sxy;
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

            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
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
                *(long *)&prim->x0 = va->sxy;
                *(long *)&prim->x1 = vb->sxy;
                *(long *)&prim->x2 = vc->sxy;
                *(long *)&prim->x3 = vd->sxy;
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

            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
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
                *(long *)&prim->x0 = va->sxy;
                *(long *)&prim->x1 = vb->sxy;
                *(long *)&prim->x2 = vc->sxy;
                *(long *)&prim->x3 = vd->sxy;
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

            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                POLY_G3 *prim = (POLY_G3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_G3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyG3(prim);
                *(long *)&prim->x0 = va->sxy;
                *(long *)&prim->x1 = vb->sxy;
                *(long *)&prim->x2 = vc->sxy;
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

            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                POLY_G3 *prim = (POLY_G3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_G3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyG3(prim);
                SetSemiTrans(prim, 1);
                *(long *)&prim->x0 = va->sxy;
                *(long *)&prim->x1 = vb->sxy;
                *(long *)&prim->x2 = vc->sxy;
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

            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                POLY_GT3 *prim = (POLY_GT3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_GT3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyGT3(prim);
                tmd_textured_primitive_color.cd = prim->code;
                prim->clut = p->cba;
                prim->tpage = p->tsb;
                *(long *)&prim->x0 = va->sxy;
                *(long *)&prim->x1 = vb->sxy;
                *(long *)&prim->x2 = vc->sxy;
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

            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                POLY_G4 *prim = (POLY_G4 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_G4);
                vd = VTX(p->v3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyG4(prim);
                *(long *)&prim->x0 = va->sxy;
                *(long *)&prim->x1 = vb->sxy;
                *(long *)&prim->x2 = vc->sxy;
                *(long *)&prim->x3 = vd->sxy;
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

            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                POLY_G4 *prim = (POLY_G4 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_G4);
                vd = VTX(p->v3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyG4(prim);
                SetSemiTrans(prim, 1);
                *(long *)&prim->x0 = va->sxy;
                *(long *)&prim->x1 = vb->sxy;
                *(long *)&prim->x2 = vc->sxy;
                *(long *)&prim->x3 = vd->sxy;
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

            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
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
                *(long *)&prim->x0 = va->sxy;
                *(long *)&prim->x1 = vb->sxy;
                *(long *)&prim->x2 = vc->sxy;
                *(long *)&prim->x3 = vd->sxy;
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

#undef VTX

DATA(0x80057b5c, 0x4)
CVECTOR model_textured_primitive_color = {0x80, 0x80, 0x80, 0};

/* Emit textured model packets with the active texture selection and the
 * separate model lighting colour. The caller supplies the signed depth bias.
 */
ADDRESS(0x8001d730, 0x6e8)
void render_enqueue_model(u16 object_index, s16 depth_bias)
{
    KfTmdObject *object;
    u8 *payload;
    u8 *normals;
    u8 *packet;
    KfScreenVertex *vertices;
    u32 remaining;
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
    vertices = tmd_projected_vertices;
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
            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                POLY_GT3 *prim = (POLY_GT3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_GT3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyGT3(prim);
                model_textured_primitive_color.cd = prim->code;
                prim->clut = active_render_clut;
                prim->tpage = active_render_tpage;
                *(long *)&prim->x0 = va->sxy;
                *(long *)&prim->x1 = vb->sxy;
                *(long *)&prim->x2 = vc->sxy;
                *(u16 *)&prim->u0 = *(u16 *)&gt->tu0;
                *(u16 *)&prim->u1 = *(u16 *)&gt->tu1;
                *(u16 *)&prim->u2 = *(u16 *)&gt->tu2;
                NormalColorDpq3((SVECTOR *)(normals + gt->n0), (SVECTOR *)(normals + gt->n1),
                                (SVECTOR *)(normals + gt->n2), &model_textured_primitive_color, va->p2,
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
                if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                    POLY_FT3 *prim = (POLY_FT3 *)display_state.primitive_buffer->cursor;

                    display_state.primitive_buffer->cursor += sizeof(POLY_FT3);
                    if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                        return;
                    }
                    SetPolyFT3(prim);
                    model_textured_primitive_color.cd = prim->code;
                    prim->clut = active_render_clut;
                    prim->tpage = active_render_tpage;
                    *(long *)&prim->x0 = va->sxy;
                    *(long *)&prim->x1 = vb->sxy;
                    *(long *)&prim->x2 = vc->sxy;
                    *(u16 *)&prim->u0 = *(u16 *)&ft->tu0;
                    *(u16 *)&prim->u1 = *(u16 *)&ft->tu1;
                    *(u16 *)&prim->u2 = *(u16 *)&ft->tu2;
                    NormalColorDpq((SVECTOR *)(normals + ft->n0), &model_textured_primitive_color,
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
                if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                    POLY_FT4 *prim = (POLY_FT4 *)display_state.primitive_buffer->cursor;

                    display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
                    vd = (KfScreenVertex *)((u8 *)vertices + ft->v3);
                    if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                        return;
                    }
                    SetPolyFT4(prim);
                    model_textured_primitive_color.cd = prim->code;
                    prim->clut = active_render_clut;
                    prim->tpage = active_render_tpage;
                    *(long *)&prim->x0 = va->sxy;
                    *(long *)&prim->x1 = vb->sxy;
                    *(long *)&prim->x2 = vc->sxy;
                    *(long *)&prim->x3 = vd->sxy;
                    *(u16 *)&prim->u0 = *(u16 *)&ft->tu0;
                    *(u16 *)&prim->u1 = *(u16 *)&ft->tu1;
                    *(u16 *)&prim->u2 = *(u16 *)&ft->tu2;
                    *(u16 *)&prim->u3 = *(u16 *)&ft->tu3;
                    NormalColorDpq((SVECTOR *)(normals + ft->n0), &model_textured_primitive_color,
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
            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                POLY_GT4 *prim = (POLY_GT4 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_GT4);
                vd = (KfScreenVertex *)((u8 *)vertices + gt->v3);
                if (display_state.primitive_buffer->cursor > display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyGT4(prim);
                model_textured_primitive_color.cd = prim->code;
                prim->clut = active_render_clut;
                prim->tpage = active_render_tpage;
                *(long *)&prim->x0 = va->sxy;
                *(long *)&prim->x1 = vb->sxy;
                *(long *)&prim->x2 = vc->sxy;
                *(long *)&prim->x3 = vd->sxy;
                *(u16 *)&prim->u0 = *(u16 *)&gt->tu0;
                *(u16 *)&prim->u1 = *(u16 *)&gt->tu1;
                *(u16 *)&prim->u2 = *(u16 *)&gt->tu2;
                *(u16 *)&prim->u3 = *(u16 *)&gt->tu3;
                NormalColorDpq3((SVECTOR *)(normals + gt->n0), (SVECTOR *)(normals + gt->n1),
                                (SVECTOR *)(normals + gt->n2), &model_textured_primitive_color, va->p2,
                                (CVECTOR *)&prim->r0, (CVECTOR *)&prim->r1, (CVECTOR *)&prim->r2);
                NormalColorDpq((SVECTOR *)(normals + gt->n3), &model_textured_primitive_color, va->p2,
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

DATA(0x80057b60, 0x4)
CVECTOR map_textured_primitive_color = {0x80, 0x80, 0x80, 0};

DATA(0x80057b64, 0x8)
SVECTOR render_sprite_light_normal = {0, 0, 0x1000, 0};

/*
 * Flat-shading colour scratch: map_textured_primitive_color.cd carries the
 * current primitive code into NormalColorCol, and render_sprite_light_normal
 * supplies the following light direction to NormalColorDpq.
 */

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
    u32 remaining;
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
    vertices = tmd_projected_vertices;
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
            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                POLY_GT3 *gt3 = (POLY_GT3 *)display_state.primitive_buffer->cursor;

                display_state.primitive_buffer->cursor += sizeof(POLY_GT3);
                if (display_state.primitive_buffer->cursor <=
                    display_state.primitive_buffer->end) {
                    SetPolyGT3(gt3);
                    gt3->clut = ft3->cba;
                    gt3->tpage = ft3->tsb;
                    *(long *)&gt3->x0 = va->sxy;
                    *(long *)&gt3->x1 = vb->sxy;
                    *(long *)&gt3->x2 = vc->sxy;
                    *(u16 *)&gt3->u0 = *(u16 *)&ft3->tu0;
                    *(u16 *)&gt3->u1 = *(u16 *)&ft3->tu1;
                    *(u16 *)&gt3->u2 = *(u16 *)&ft3->tu2;
                    map_textured_primitive_color.cd = gt3->code;
                    NormalColorCol((SVECTOR *)(normals + ft3->n0),
                                   &map_textured_primitive_color, &shade);
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
            if (NormalClip(va->sxy, vb->sxy, vc->sxy) > 0) {
                prim = (POLY_GT4 *)display_state.primitive_buffer->cursor;
                display_state.primitive_buffer->cursor += sizeof(POLY_GT4);
                if (display_state.primitive_buffer->cursor <=
                    display_state.primitive_buffer->end) {
                    SetPolyGT4(prim);
                    vd = (KfScreenVertex *)((u8 *)vertices + ft4->v3);
                    prim->clut = ft4->cba;
                    prim->tpage = ft4->tsb;
                    *(long *)&prim->x0 = va->sxy;
                    *(long *)&prim->x1 = vb->sxy;
                    *(long *)&prim->x2 = vc->sxy;
                    *(long *)&prim->x3 = vd->sxy;
                    *(u16 *)&prim->u0 = *(u16 *)&ft4->tu0;
                    *(u16 *)&prim->u1 = *(u16 *)&ft4->tu1;
                    *(u16 *)&prim->u2 = *(u16 *)&ft4->tu2;
                    *(u16 *)&prim->u3 = *(u16 *)&ft4->tu3;
                    map_textured_primitive_color.cd = prim->code;
                    NormalColorCol((SVECTOR *)(normals + ft4->n0),
                                   &map_textured_primitive_color, &shade);
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
    prim->clut = active_render_clut;
    *(long *)&prim->x0 = sxy0;
    *(long *)&prim->x1 = sxy1;
    *(long *)&prim->x2 = sxy2;
    prim->tpage = active_render_tpage;
    *(long *)&prim->x3 = sxy3;
    prim->u0 = prim->u2 = sprite->u;
    prim->u1 = prim->u3 = sprite->u + sprite->u_span;
    prim->v0 = prim->v1 = sprite->v;
    prim->v2 = prim->v3 = sprite->v + sprite->v_span;
    active_render_code = prim->code;
    if (flag == 1) {
        p += p >> 1;
    }
    /* WIP graphics ownership: retail derives the CVECTOR from CLUT + 4 bytes. */
    NormalColorDpq(&render_sprite_light_normal, (CVECTOR *)(&active_render_clut + 2), p,
                   (CVECTOR *)&prim->r0);
    otz += screen_scale;
    if (otz >= 5) {
        AddPrim(&display_state.ordering_table[otz & 0x3fff], prim);
    }
}
