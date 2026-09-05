#include <kf/address.h>
#include <kf/gpu_packets.h>
#include <kf/open_render.h>

ADDRESS(0x80018344, 0x2a4)
void render_enqueue_unlit_triangles(u16 object_index, s16 depth_bias)
{
    KfTmdObject *object = tmd_get_object(object_index);
    u32 remaining = object->primitive_count;
    u8 *packet = (u8 *)open_graphics_runtime.tmd_state.current_asset +
        (object->primitive_offset + 12);
    KfScreenVertex *vertex0;
    KfScreenVertex *vertex1;
    KfScreenVertex *vertex2;
    void *primitive;
    u32 header;
    s32 depth;

    while (remaining-- != 0) {
        u8 *vertices = (u8 *)open_graphics_runtime.tmd_projected_vertices;
        s32 bias = depth_bias;

        header = *(u32 *)packet;
        packet += 4;
        switch (header >> 24) {
        case 0x24: {
            KfTmdPrimitive *triangle = (KfTmdPrimitive *)packet;
            KfGpuFT3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + triangle->ft3.v0);
            vertex1 = (KfScreenVertex *)(vertices + triangle->ft3.v1);
            vertex2 = (KfScreenVertex *)(vertices + triangle->ft3.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy,
                           vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_FT3));
            primitive = &prim->sdk;
            SetPolyFT3(&prim->sdk);
            prim->packed.clut = triangle->ft3.cba;
            prim->packed.tpage = triangle->ft3.tsb;
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.uv0 = triangle->texture.uv0;
            prim->packed.uv1 = triangle->texture.uv1;
            prim->packed.uv2 = triangle->texture.uv2;
            prim->sdk.r0 = open_graphics_runtime.floor_item_state.material.color.r;
            prim->sdk.g0 = open_graphics_runtime.floor_item_state.material.color.g;
            prim->sdk.b0 = open_graphics_runtime.floor_item_state.material.color.b;
            break;
        }
        case 0x20: {
            KfTmdPrimitive *triangle = (KfTmdPrimitive *)packet;
            KfGpuF3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + triangle->f3.v0);
            vertex1 = (KfScreenVertex *)(vertices + triangle->f3.v1);
            vertex2 = (KfScreenVertex *)(vertices + triangle->f3.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy,
                           vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_F3));
            primitive = &prim->sdk;
            SetPolyF3(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->sdk.r0 = triangle->f3.r;
            prim->sdk.g0 = triangle->f3.g;
            prim->sdk.b0 = triangle->f3.b;
            break;
        }
        default:
            goto next_packet;
        }
        depth = ((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2;
        depth += bias;
        if (depth >= 5) {
            AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], primitive);
        }

    next_packet:
        packet += (header >> 6) & 0x3fc;
    }
}
