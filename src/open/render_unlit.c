#include <kf/address.h>
#include <kf/gpu_packets.h>
#include <kf/open_render.h>

ADDRESS(0x80018344, 0x2a4)
void render_enqueue_unlit_triangles(u16 object_index, s16 depth_bias)
{
    KfTmdPrimitive *polygon;
    KfTmdObject *object = tmd_get_object(object_index);
    u32 remaining = object->primitive_count;
    u8 *packet = (u8 *)open_graphics_runtime.tmd_state.current_asset +
        (object->primitive_offset + KF_TMD_HEADER_BYTES);
    KfScreenVertex *vertex0;
    KfScreenVertex *vertex1;
    KfScreenVertex *vertex2;
    void *primitive;
    u32 header;
    s32 depth;

    for (; remaining-- != 0;
         packet += (header >> KF_TMD_ILEN_TO_BYTES_SHIFT) & KF_TMD_BODY_BYTES_MASK) {
        u8 *vertices = (u8 *)open_graphics_runtime.tmd_projected_vertices;
        s32 bias = depth_bias;

        header = *(u32 *)packet;
        packet += KF_TMD_PACKET_HEADER_BYTES;
        polygon = (KfTmdPrimitive *)packet;
        switch (tmd_packet_mode(header)) {
        case KF_TMD_MODE_FT3: {
            KfGpuFT3 *prim;
            u16 vertex1_offset;
            u16 vertex2_offset;

            vertex0 = (KfScreenVertex *)(vertices + polygon->ft3.v0);
            vertex1_offset = polygon->ft3.v1;
            vertex1 = (KfScreenVertex *)(vertices + vertex1_offset);
            vertex2_offset = polygon->ft3.v2;
            /* Prepared indices are byte offsets into the same projected array. */
            vertex2 = (KfScreenVertex *)((u8 *)vertex1 +
                (vertex2_offset - vertex1_offset));
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word,
                           vertex2->sxy.word) <= 0) {
                continue;
            }
            prim = (KfGpuFT3 *)primitive_buffer_allocate(sizeof(POLY_FT3));
            primitive = &prim->sdk;
            SetPolyFT3(&prim->sdk);
            prim->packed.clut = polygon->ft3.cba;
            prim->packed.tpage = polygon->ft3.tsb;
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            prim->packed.uv0 = polygon->texture.uv0;
            prim->packed.uv1 = polygon->texture.uv1;
            prim->packed.uv2 = polygon->texture.uv2;
            prim->sdk.r0 = open_graphics_runtime.floor_item_state.material.color.r;
            prim->sdk.g0 = open_graphics_runtime.floor_item_state.material.color.g;
            prim->sdk.b0 = open_graphics_runtime.floor_item_state.material.color.b;
            break;
        }
        case KF_TMD_MODE_F3: {
            KfGpuF3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->f3.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->f3.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->f3.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word,
                           vertex2->sxy.word) <= 0) {
                continue;
            }
            prim = (KfGpuF3 *)primitive_buffer_allocate(sizeof(POLY_F3));
            primitive = &prim->sdk;
            SetPolyF3(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            prim->sdk.r0 = polygon->f3.r;
            prim->sdk.g0 = polygon->f3.g;
            prim->sdk.b0 = polygon->f3.b;
            break;
        }
        default:
            continue;
        }
        depth = ((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> KF_GTE_DEPTH_TO_OT_SHIFT;
        depth += bias;
        if (depth >= KF_SCENE_MIN_OT_DEPTH) {
            /* The projected array and active OT share this complete owner. */
            KfGraphicsRuntimeOpen *graphics = (KfGraphicsRuntimeOpen *)(
                vertices - (unsigned long)&((KfGraphicsRuntimeOpen *)0)->
                    tmd_projected_vertices);

            AddPrim(
                &graphics->ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                primitive);
        }
    }
}
