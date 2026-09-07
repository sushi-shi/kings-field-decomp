#include <kf/address.h>
#include <kf/gpu_packets.h>
#include <kf/open_render.h>

/* Each mode allocates only its active SDK packet variant. */
typedef union KfMapGpuPrimitive {
    KfGpuGT3 triangle;
    KfGpuGT4 quad;
} KfMapGpuPrimitive;

DATA(0x800372f8, 0x4)
CVECTOR map_textured_primitive_color = {
    KF_TEXTURE_BASE_BRIGHTNESS, KF_TEXTURE_BASE_BRIGHTNESS,
    KF_TEXTURE_BASE_BRIGHTNESS, 0
};

ADDRESS(0x800185e8, 0x3b8)
void render_enqueue_map(u16 object_index)
{
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
    while (remaining-- != 0) {
        u8 *vertices = (u8 *)open_graphics_runtime.tmd_projected_vertices;
        KfMapGpuPrimitive *prim;

        header = *(u32 *)packet;
        packet += KF_TMD_PACKET_HEADER_BYTES;
        switch (header >> KF_TMD_MODE_SHIFT) {
        case KF_TMD_MODE_FT4: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            s32 depth;

            vertex0 = (KfScreenVertex *)(vertices + polygon->ft4.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->ft4.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->ft4.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->ft4.v3);
            prim = primitive_buffer_allocate(sizeof(POLY_GT4));
            SetPolyGT4(&prim->quad.sdk);
            prim->quad.packed.clut = polygon->ft4.cba;
            prim->quad.packed.tpage = polygon->ft4.tsb;
            prim->quad.packed.xy0 = vertex0->sxy;
            prim->quad.packed.xy1 = vertex1->sxy;
            prim->quad.packed.xy2 = vertex2->sxy;
            prim->quad.packed.xy3 = vertex3->sxy;
            prim->quad.packed.uv0 = polygon->texture.uv0;
            prim->quad.packed.uv1 = polygon->texture.uv1;
            prim->quad.packed.uv2 = polygon->texture.uv2;
            prim->quad.packed.uv3 = polygon->texture.uv3;
            map_textured_primitive_color.cd = prim->quad.sdk.code;
            NormalColorCol((SVECTOR *)(normals + polygon->ft4.n0),
                           &map_textured_primitive_color, &shade);
            DpqColor(&shade, vertex0->p2, &prim->quad.packed.color0);
            DpqColor(&shade, vertex1->p2, &prim->quad.packed.color1);
            DpqColor(&shade, vertex2->p2, &prim->quad.packed.color2);
            DpqColor(&shade, vertex3->p2, &prim->quad.packed.color3);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2))
                + KF_MAP_OT_DEPTH_BIAS;
            if (depth < KF_ORDERING_TABLE_LENGTH) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->quad.sdk);
            }
            break;
        }
        case KF_TMD_MODE_FT3: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            s32 depth;

            vertex0 = (KfScreenVertex *)(vertices + polygon->ft3.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->ft3.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->ft3.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_GT3));
            SetPolyGT3(&prim->triangle.sdk);
            prim->triangle.packed.clut = polygon->ft3.cba;
            prim->triangle.packed.tpage = polygon->ft3.tsb;
            prim->triangle.packed.xy0 = vertex0->sxy;
            prim->triangle.packed.xy1 = vertex1->sxy;
            prim->triangle.packed.xy2 = vertex2->sxy;
            prim->triangle.packed.uv0 = polygon->texture.uv0;
            prim->triangle.packed.uv1 = polygon->texture.uv1;
            prim->triangle.packed.uv2 = polygon->texture.uv2;
            map_textured_primitive_color.cd = prim->triangle.sdk.code;
            NormalColorCol((SVECTOR *)(normals + polygon->ft3.n0),
                           &map_textured_primitive_color, &shade);
            DpqColor(&shade, vertex0->p2, &prim->triangle.packed.color0);
            DpqColor(&shade, vertex1->p2, &prim->triangle.packed.color1);
            DpqColor(&shade, vertex2->p2, &prim->triangle.packed.color2);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + KF_MAP_OT_DEPTH_BIAS;
            if (depth < KF_ORDERING_TABLE_LENGTH) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->triangle.sdk);
            }
            break;
        }
        default:
            goto next_packet;
        }
    next_packet:
        packet += (header >> KF_TMD_ILEN_TO_BYTES_SHIFT) & KF_TMD_BODY_BYTES_MASK;
    }
}
