#include <kf/null.h>
#include <kf/address.h>
#include <kf/gpu_packets.h>
#include <kf/open_render.h>

/* Prepared vertex indices are byte offsets into the projected array. */
#define VTX(off) TMD_PREPARED_VERTEX(open_graphics_runtime.tmd_projected_vertices, (off))

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
    /* Retail reserves one unallocated stack word that no live value
     * explains; this never-read local carries it
     * (docs/patterns/reconstruction-debt-review.md). */
    u16 unattributed_stack_slot[2];
    u32 header;
    s32 depth;

    for (; remaining-- != 0;
         packet += TMD_PACKET_BODY_BYTES(header)) {
        header = *(u32 *)packet;
        packet = TMD_PACKET_BODY(packet);
        polygon = (KfTmdPrimitive *)packet;
        switch (tmd_packet_mode(header)) {
        case KF_TMD_MODE_FT3: {
            KfGpuFT3 *prim;

            vertex0 = VTX(polygon->ft3.v0);
            vertex1 = VTX(polygon->ft3.v1);
            vertex2 = VTX(polygon->ft3.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word,
                           vertex2->sxy.word) <= 0) {
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
            setRGB0(&prim->sdk,
                open_graphics_runtime.floor_item_state.material.color.r,
                open_graphics_runtime.floor_item_state.material.color.g,
                open_graphics_runtime.floor_item_state.material.color.b);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        case KF_TMD_MODE_F3: {
            KfGpuF3 *prim;

            vertex0 = VTX(polygon->f3.v0);
            vertex1 = VTX(polygon->f3.v1);
            vertex2 = VTX(polygon->f3.v2);
            if (NormalClip(vertex0->sxy.word, vertex1->sxy.word,
                           vertex2->sxy.word) <= 0) {
                continue;
            }
            prim = (KfGpuF3 *)primitive_buffer_allocate(sizeof(POLY_F3));
            SetPolyF3(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy.word;
            prim->packed.xy1 = vertex1->sxy.word;
            prim->packed.xy2 = vertex2->sxy.word;
            setRGB0(&prim->sdk, polygon->f3.r, polygon->f3.g, polygon->f3.b);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        default:
            continue;
        }
    }
}
