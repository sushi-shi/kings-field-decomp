#include <kf/address.h>
#include <kf/open_render.h>

DATA(0x800372f8, 0x4)
CVECTOR map_textured_primitive_color = {0x80, 0x80, 0x80, 0};

ADDRESS(0x800185e8, 0x3b8)
void render_enqueue_map(u16 object_index)
{
    KfTmdObject *object = tmd_get_object(object_index);
    u8 *normals = (u8 *)tmd_state.current_asset + (object->normal_offset + 12);
    u8 *packet;
    u8 *vertices;
    KfScreenVertex *vertex0;
    KfScreenVertex *vertex1;
    KfScreenVertex *vertex2;
    KfScreenVertex *vertex3;
    CVECTOR shade;
    u32 remaining;
    u32 header;
    s32 depth;

    tmd_project_vertices(object->vertex_count);
    remaining = object->primitive_count;
    packet = (u8 *)tmd_state.current_asset + (object->primitive_offset + 12);
    vertices = (u8 *)tmd_projected_vertices;
    while (remaining-- != 0) {
        header = *(u32 *)packet;
        packet += 4;
        switch (header >> 24) {
        case 0x2c: {
            KfTmdFt4 *quad = (KfTmdFt4 *)packet;
            POLY_GT4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + quad->v0);
            vertex1 = (KfScreenVertex *)(vertices + quad->v1);
            vertex2 = (KfScreenVertex *)(vertices + quad->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + quad->v3);
            prim = primitive_buffer_allocate(sizeof(POLY_GT4));
            SetPolyGT4(prim);
            prim->clut = quad->cba;
            prim->tpage = quad->tsb;
            *(long *)&prim->x0 = vertex0->sxy;
            *(long *)&prim->x1 = vertex1->sxy;
            *(long *)&prim->x2 = vertex2->sxy;
            *(long *)&prim->x3 = vertex3->sxy;
            *(u16 *)&prim->u0 = *(u16 *)&quad->tu0;
            *(u16 *)&prim->u1 = *(u16 *)&quad->tu1;
            *(u16 *)&prim->u2 = *(u16 *)&quad->tu2;
            *(u16 *)&prim->u3 = *(u16 *)&quad->tu3;
            map_textured_primitive_color.cd = prim->code;
            NormalColorCol((SVECTOR *)(normals + quad->n0),
                           &map_textured_primitive_color, &shade);
            DpqColor(&shade, vertex0->p2, (CVECTOR *)&prim->r0);
            DpqColor(&shade, vertex1->p2, (CVECTOR *)&prim->r1);
            DpqColor(&shade, vertex2->p2, (CVECTOR *)&prim->r2);
            DpqColor(&shade, vertex3->p2, (CVECTOR *)&prim->r3);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4)
                + 200;
            if (depth < 16384) {
                AddPrim(&ordering_table[depth & 0x3fff], prim);
            }
            break;
        }
        case 0x24: {
            KfTmdFt3 *triangle = (KfTmdFt3 *)packet;
            POLY_GT3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + triangle->v0);
            vertex1 = (KfScreenVertex *)(vertices + triangle->v1);
            vertex2 = (KfScreenVertex *)(vertices + triangle->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_GT3));
            SetPolyGT3(prim);
            prim->clut = triangle->cba;
            prim->tpage = triangle->tsb;
            *(long *)&prim->x0 = vertex0->sxy;
            *(long *)&prim->x1 = vertex1->sxy;
            *(long *)&prim->x2 = vertex2->sxy;
            *(u16 *)&prim->u0 = *(u16 *)&triangle->tu0;
            *(u16 *)&prim->u1 = *(u16 *)&triangle->tu1;
            *(u16 *)&prim->u2 = *(u16 *)&triangle->tu2;
            map_textured_primitive_color.cd = prim->code;
            NormalColorCol((SVECTOR *)(normals + triangle->n0),
                           &map_textured_primitive_color, &shade);
            DpqColor(&shade, vertex0->p2, (CVECTOR *)&prim->r0);
            DpqColor(&shade, vertex1->p2, (CVECTOR *)&prim->r1);
            DpqColor(&shade, vertex2->p2, (CVECTOR *)&prim->r2);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2) + 200;
            if (depth < 16384) {
                AddPrim(&ordering_table[depth & 0x3fff], prim);
            }
            break;
        }
        default:
            goto next_packet;
        }
    next_packet:
        packet += (header >> 6) & 0x3fc;
    }
}
