#include <kf/lib/null.h>
#include <kf/lib/address.h>
#include <kf/lib/gpu_packets.h>
#include <kf/open/render.h>

DATA(0x800372f0, 0x4)
CVECTOR tmd_textured_primitive_color = {
    KF_TEXTURE_BASE_BRIGHTNESS, KF_TEXTURE_BASE_BRIGHTNESS,
    KF_TEXTURE_BASE_BRIGHTNESS, 0
};

RODATA(0x800121c0, 0x74)

/* Prepared vertex indices are byte offsets into the projected array. */
#define VTX(off) TMD_PREPARED_VERTEX(open_graphics_runtime.tmd_projected_vertices, (off))

ADDRESS(0x8001764c, 0xcf8)
void render_enqueue_tmd(u16 object_index, s16 depth_bias)
{
    KfTmdObject *object = tmd_get_object(object_index);
    /* Retail reserves one unallocated stack word below its spills that no
     * live value explains; this never-read local carries it
     * (docs/patterns/reconstruction-debt-review.md). */
    u16 unattributed_stack_slot[2];
    u32 header;
    u32 remaining = object->primitive_count;
    u8 *packet = (u8 *)open_graphics_runtime.tmd_state.current_asset +
        (object->primitive_offset + KF_TMD_HEADER_BYTES);
    u8 *normals = (u8 *)open_graphics_runtime.tmd_state.current_asset +
        (object->normal_offset + KF_TMD_HEADER_BYTES);
    KfScreenVertex *vertex0;
    KfScreenVertex *vertex1;
    KfScreenVertex *vertex2;
    KfScreenVertex *vertex3;
    s32 depth;

    for (; remaining-- != 0;
         packet += TMD_PACKET_BODY_BYTES(header)) {
        header = *(u32 *)packet;
        packet = TMD_PACKET_BODY(packet);
        switch (tmd_packet_mode(header)) {
        case KF_TMD_MODE_FT3: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuFT3 *prim;

            vertex0 = VTX(polygon->ft3.v0);
            vertex1 = VTX(polygon->ft3.v1);
            vertex2 = VTX(polygon->ft3.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            prim = (KfGpuFT3 *)primitive_buffer_allocate(sizeof(POLY_FT3));
            SetPolyFT3(&prim->sdk);
            prim->packed.clut = polygon->ft3.cba;
            prim->packed.tpage = polygon->ft3.tsb;
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            prim->packed.uv0 = polygon->texture.uv0;
            prim->packed.uv1 = polygon->texture.uv1;
            prim->packed.uv2 = polygon->texture.uv2;
            tmd_textured_primitive_color.cd = prim->sdk.code;
            NormalColorDpq((SVECTOR *)(normals + polygon->ft3.n0), &tmd_textured_primitive_color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2) / 3,
                           &prim->packed.color0);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    (void *)(&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK]),
                    (void *)&prim->sdk);
            }
            break;
        }
        case KF_TMD_MODE_F4: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuF4 *prim;

            vertex0 = VTX(polygon->f4.v0);
            vertex1 = VTX(polygon->f4.v1);
            vertex2 = VTX(polygon->f4.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            vertex3 = VTX(polygon->f4.v3);
            prim = (KfGpuF4 *)primitive_buffer_allocate(sizeof(POLY_F4));
            SetPolyF4(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            prim->packed.xy3 = vertex3->sxy.word;
            NormalColorDpq((SVECTOR *)(normals + polygon->f4.n0), &polygon->color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2,
                           &prim->packed.color0);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    (void *)(&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK]),
                    (void *)&prim->sdk);
            }
            break;
        }
        case KF_TMD_MODE_G3: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuG3 *prim;

            vertex0 = VTX(polygon->g3.v0);
            vertex1 = VTX(polygon->g3.v1);
            vertex2 = VTX(polygon->g3.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            prim = (KfGpuG3 *)primitive_buffer_allocate(sizeof(POLY_G3));
            SetPolyG3(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            NormalColorDpq3((SVECTOR *)(normals + polygon->g3.n0),
                            (SVECTOR *)(normals + polygon->g3.n1),
                            (SVECTOR *)(normals + polygon->g3.n2), &polygon->color,
                            vertex0->p2, &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    (void *)(&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK]),
                    (void *)&prim->sdk);
            }
            break;
        }
        case KF_TMD_MODE_G4: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuG4 *prim;

            vertex0 = VTX(polygon->g4.v0);
            vertex1 = VTX(polygon->g4.v1);
            vertex2 = VTX(polygon->g4.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            vertex3 = VTX(polygon->g4.v3);
            prim = (KfGpuG4 *)primitive_buffer_allocate(sizeof(POLY_G4));
            SetPolyG4(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            prim->packed.xy3 = vertex3->sxy.word;
            NormalColorDpq3((SVECTOR *)(normals + polygon->g4.n0),
                            (SVECTOR *)(normals + polygon->g4.n1),
                            (SVECTOR *)(normals + polygon->g4.n2), &polygon->color,
                            vertex0->p2, &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            NormalColorDpq((SVECTOR *)(normals + polygon->g4.n3), &polygon->color,
                           vertex0->p2, &prim->packed.color3);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    (void *)(&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK]),
                    (void *)&prim->sdk);
            }
            break;
        }
        case KF_TMD_MODE_GT3: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuGT3 *prim;

            vertex0 = VTX(polygon->gt3.v0);
            vertex1 = VTX(polygon->gt3.v1);
            vertex2 = VTX(polygon->gt3.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            prim = (KfGpuGT3 *)primitive_buffer_allocate(sizeof(POLY_GT3));
            SetPolyGT3(&prim->sdk);
            prim->packed.clut = polygon->gt3.cba;
            prim->packed.tpage = polygon->gt3.tsb;
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            prim->packed.uv0 = polygon->texture.uv0;
            prim->packed.uv1 = polygon->texture.uv1;
            prim->packed.uv2 = polygon->texture.uv2;
            tmd_textured_primitive_color.cd = prim->sdk.code;
            NormalColorDpq3((SVECTOR *)(normals + polygon->gt3.n0),
                            (SVECTOR *)(normals + polygon->gt3.n1),
                            (SVECTOR *)(normals + polygon->gt3.n2), &tmd_textured_primitive_color,
                            vertex0->p2, &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    (void *)(&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK]),
                    (void *)&prim->sdk);
            }
            break;
        }
        case KF_TMD_MODE_GT4: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuGT4 *prim;

            vertex0 = VTX(polygon->gt4.v0);
            vertex1 = VTX(polygon->gt4.v1);
            vertex2 = VTX(polygon->gt4.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            vertex3 = VTX(polygon->gt4.v3);
            prim = (KfGpuGT4 *)primitive_buffer_allocate(sizeof(POLY_GT4));
            SetPolyGT4(&prim->sdk);
            prim->packed.clut = polygon->gt4.cba;
            prim->packed.tpage = polygon->gt4.tsb;
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            prim->packed.xy3 = vertex3->sxy.word;
            prim->packed.uv0 = polygon->texture.uv0;
            prim->packed.uv1 = polygon->texture.uv1;
            prim->packed.uv2 = polygon->texture.uv2;
            prim->packed.uv3 = polygon->texture.uv3;
            tmd_textured_primitive_color.cd = prim->sdk.code;
            NormalColorDpq3((SVECTOR *)(normals + polygon->gt4.n0),
                            (SVECTOR *)(normals + polygon->gt4.n1),
                            (SVECTOR *)(normals + polygon->gt4.n2), &tmd_textured_primitive_color,
                            vertex0->p2, &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            NormalColorDpq((SVECTOR *)(normals + polygon->gt4.n3), &tmd_textured_primitive_color,
                           vertex0->p2, &prim->packed.color3);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    (void *)(&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK]),
                    (void *)&prim->sdk);
            }
            break;
        }
        case (KF_TMD_MODE_G3 | KF_TMD_MODE_SEMITRANS): {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuG3 *prim;

            vertex0 = VTX(polygon->g3.v0);
            vertex1 = VTX(polygon->g3.v1);
            vertex2 = VTX(polygon->g3.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            prim = (KfGpuG3 *)primitive_buffer_allocate(sizeof(POLY_G3));
            SetPolyG3(&prim->sdk);
            SetSemiTrans((void *)&prim->sdk, 1);
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            NormalColorCol3((SVECTOR *)(normals + polygon->g3.n0),
                            (SVECTOR *)(normals + polygon->g3.n1),
                            (SVECTOR *)(normals + polygon->g3.n2), &polygon->color,
                            &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    (void *)(&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK]),
                    (void *)&prim->sdk);
            }
            break;
        }
        case KF_TMD_MODE_FT4: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuFT4 *prim;

            vertex0 = VTX(polygon->ft4.v0);
            vertex1 = VTX(polygon->ft4.v1);
            vertex2 = VTX(polygon->ft4.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            vertex3 = VTX(polygon->ft4.v3);
            prim = (KfGpuFT4 *)primitive_buffer_allocate(sizeof(POLY_FT4));
            SetPolyFT4(&prim->sdk);
            prim->packed.clut = polygon->ft4.cba;
            prim->packed.tpage = polygon->ft4.tsb;
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            prim->packed.xy3 = vertex3->sxy.word;
            prim->packed.uv0 = polygon->texture.uv0;
            prim->packed.uv1 = polygon->texture.uv1;
            prim->packed.uv2 = polygon->texture.uv2;
            prim->packed.uv3 = polygon->texture.uv3;
            tmd_textured_primitive_color.cd = prim->sdk.code;
            NormalColorDpq((SVECTOR *)(normals + polygon->ft4.n0), &tmd_textured_primitive_color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2,
                           &prim->packed.color0);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    (void *)(&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK]),
                    (void *)&prim->sdk);
            }
            break;
        }
        case KF_TMD_MODE_F3: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuF3 *prim;

            vertex0 = VTX(polygon->f3.v0);
            vertex1 = VTX(polygon->f3.v1);
            vertex2 = VTX(polygon->f3.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            prim = (KfGpuF3 *)primitive_buffer_allocate(sizeof(POLY_F3));
            SetPolyF3(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            NormalColorDpq((SVECTOR *)(normals + polygon->f3.n0), &polygon->color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2) / 3,
                           &prim->packed.color0);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    (void *)(&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK]),
                    (void *)&prim->sdk);
            }
            break;
        }
        case (KF_TMD_MODE_G4 | KF_TMD_MODE_SEMITRANS): {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuG4 *prim;

            vertex0 = VTX(polygon->g4.v0);
            vertex1 = VTX(polygon->g4.v1);
            vertex2 = VTX(polygon->g4.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            vertex3 = VTX(polygon->g4.v3);
            prim = (KfGpuG4 *)primitive_buffer_allocate(sizeof(POLY_G4));
            SetPolyG4(&prim->sdk);
            SetSemiTrans((void *)&prim->sdk, 1);
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            prim->packed.xy3 = vertex3->sxy.word;
            NormalColorDpq((SVECTOR *)(normals + polygon->g4.n0), &polygon->color,
                           vertex0->p2, &prim->packed.color0);
            NormalColorDpq((SVECTOR *)(normals + polygon->g4.n1), &polygon->color,
                           vertex0->p2, &prim->packed.color1);
            NormalColorDpq((SVECTOR *)(normals + polygon->g4.n2), &polygon->color,
                           vertex0->p2, &prim->packed.color2);
            NormalColorDpq((SVECTOR *)(normals + polygon->g4.n3), &polygon->color,
                           vertex0->p2, &prim->packed.color3);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    (void *)(&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK]),
                    (void *)&prim->sdk);
            }
            break;
        }
        case (KF_TMD_MODE_F3 | KF_TMD_MODE_SEMITRANS): {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuF3 *prim;

            vertex0 = VTX(polygon->f3.v0);
            vertex1 = VTX(polygon->f3.v1);
            vertex2 = VTX(polygon->f3.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            prim = (KfGpuF3 *)primitive_buffer_allocate(sizeof(POLY_F3));
            SetPolyF3(&prim->sdk);
            SetSemiTrans((void *)&prim->sdk, 1);
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            NormalColorDpq((SVECTOR *)(normals + polygon->f3.n0), &polygon->color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2) / 3,
                           &prim->packed.color0);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    (void *)(&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK]),
                    (void *)&prim->sdk);
            }
            break;
        }
        case (KF_TMD_MODE_F4 | KF_TMD_MODE_SEMITRANS): {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuF4 *prim;

            vertex0 = VTX(polygon->f4.v0);
            vertex1 = VTX(polygon->f4.v1);
            vertex2 = VTX(polygon->f4.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            vertex3 = VTX(polygon->f4.v3);
            prim = (KfGpuF4 *)primitive_buffer_allocate(sizeof(POLY_F4));
            SetPolyF4(&prim->sdk);
            SetSemiTrans((void *)&prim->sdk, 1);
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            prim->packed.xy3 = vertex3->sxy.word;
            NormalColorDpq((SVECTOR *)(normals + polygon->f4.n0), &polygon->color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2,
                           &prim->packed.color0);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    (void *)(&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK]),
                    (void *)&prim->sdk);
            }
            break;
        }
        default:
            continue;
        }
    }
}

#undef VTX
