#include <kf/lib/gpu_packets.h>
#include <kf/open/render.h>

CVECTOR map_textured_primitive_color = {
    KF_TEXTURE_BASE_BRIGHTNESS, KF_TEXTURE_BASE_BRIGHTNESS,
    KF_TEXTURE_BASE_BRIGHTNESS, 0
};

void render_enqueue_map(u16 object_index)
{
    KfTmdPrimitive *primitive;
    KfTmdObject *object = tmd_get_object(object_index);
    u32 header;
    u8 *normals = (u8 *)open_graphics_runtime.tmd_state.current_asset +
        (object->normal_offset + KF_TMD_HEADER_BYTES);
    u8 *packet;
    KfScreenVertex *vertex0;
    KfScreenVertex *vertex1;
    KfScreenVertex *vertex2;
    KfScreenVertex *vertex3;
    CVECTOR shade;
    u32 remaining;

    tmd_project_vertices(object->vertex_count);
    remaining = object->primitive_count;
    packet = (u8 *)open_graphics_runtime.tmd_state.current_asset +
        (object->primitive_offset + KF_TMD_HEADER_BYTES);
    for (; remaining-- != 0;
         packet += TMD_PACKET_BODY_BYTES(header)) {
        u8 *vertices = (u8 *)open_graphics_runtime.tmd_projected_vertices;
        union {
            KfGpuGT4 *quad;
            KfGpuGT3 *triangle;
        } prim;

        header = *(u32 *)packet;
        packet = TMD_PACKET_BODY(packet);
        primitive = (KfTmdPrimitive *)packet;
        switch (tmd_packet_mode(header)) {
        case KF_TMD_MODE_FT4: {
            s32 depth;

            vertex0 = TMD_PREPARED_VERTEX(vertices, primitive->ft4.v0);
            vertex1 = TMD_PREPARED_VERTEX(vertices, primitive->ft4.v1);
            vertex2 = TMD_PREPARED_VERTEX(vertices, primitive->ft4.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            vertex3 = TMD_PREPARED_VERTEX(vertices, primitive->ft4.v3);
            prim.quad = (KfGpuGT4 *)primitive_buffer_allocate(sizeof(POLY_GT4));
            SetPolyGT4(&prim.quad->sdk);
            prim.quad->packed.clut = primitive->ft4.cba;
            prim.quad->packed.tpage = primitive->ft4.tsb;
            prim.quad->packed.xy0 = vertex0->sxy.word;
            prim.quad->packed.xy1 = vertex1->sxy.word;
            prim.quad->packed.xy2 = vertex2->sxy.word;
            prim.quad->packed.xy3 = vertex3->sxy.word;
            prim.quad->packed.uv0 = primitive->texture.uv0;
            prim.quad->packed.uv1 = primitive->texture.uv1;
            prim.quad->packed.uv2 = primitive->texture.uv2;
            prim.quad->packed.uv3 = primitive->texture.uv3;
            map_textured_primitive_color.cd = prim.quad->sdk.code;
            NormalColorCol((SVECTOR *)(normals + primitive->ft4.n0),
                           &map_textured_primitive_color, &shade);
            DpqColor(&shade, vertex0->p2, &prim.quad->packed.color0);
            DpqColor(&shade, vertex1->p2, &prim.quad->packed.color1);
            DpqColor(&shade, vertex2->p2, &prim.quad->packed.color2);
            DpqColor(&shade, vertex3->p2, &prim.quad->packed.color3);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2))
                + KF_MAP_OT_DEPTH_BIAS;
            if (depth < KF_ORDERING_TABLE_LENGTH) {
                AddPrim(
                    (void *)&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    (void *)&prim.quad->sdk);
            }
            break;
        }
        case KF_TMD_MODE_FT3: {
            s32 depth;

            vertex0 = TMD_PREPARED_VERTEX(vertices, primitive->ft3.v0);
            vertex1 = TMD_PREPARED_VERTEX(vertices, primitive->ft3.v1);
            vertex2 = TMD_PREPARED_VERTEX(vertices, primitive->ft3.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word, vertex2->sxy.word) <= 0) {
                continue;
            }
            prim.triangle = (KfGpuGT3 *)primitive_buffer_allocate(sizeof(POLY_GT3));
            SetPolyGT3(&prim.triangle->sdk);
            prim.triangle->packed.clut = primitive->ft3.cba;
            prim.triangle->packed.tpage = primitive->ft3.tsb;
            prim.triangle->packed.xy0 = vertex0->sxy.word;
            prim.triangle->packed.xy1 = vertex1->sxy.word;
            prim.triangle->packed.xy2 = vertex2->sxy.word;
            prim.triangle->packed.uv0 = primitive->texture.uv0;
            prim.triangle->packed.uv1 = primitive->texture.uv1;
            prim.triangle->packed.uv2 = primitive->texture.uv2;
            map_textured_primitive_color.cd = prim.triangle->sdk.code;
            NormalColorCol((SVECTOR *)(normals + primitive->ft3.n0),
                           &map_textured_primitive_color, &shade);
            DpqColor(&shade, vertex0->p2, &prim.triangle->packed.color0);
            DpqColor(&shade, vertex1->p2, &prim.triangle->packed.color1);
            DpqColor(&shade, vertex2->p2, &prim.triangle->packed.color2);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + KF_MAP_OT_DEPTH_BIAS;
            if (depth < KF_ORDERING_TABLE_LENGTH) {
                AddPrim(
                    (void *)&open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    (void *)&prim.triangle->sdk);
            }
            break;
        }
        default:
            continue;
        }
    }
}
