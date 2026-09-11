#include <kf/null.h>
#include <kf/game_graphics.h>
#include <kf/address.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <kf/game_render.h>
#include <kf/tmd.h>
#include <kf/gpu_packets.h>

DATA(0x80057b58, 0x4)
CVECTOR tmd_textured_primitive_color = {
    KF_TEXTURE_BASE_BRIGHTNESS, KF_TEXTURE_BASE_BRIGHTNESS,
    KF_TEXTURE_BASE_BRIGHTNESS, 0
};

/* Prepared vertex indices are byte offsets into the projected array. */
#define VTX(off) ((KfScreenVertex *)((u8 *)game_graphics_runtime.tmd_projected_vertices + (off)))

RODATA(0x8001222c, 0x74)

ADDRESS(0x8001c7f8, 0xf38)
void render_enqueue_tmd(u16 object_index, s16 depth_bias)
{
    KfTmdObject *object;
    u8 *normals;
    u8 *packet;
    u32 header;
    u32 remaining;
    s32 otz;
    KfScreenVertex *va;
    KfScreenVertex *vb;
    KfScreenVertex *vc;
    KfScreenVertex *vd;

    object = tmd_get_object(object_index);
    remaining = object->primitive_count;
    packet = (u8 *)game_graphics_runtime.tmd_state.current_asset + (object->primitive_offset + KF_TMD_HEADER_BYTES);
    normals = (u8 *)game_graphics_runtime.tmd_state.current_asset + (object->normal_offset + KF_TMD_HEADER_BYTES);
    while (remaining-- != 0) {
        header = *(u32 *)packet;
        packet += KF_TMD_PACKET_HEADER_BYTES;
        switch (tmd_packet_mode(header)) {
        case KF_TMD_MODE_FT3: {
            KfTmdPrimitive *p = (KfTmdPrimitive *)packet;
            va = VTX(p->ft3.v0);
            vb = VTX(p->ft3.v1);
            vc = VTX(p->ft3.v2);

            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuFT3 *prim = (KfGpuFT3 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT3);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyFT3(&prim->sdk);
                prim->sdk.clut = p->ft3.cba;
                prim->sdk.tpage = p->ft3.tsb;
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                prim->packed.uv0 = p->texture.uv0;
                prim->packed.uv1 = p->texture.uv1;
                prim->packed.uv2 = p->texture.uv2;
                tmd_textured_primitive_color.cd = prim->sdk.code;
                NormalColorDpq((SVECTOR *)(normals + p->ft3.n0), &tmd_textured_primitive_color,
                               (va->p2 + vb->p2 + vc->p2) / 3, &prim->packed.color0);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case KF_TMD_MODE_F4: {
            KfTmdPrimitive *p = (KfTmdPrimitive *)packet;
            va = VTX(p->f4.v0);
            vb = VTX(p->f4.v1);
            vc = VTX(p->f4.v2);

            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuF4 *prim = (KfGpuF4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                vd = VTX(p->f4.v3);
                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_F4);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyF4(&prim->sdk);
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                prim->packed.xy3 = vd->sxy.word;
                NormalColorDpq((SVECTOR *)(normals + p->f4.n0), &p->color,
                               (va->p2 + vb->p2 + vc->p2 + vd->p2) >> 2, &prim->packed.color0);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case KF_TMD_MODE_G3: {
            KfTmdPrimitive *p = (KfTmdPrimitive *)packet;
            va = VTX(p->g3.v0);
            vb = VTX(p->g3.v1);
            vc = VTX(p->g3.v2);

            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuG3 *prim = (KfGpuG3 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_G3);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyG3(&prim->sdk);
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                NormalColorDpq3((SVECTOR *)(normals + p->g3.n0), (SVECTOR *)(normals + p->g3.n1),
                                (SVECTOR *)(normals + p->g3.n2), &p->color, va->p2,
                                &prim->packed.color0, &prim->packed.color1, &prim->packed.color2);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case KF_TMD_MODE_G4: {
            KfTmdPrimitive *p = (KfTmdPrimitive *)packet;
            va = VTX(p->g4.v0);
            vb = VTX(p->g4.v1);
            vc = VTX(p->g4.v2);

            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuG4 *prim = (KfGpuG4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                vd = VTX(p->g4.v3);
                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_G4);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyG4(&prim->sdk);
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                prim->packed.xy3 = vd->sxy.word;
                NormalColorDpq3((SVECTOR *)(normals + p->g4.n0), (SVECTOR *)(normals + p->g4.n1),
                                (SVECTOR *)(normals + p->g4.n2), &p->color, va->p2,
                                &prim->packed.color0, &prim->packed.color1, &prim->packed.color2);
                NormalColorDpq((SVECTOR *)(normals + p->g4.n3), &p->color, va->p2,
                               &prim->packed.color3);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case KF_TMD_MODE_GT3: {
            KfTmdPrimitive *p = (KfTmdPrimitive *)packet;
            va = VTX(p->gt3.v0);
            vb = VTX(p->gt3.v1);
            vc = VTX(p->gt3.v2);

            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuGT3 *prim = (KfGpuGT3 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_GT3);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyGT3(&prim->sdk);
                prim->sdk.clut = p->gt3.cba;
                prim->sdk.tpage = p->gt3.tsb;
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                prim->packed.uv0 = p->texture.uv0;
                prim->packed.uv1 = p->texture.uv1;
                prim->packed.uv2 = p->texture.uv2;
                tmd_textured_primitive_color.cd = prim->sdk.code;
                NormalColorDpq3((SVECTOR *)(normals + p->gt3.n0), (SVECTOR *)(normals + p->gt3.n1),
                                (SVECTOR *)(normals + p->gt3.n2), &tmd_textured_primitive_color, va->p2,
                                &prim->packed.color0, &prim->packed.color1, &prim->packed.color2);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case KF_TMD_MODE_GT4: {
            KfTmdPrimitive *p = (KfTmdPrimitive *)packet;
            va = VTX(p->gt4.v0);
            vb = VTX(p->gt4.v1);
            vc = VTX(p->gt4.v2);

            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuGT4 *prim = (KfGpuGT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                vd = VTX(p->gt4.v3);
                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_GT4);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyGT4(&prim->sdk);
                prim->sdk.clut = p->gt4.cba;
                prim->sdk.tpage = p->gt4.tsb;
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                prim->packed.xy3 = vd->sxy.word;
                prim->packed.uv0 = p->texture.uv0;
                prim->packed.uv1 = p->texture.uv1;
                prim->packed.uv2 = p->texture.uv2;
                prim->packed.uv3 = p->texture.uv3;
                tmd_textured_primitive_color.cd = prim->sdk.code;
                NormalColorDpq3((SVECTOR *)(normals + p->gt4.n0), (SVECTOR *)(normals + p->gt4.n1),
                                (SVECTOR *)(normals + p->gt4.n2), &tmd_textured_primitive_color, va->p2,
                                &prim->packed.color0, &prim->packed.color1, &prim->packed.color2);
                NormalColorDpq((SVECTOR *)(normals + p->gt4.n3), &tmd_textured_primitive_color, va->p2,
                               &prim->packed.color3);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case (KF_TMD_MODE_G3 | KF_TMD_MODE_SEMITRANS): {
            KfTmdPrimitive *p = (KfTmdPrimitive *)packet;
            va = VTX(p->g3.v0);
            vb = VTX(p->g3.v1);
            vc = VTX(p->g3.v2);

            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuG3 *prim = (KfGpuG3 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_G3);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyG3(&prim->sdk);
                SetSemiTrans(&prim->sdk, 1);
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                NormalColorCol3((SVECTOR *)(normals + p->g3.n0), (SVECTOR *)(normals + p->g3.n1),
                                (SVECTOR *)(normals + p->g3.n2), &p->color,
                                &prim->packed.color0, &prim->packed.color1, &prim->packed.color2);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case KF_TMD_MODE_FT4: {
            KfTmdPrimitive *p = (KfTmdPrimitive *)packet;
            va = VTX(p->ft4.v0);
            vb = VTX(p->ft4.v1);
            vc = VTX(p->ft4.v2);

            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuFT4 *prim = (KfGpuFT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                vd = VTX(p->ft4.v3);
                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyFT4(&prim->sdk);
                prim->sdk.clut = p->ft4.cba;
                prim->sdk.tpage = p->ft4.tsb;
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                prim->packed.xy3 = vd->sxy.word;
                prim->packed.uv0 = p->texture.uv0;
                prim->packed.uv1 = p->texture.uv1;
                prim->packed.uv2 = p->texture.uv2;
                prim->packed.uv3 = p->texture.uv3;
                tmd_textured_primitive_color.cd = prim->sdk.code;
                NormalColorDpq((SVECTOR *)(normals + p->ft4.n0), &tmd_textured_primitive_color,
                               (va->p2 + vb->p2 + vc->p2 + vd->p2) >> 2, &prim->packed.color0);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case KF_TMD_MODE_F3: {
            KfTmdPrimitive *p = (KfTmdPrimitive *)packet;
            va = VTX(p->f3.v0);
            vb = VTX(p->f3.v1);
            vc = VTX(p->f3.v2);

            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuF3 *prim = (KfGpuF3 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_F3);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyF3(&prim->sdk);
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                NormalColorDpq((SVECTOR *)(normals + p->f3.n0), &p->color,
                               (va->p2 + vb->p2 + vc->p2) / 3, &prim->packed.color0);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case (KF_TMD_MODE_G4 | KF_TMD_MODE_SEMITRANS): {
            KfTmdPrimitive *p = (KfTmdPrimitive *)packet;
            va = VTX(p->g4.v0);
            vb = VTX(p->g4.v1);
            vc = VTX(p->g4.v2);

            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuG4 *prim = (KfGpuG4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                vd = VTX(p->g4.v3);
                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_G4);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyG4(&prim->sdk);
                SetSemiTrans(&prim->sdk, 1);
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                prim->packed.xy3 = vd->sxy.word;
                NormalColorDpq((SVECTOR *)(normals + p->g4.n0), &p->color, va->p2, &prim->packed.color0);
                NormalColorDpq((SVECTOR *)(normals + p->g4.n1), &p->color, va->p2, &prim->packed.color1);
                NormalColorDpq((SVECTOR *)(normals + p->g4.n2), &p->color, va->p2, &prim->packed.color2);
                NormalColorDpq((SVECTOR *)(normals + p->g4.n3), &p->color, va->p2, &prim->packed.color3);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case (KF_TMD_MODE_F3 | KF_TMD_MODE_SEMITRANS): {
            KfTmdPrimitive *p = (KfTmdPrimitive *)packet;
            va = VTX(p->f3.v0);
            vb = VTX(p->f3.v1);
            vc = VTX(p->f3.v2);

            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuF3 *prim = (KfGpuF3 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_F3);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyF3(&prim->sdk);
                SetSemiTrans(&prim->sdk, 1);
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                NormalColorDpq((SVECTOR *)(normals + p->f3.n0), &p->color,
                               (va->p2 + vb->p2 + vc->p2) / 3, &prim->packed.color0);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case (KF_TMD_MODE_F4 | KF_TMD_MODE_SEMITRANS): {
            KfTmdPrimitive *p = (KfTmdPrimitive *)packet;
            va = VTX(p->f4.v0);
            vb = VTX(p->f4.v1);
            vc = VTX(p->f4.v2);

            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuF4 *prim = (KfGpuF4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                vd = VTX(p->f4.v3);
                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_F4);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyF4(&prim->sdk);
                SetSemiTrans(&prim->sdk, 1);
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                prim->packed.xy3 = vd->sxy.word;
                NormalColorDpq((SVECTOR *)(normals + p->f4.n0), &p->color,
                               (va->p2 + vb->p2 + vc->p2 + vd->p2) >> 2, &prim->packed.color0);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        }
        packet += (header >> KF_TMD_ILEN_TO_BYTES_SHIFT) & KF_TMD_BODY_BYTES_MASK;
    }
}

#undef VTX

DATA(0x80057b5c, 0x4)
CVECTOR model_textured_primitive_color = {
    KF_TEXTURE_BASE_BRIGHTNESS, KF_TEXTURE_BASE_BRIGHTNESS,
    KF_TEXTURE_BASE_BRIGHTNESS, 0
};

/* Emit textured model packets with the active texture selection and the
 * separate model lighting colour. The caller supplies the signed depth bias.
 */
ADDRESS(0x8001d730, 0x6e8)
void render_enqueue_model(u16 object_index, s16 depth_bias)
{
    KfTmdPrimitive *primitive;
    KfTmdObject *object;
    u8 *normals;
    u8 *packet;
    u32 header;
    u32 remaining;
    KF_ENUM_PARAM(KfTmdMode, s32) type;
    s32 otz;
    KfScreenVertex *va;
    KfScreenVertex *vb;
    KfScreenVertex *vc;
    KfScreenVertex *vd;

    object = tmd_get_object(object_index);
    remaining = object->primitive_count;
    packet = (u8 *)game_graphics_runtime.tmd_state.current_asset + (object->primitive_offset + KF_TMD_HEADER_BYTES);
    normals = (u8 *)game_graphics_runtime.tmd_state.current_asset + (object->normal_offset + KF_TMD_HEADER_BYTES);
    while (remaining-- != 0) {
        u8 *vertices = (u8 *)game_graphics_runtime.tmd_projected_vertices;

        header = *(u32 *)packet;
        packet += KF_TMD_PACKET_HEADER_BYTES;
        primitive = (KfTmdPrimitive *)packet;
        type = tmd_packet_mode(header);
        switch (type) {
        case KF_TMD_MODE_GT3: {
            va = (KfScreenVertex *)(vertices + primitive->gt3.v0);
            vb = (KfScreenVertex *)(vertices + primitive->gt3.v1);
            vc = (KfScreenVertex *)(vertices + primitive->gt3.v2);
            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuGT3 *prim = (KfGpuGT3 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_GT3);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyGT3(&prim->sdk);
                prim->sdk.clut = game_graphics_runtime.active_render_clut;
                prim->sdk.tpage = game_graphics_runtime.active_render_tpage;
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                prim->packed.uv0 = primitive->texture.uv0;
                prim->packed.uv1 = primitive->texture.uv1;
                prim->packed.uv2 = primitive->texture.uv2;
                model_textured_primitive_color.cd = prim->sdk.code;
                NormalColorDpq3((SVECTOR *)(normals + primitive->gt3.n0), (SVECTOR *)(normals + primitive->gt3.n1),
                                (SVECTOR *)(normals + primitive->gt3.n2), &model_textured_primitive_color, va->p2,
                                &prim->packed.color0, &prim->packed.color1, &prim->packed.color2);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case KF_TMD_MODE_GT4: {
            va = (KfScreenVertex *)(vertices + primitive->gt4.v0);
            vb = (KfScreenVertex *)(vertices + primitive->gt4.v1);
            vc = (KfScreenVertex *)(vertices + primitive->gt4.v2);
            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuGT4 *prim = (KfGpuGT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                vd = (KfScreenVertex *)(vertices + primitive->gt4.v3);
                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_GT4);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyGT4(&prim->sdk);
                prim->sdk.clut = game_graphics_runtime.active_render_clut;
                prim->sdk.tpage = game_graphics_runtime.active_render_tpage;
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                prim->packed.xy3 = vd->sxy.word;
                prim->packed.uv0 = primitive->texture.uv0;
                prim->packed.uv1 = primitive->texture.uv1;
                prim->packed.uv2 = primitive->texture.uv2;
                prim->packed.uv3 = primitive->texture.uv3;
                model_textured_primitive_color.cd = prim->sdk.code;
                NormalColorDpq3((SVECTOR *)(normals + primitive->gt4.n0), (SVECTOR *)(normals + primitive->gt4.n1),
                                (SVECTOR *)(normals + primitive->gt4.n2), &model_textured_primitive_color, va->p2,
                                &prim->packed.color0, &prim->packed.color1, &prim->packed.color2);
                NormalColorDpq((SVECTOR *)(normals + primitive->gt4.n3), &model_textured_primitive_color, va->p2,
                               &prim->packed.color3);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case KF_TMD_MODE_FT3: {
            va = (KfScreenVertex *)(vertices + primitive->ft3.v0);
            vb = (KfScreenVertex *)(vertices + primitive->ft3.v1);
            vc = (KfScreenVertex *)(vertices + primitive->ft3.v2);
            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuFT3 *prim = (KfGpuFT3 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT3);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyFT3(&prim->sdk);
                prim->sdk.clut = game_graphics_runtime.active_render_clut;
                prim->sdk.tpage = game_graphics_runtime.active_render_tpage;
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                prim->packed.uv0 = primitive->texture.uv0;
                prim->packed.uv1 = primitive->texture.uv1;
                prim->packed.uv2 = primitive->texture.uv2;
                model_textured_primitive_color.cd = prim->sdk.code;
                NormalColorDpq((SVECTOR *)(normals + primitive->ft3.n0), &model_textured_primitive_color,
                               (va->p2 + vb->p2 + vc->p2) / 3, &prim->packed.color0);
                otz = (va->sz + vb->sz + vc->sz) / 3 >> KF_GTE_DEPTH_TO_OT_SHIFT;
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        case KF_TMD_MODE_FT4: {
            va = (KfScreenVertex *)(vertices + primitive->ft4.v0);
            vb = (KfScreenVertex *)(vertices + primitive->ft4.v1);
            vc = (KfScreenVertex *)(vertices + primitive->ft4.v2);
            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuFT4 *prim = (KfGpuFT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                vd = (KfScreenVertex *)(vertices + primitive->ft4.v3);
                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
                    return;
                }
                SetPolyFT4(&prim->sdk);
                prim->sdk.clut = game_graphics_runtime.active_render_clut;
                prim->sdk.tpage = game_graphics_runtime.active_render_tpage;
                prim->packed.xy0 = va->sxy.word;
                prim->packed.xy1 = vb->sxy.word;
                prim->packed.xy2 = vc->sxy.word;
                prim->packed.xy3 = vd->sxy.word;
                prim->packed.uv0 = primitive->texture.uv0;
                prim->packed.uv1 = primitive->texture.uv1;
                prim->packed.uv2 = primitive->texture.uv2;
                prim->packed.uv3 = primitive->texture.uv3;
                model_textured_primitive_color.cd = prim->sdk.code;
                NormalColorDpq((SVECTOR *)(normals + primitive->ft4.n0), &model_textured_primitive_color,
                               (va->p2 + vb->p2 + vc->p2 + vd->p2) >> 2, &prim->packed.color0);
                otz = (va->sz + vb->sz + vc->sz + vd->sz) >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2);
                if (otz + depth_bias > (KF_SCENE_MIN_OT_DEPTH - 1)) {
                    AddPrim(
                        &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
                        &prim->sdk);
                }
            }
            break;
        }
        }
        packet += (header >> KF_TMD_ILEN_TO_BYTES_SHIFT) & KF_TMD_BODY_BYTES_MASK;
    }
}

DATA(0x80057b60, 0x4)
CVECTOR map_textured_primitive_color = {
    KF_TEXTURE_BASE_BRIGHTNESS, KF_TEXTURE_BASE_BRIGHTNESS,
    KF_TEXTURE_BASE_BRIGHTNESS, 0
};

DATA(0x80057b64, 0x8)
SVECTOR render_sprite_light_normal = {0, 0, KF_FIXED12_ONE, 0};

/*
 * Flat-shading colour scratch: map_textured_primitive_color.cd carries the
 * current primitive code into NormalColorCol, and render_sprite_light_normal
 * supplies the following light direction to NormalColorDpq.
 */

/*
 * Emits one prepared TMD object as lit Gouraud-textured primitives.  Each FT3
 * or FT4 packet indexes the projection scratch buffer
 * for its screen vertices, is back-face clipped, and drives a GT3/GT4 packet
 * whose colours are the face-normal shade depth-cued at each vertex.  The
 * ordering-table slot is the averaged screen depth biased by 200.
 */
ADDRESS(0x8001de18, 0x418)
void render_enqueue_map(u16 object_index)
{
    KfTmdPrimitive *primitive;
    KfTmdObject *object;
    u32 header;
    u8 *normals;
    u8 *packet;
    u32 remaining;
    CVECTOR shade;
    KfGpuGT4 *prim;
    KfScreenVertex *va;
    KfScreenVertex *vb;
    KfScreenVertex *vc;
    KfScreenVertex *vd;

    object = tmd_get_object(object_index);
    normals = (u8 *)game_graphics_runtime.tmd_state.current_asset + (object->normal_offset + KF_TMD_HEADER_BYTES);
    tmd_project_vertices(object->vertex_count);
    packet = (u8 *)game_graphics_runtime.tmd_state.current_asset + (object->primitive_offset + KF_TMD_HEADER_BYTES);
    remaining = object->primitive_count;
    while (remaining-- != 0) {
        u8 *vertices = (u8 *)game_graphics_runtime.tmd_projected_vertices;

        header = *(u32 *)packet;
        packet += KF_TMD_PACKET_HEADER_BYTES;
        primitive = (KfTmdPrimitive *)packet;
        switch (tmd_packet_mode(header)) {
        case KF_TMD_MODE_FT4: {
            s32 otz;

            va = (KfScreenVertex *)(vertices + primitive->ft4.v0);
            vb = (KfScreenVertex *)(vertices + primitive->ft4.v1);
            vc = (KfScreenVertex *)(vertices + primitive->ft4.v2);
            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                prim = (KfGpuGT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
                vd = (KfScreenVertex *)(vertices + primitive->ft4.v3);
                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_GT4);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor <=
                    game_graphics_runtime.display_state.primitive_buffer->end) {
                    SetPolyGT4(&prim->sdk);
                    prim->sdk.clut = primitive->ft4.cba;
                    prim->sdk.tpage = primitive->ft4.tsb;
                    prim->packed.xy0 = va->sxy.word;
                    prim->packed.xy1 = vb->sxy.word;
                    prim->packed.xy2 = vc->sxy.word;
                    prim->packed.xy3 = vd->sxy.word;
                    prim->packed.uv0 = primitive->texture.uv0;
                    prim->packed.uv1 = primitive->texture.uv1;
                    prim->packed.uv2 = primitive->texture.uv2;
                    prim->packed.uv3 = primitive->texture.uv3;
                    map_textured_primitive_color.cd = prim->sdk.code;
                    NormalColorCol((SVECTOR *)(normals + primitive->ft4.n0),
                                   &map_textured_primitive_color, &shade);
                    DpqColor(&shade, va->p2, &prim->packed.color0);
                    DpqColor(&shade, vb->p2, &prim->packed.color1);
                    DpqColor(&shade, vc->p2, &prim->packed.color2);
                    DpqColor(&shade, vd->p2, &prim->packed.color3);
                    otz = ((va->sz + vb->sz + vc->sz + vd->sz)
                        >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + KF_MAP_OT_DEPTH_BIAS;
                    if (otz < KF_ORDERING_TABLE_LENGTH) {
                        AddPrim(
                            &game_graphics_runtime.display_state.ordering_table[otz & KF_ORDERING_TABLE_INDEX_MASK],
                            &prim->sdk);
                    }
                } else {
                    return;
                }
            }
            break;
        }
        case KF_TMD_MODE_FT3: {
            s32 otz;

            va = (KfScreenVertex *)(vertices + primitive->ft3.v0);
            vb = (KfScreenVertex *)(vertices + primitive->ft3.v1);
            vc = (KfScreenVertex *)(vertices + primitive->ft3.v2);
            if (NormalClip(va->sxy.word, vb->sxy.word, vc->sxy.word) > 0) {
                KfGpuGT3 *gt3 = (KfGpuGT3 *)game_graphics_runtime.display_state.primitive_buffer->cursor;

                game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_GT3);
                if (game_graphics_runtime.display_state.primitive_buffer->cursor <=
                    game_graphics_runtime.display_state.primitive_buffer->end) {
                    SetPolyGT3(&gt3->sdk);
                    gt3->sdk.clut = primitive->ft3.cba;
                    gt3->sdk.tpage = primitive->ft3.tsb;
                    gt3->packed.xy0 = va->sxy.word;
                    gt3->packed.xy1 = vb->sxy.word;
                    gt3->packed.xy2 = vc->sxy.word;
                    gt3->packed.uv0 = primitive->texture.uv0;
                    gt3->packed.uv1 = primitive->texture.uv1;
                    gt3->packed.uv2 = primitive->texture.uv2;
                    map_textured_primitive_color.cd = gt3->sdk.code;
                    NormalColorCol((SVECTOR *)(normals + primitive->ft3.n0),
                                   &map_textured_primitive_color, &shade);
                    DpqColor(&shade, va->p2, &gt3->packed.color0);
                    DpqColor(&shade, vb->p2, &gt3->packed.color1);
                    DpqColor(&shade, vc->p2, &gt3->packed.color2);
                    otz = ((va->sz + vb->sz + vc->sz) / 3
                        >> KF_GTE_DEPTH_TO_OT_SHIFT) + KF_MAP_OT_DEPTH_BIAS;
                    if (otz < KF_ORDERING_TABLE_LENGTH) {
                        AddPrim(
                            &game_graphics_runtime.display_state.ordering_table[otz & KF_ORDERING_TABLE_INDEX_MASK],
                            &gt3->sdk);
                    }
                } else {
                    return;
                }
            }
            break;
        }
        }
        packet += (header >> KF_TMD_ILEN_TO_BYTES_SHIFT) & KF_TMD_BODY_BYTES_MASK;
    }
}

/*
 * Enqueues one screen sprite as a depth-sorted textured quad.  The quad's four
 * corners are projected through the GTE, the primitive's colour is the light
 * normal shade depth-cued by the projected perspective term (optionally scaled
 * by 1.5 when flag selects it), and it sorts into the ordering table at the
 * projected depth biased by the caller's depth_bias.
 */
ADDRESS(0x8001e230, 0x250)
void render_enqueue_sprite(
    KfSpriteQuad *sprite, s16 depth_bias, KfSpriteDepthCueMode depth_cue_mode)
{
    SVECTOR corners[4];
    SVECTOR anchor;
    long anchor_sxy;
    long depth_cue;
    long clip_flag;
    long sxy0;
    long sxy1;
    long sxy2;
    long sxy3;
    KfGpuFT4 *prim;
    s32 otz;

    corners[0].vx = corners[2].vx = sprite->x;
    corners[1].vx = corners[3].vx = sprite->x + sprite->w;
    corners[0].vy = corners[1].vy = sprite->y;
    corners[2].vy = corners[3].vy = sprite->y + sprite->h;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;
    anchor.vx = anchor.vy = anchor.vz = 0;
    otz = RotTransPers(&anchor, &anchor_sxy, &depth_cue, &clip_flag);
    RotTransPers4(&corners[0], &corners[1], &corners[2], &corners[3],
                  &sxy0, &sxy1, &sxy2, &sxy3, &depth_cue, &clip_flag);

    prim = (KfGpuFT4 *)game_graphics_runtime.display_state.primitive_buffer->cursor;
    game_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
    if (game_graphics_runtime.display_state.primitive_buffer->cursor > game_graphics_runtime.display_state.primitive_buffer->end) {
        return;
    }
    SetPolyFT4(&prim->sdk);
    prim->sdk.clut = game_graphics_runtime.active_render_clut;
    prim->sdk.tpage = game_graphics_runtime.active_render_tpage;
    /* Each packed GTE word spans the SDK packet's x/y halfwords. */
    memcpy(&prim->sdk.x0, &sxy0, sizeof sxy0);
    memcpy(&prim->sdk.x1, &sxy1, sizeof sxy1);
    memcpy(&prim->sdk.x2, &sxy2, sizeof sxy2);
    memcpy(&prim->sdk.x3, &sxy3, sizeof sxy3);
    prim->sdk.u0 = prim->sdk.u2 = sprite->u;
    prim->sdk.u1 = prim->sdk.u3 = sprite->u + sprite->u_span;
    prim->sdk.v0 = prim->sdk.v1 = sprite->v;
    prim->sdk.v2 = prim->sdk.v3 = sprite->v + sprite->v_span;
    game_graphics_runtime.active_render_color.cd = prim->sdk.code;
    if (depth_cue_mode == KF_SPRITE_DEPTH_CUE_BOOSTED) {
        depth_cue += depth_cue >> 1;
    }
    NormalColorDpq(&render_sprite_light_normal, &game_graphics_runtime.active_render_color, depth_cue,
                   &prim->packed.color0);
    if (otz + depth_bias >= KF_SCENE_MIN_OT_DEPTH) {
        AddPrim(
            &game_graphics_runtime.display_state.ordering_table[(otz + depth_bias) & KF_ORDERING_TABLE_INDEX_MASK],
            &prim->sdk);
    }
}
