#include <kf/address.h>
#include <kf/open_render.h>

ADDRESS(0x80018344, 0x2a4)
void render_enqueue_unlit_triangles(u16 object_index, s16 depth_bias)
{
    KfTmdObject *object = tmd_get_object(object_index);
    u32 remaining = object->primitive_count;
    u8 *payload = open_graphics_runtime.tmd_state.current_asset;
    u8 *packet = payload + (object->primitive_offset + 12);
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
            KfTmdFt3 *triangle = (KfTmdFt3 *)packet;
            POLY_FT3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + triangle->v0);
            vertex1 = (KfScreenVertex *)(vertices + triangle->v1);
            vertex2 = (KfScreenVertex *)(vertices + triangle->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy,
                           vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_FT3));
            primitive = prim;
            SetPolyFT3(prim);
            prim->clut = triangle->cba;
            prim->tpage = triangle->tsb;
            /* Packed GTE coordinates and texture pairs retain their word widths. */
            *(long *)&prim->x0 = vertex0->sxy;
            *(long *)&prim->x1 = vertex1->sxy;
            *(long *)&prim->x2 = vertex2->sxy;
            *(u16 *)&prim->u0 = *(u16 *)&triangle->tu0;
            *(u16 *)&prim->u1 = *(u16 *)&triangle->tu1;
            *(u16 *)&prim->u2 = *(u16 *)&triangle->tu2;
            prim->r0 = open_graphics_runtime.floor_item_state.material.color.r;
            prim->g0 = open_graphics_runtime.floor_item_state.material.color.g;
            prim->b0 = open_graphics_runtime.floor_item_state.material.color.b;
            break;
        }
        case 0x20: {
            KfTmdF3 *triangle = (KfTmdF3 *)packet;
            POLY_F3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + triangle->v0);
            vertex1 = (KfScreenVertex *)(vertices + triangle->v1);
            vertex2 = (KfScreenVertex *)(vertices + triangle->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy,
                           vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_F3));
            primitive = prim;
            SetPolyF3(prim);
            *(long *)&prim->x0 = vertex0->sxy;
            *(long *)&prim->x1 = vertex1->sxy;
            *(long *)&prim->x2 = vertex2->sxy;
            prim->r0 = triangle->r;
            prim->g0 = triangle->g;
            prim->b0 = triangle->b;
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
