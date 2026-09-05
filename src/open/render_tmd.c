#include <kf/address.h>
#include <kf/gpu_packets.h>
#include <kf/open_render.h>

DATA(0x800372f0, 0x4)
CVECTOR tmd_textured_primitive_color = {0x80, 0x80, 0x80, 0};

RODATA(0x800121c0, 0x74)

ADDRESS(0x8001764c, 0xcf8)
void render_enqueue_tmd(u16 object_index, s16 depth_bias)
{
    KfTmdObject *object = tmd_get_object(object_index);
    u32 remaining = object->primitive_count;
    u8 *payload = open_graphics_runtime.tmd_state.current_asset;
    u8 *packet = payload + (object->primitive_offset + 12);
    u8 *normals = payload + (object->normal_offset + 12);
    KfScreenVertex *vertex0;
    KfScreenVertex *vertex1;
    KfScreenVertex *vertex2;
    KfScreenVertex *vertex3;
    KfTmdPrimitive *body;
    u32 header;
    s32 depth;

    while (remaining-- != 0) {
        u8 *vertices = (u8 *)open_graphics_runtime.tmd_projected_vertices;

        header = *(u32 *)packet;
        body = (KfTmdPrimitive *)(packet + 4);
        switch (header >> 24) {
        case 0x24: {
            KfTmdFt3 *polygon = &body->ft3;
            KfGpuFT3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_FT3));
            SetPolyFT3(&prim->sdk);
            prim->packed.clut = polygon->cba;
            prim->packed.tpage = polygon->tsb;
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.uv0 = body->texture.uv0;
            prim->packed.uv1 = body->texture.uv1;
            prim->packed.uv2 = body->texture.uv2;
            tmd_textured_primitive_color.cd = prim->sdk.code;
            NormalColorDpq((SVECTOR *)(normals + polygon->n0), &tmd_textured_primitive_color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2) / 3,
                           &prim->packed.color0);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2;
            depth += depth_bias;
            if (depth >= 5) {
                AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], &prim->sdk);
            }
            break;
        }
        case 0x28: {
            KfTmdF4 *polygon = &body->f4;
            KfGpuF4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->v3);
            prim = primitive_buffer_allocate(sizeof(POLY_F4));
            SetPolyF4(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.xy3 = vertex3->sxy;
            NormalColorDpq((SVECTOR *)(normals + polygon->n0), &body->color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2,
                           &prim->packed.color0);
            depth = (vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4;
            depth += depth_bias;
            if (depth >= 5) {
                AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], &prim->sdk);
            }
            break;
        }
        case 0x30: {
            KfTmdG3 *polygon = &body->g3;
            KfGpuG3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_G3));
            SetPolyG3(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            NormalColorDpq3((SVECTOR *)(normals + polygon->n0),
                            (SVECTOR *)(normals + polygon->n1),
                            (SVECTOR *)(normals + polygon->n2), &body->color,
                            vertex0->p2, &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2;
            depth += depth_bias;
            if (depth >= 5) {
                AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], &prim->sdk);
            }
            break;
        }
        case 0x38: {
            KfTmdG4 *polygon = &body->g4;
            KfGpuG4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->v3);
            prim = primitive_buffer_allocate(sizeof(POLY_G4));
            SetPolyG4(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.xy3 = vertex3->sxy;
            NormalColorDpq3((SVECTOR *)(normals + polygon->n0),
                            (SVECTOR *)(normals + polygon->n1),
                            (SVECTOR *)(normals + polygon->n2), &body->color,
                            vertex0->p2, &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            NormalColorDpq((SVECTOR *)(normals + polygon->n3), &body->color,
                           vertex0->p2, &prim->packed.color3);
            depth = (vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4;
            depth += depth_bias;
            if (depth >= 5) {
                AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], &prim->sdk);
            }
            break;
        }
        case 0x34: {
            KfTmdGt3 *polygon = &body->gt3;
            KfGpuGT3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_GT3));
            SetPolyGT3(&prim->sdk);
            prim->packed.clut = polygon->cba;
            prim->packed.tpage = polygon->tsb;
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.uv0 = body->texture.uv0;
            prim->packed.uv1 = body->texture.uv1;
            prim->packed.uv2 = body->texture.uv2;
            tmd_textured_primitive_color.cd = prim->sdk.code;
            NormalColorDpq3((SVECTOR *)(normals + polygon->n0),
                            (SVECTOR *)(normals + polygon->n1),
                            (SVECTOR *)(normals + polygon->n2), &tmd_textured_primitive_color,
                            vertex0->p2, &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2;
            depth += depth_bias;
            if (depth >= 5) {
                AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], &prim->sdk);
            }
            break;
        }
        case 0x3c: {
            KfTmdGt4 *polygon = &body->gt4;
            KfGpuGT4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->v3);
            prim = primitive_buffer_allocate(sizeof(POLY_GT4));
            SetPolyGT4(&prim->sdk);
            prim->packed.clut = polygon->cba;
            prim->packed.tpage = polygon->tsb;
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.xy3 = vertex3->sxy;
            prim->packed.uv0 = body->texture.uv0;
            prim->packed.uv1 = body->texture.uv1;
            prim->packed.uv2 = body->texture.uv2;
            prim->packed.uv3 = body->texture.uv3;
            tmd_textured_primitive_color.cd = prim->sdk.code;
            NormalColorDpq3((SVECTOR *)(normals + polygon->n0),
                            (SVECTOR *)(normals + polygon->n1),
                            (SVECTOR *)(normals + polygon->n2), &tmd_textured_primitive_color,
                            vertex0->p2, &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            NormalColorDpq((SVECTOR *)(normals + polygon->n3), &tmd_textured_primitive_color,
                           vertex0->p2, &prim->packed.color3);
            depth = (vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4;
            depth += depth_bias;
            if (depth >= 5) {
                AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], &prim->sdk);
            }
            break;
        }
        case 0x32: {
            KfTmdG3 *polygon = &body->g3;
            KfGpuG3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_G3));
            SetPolyG3(&prim->sdk);
            SetSemiTrans(&prim->sdk, 1);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            NormalColorCol3((SVECTOR *)(normals + polygon->n0),
                            (SVECTOR *)(normals + polygon->n1),
                            (SVECTOR *)(normals + polygon->n2), &body->color,
                            &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2;
            depth += depth_bias;
            if (depth >= 5) {
                AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], &prim->sdk);
            }
            break;
        }
        case 0x2c: {
            KfTmdFt4 *polygon = &body->ft4;
            KfGpuFT4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->v3);
            prim = primitive_buffer_allocate(sizeof(POLY_FT4));
            SetPolyFT4(&prim->sdk);
            prim->packed.clut = polygon->cba;
            prim->packed.tpage = polygon->tsb;
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.xy3 = vertex3->sxy;
            prim->packed.uv0 = body->texture.uv0;
            prim->packed.uv1 = body->texture.uv1;
            prim->packed.uv2 = body->texture.uv2;
            prim->packed.uv3 = body->texture.uv3;
            tmd_textured_primitive_color.cd = prim->sdk.code;
            NormalColorDpq((SVECTOR *)(normals + polygon->n0), &tmd_textured_primitive_color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2,
                           &prim->packed.color0);
            depth = (vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4;
            depth += depth_bias;
            if (depth >= 5) {
                AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], &prim->sdk);
            }
            break;
        }
        case 0x20: {
            KfTmdF3 *polygon = &body->f3;
            KfGpuF3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_F3));
            SetPolyF3(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            NormalColorDpq((SVECTOR *)(normals + polygon->n0), &body->color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2) / 3,
                           &prim->packed.color0);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2;
            depth += depth_bias;
            if (depth >= 5) {
                AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], &prim->sdk);
            }
            break;
        }
        case 0x3a: {
            KfTmdG4 *polygon = &body->g4;
            KfGpuG4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->v3);
            prim = primitive_buffer_allocate(sizeof(POLY_G4));
            SetPolyG4(&prim->sdk);
            SetSemiTrans(&prim->sdk, 1);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.xy3 = vertex3->sxy;
            NormalColorDpq((SVECTOR *)(normals + polygon->n0), &body->color,
                           vertex0->p2, &prim->packed.color0);
            NormalColorDpq((SVECTOR *)(normals + polygon->n1), &body->color,
                           vertex0->p2, &prim->packed.color1);
            NormalColorDpq((SVECTOR *)(normals + polygon->n2), &body->color,
                           vertex0->p2, &prim->packed.color2);
            NormalColorDpq((SVECTOR *)(normals + polygon->n3), &body->color,
                           vertex0->p2, &prim->packed.color3);
            depth = (vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4;
            depth += depth_bias;
            if (depth >= 5) {
                AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], &prim->sdk);
            }
            break;
        }
        case 0x22: {
            KfTmdF3 *polygon = &body->f3;
            KfGpuF3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_F3));
            SetPolyF3(&prim->sdk);
            SetSemiTrans(&prim->sdk, 1);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            NormalColorDpq((SVECTOR *)(normals + polygon->n0), &body->color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2) / 3,
                           &prim->packed.color0);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2;
            depth += depth_bias;
            if (depth >= 5) {
                AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], &prim->sdk);
            }
            break;
        }
        case 0x2a: {
            KfTmdF4 *polygon = &body->f4;
            KfGpuF4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->v3);
            prim = primitive_buffer_allocate(sizeof(POLY_F4));
            SetPolyF4(&prim->sdk);
            SetSemiTrans(&prim->sdk, 1);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.xy3 = vertex3->sxy;
            NormalColorDpq((SVECTOR *)(normals + polygon->n0), &body->color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2,
                           &prim->packed.color0);
            depth = (vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4;
            depth += depth_bias;
            if (depth >= 5) {
                AddPrim(&open_graphics_runtime.ordering_table[depth & 0x3fff], &prim->sdk);
            }
            break;
        }
        default:
            goto next_packet;
        }

    next_packet:
        packet = (u8 *)body + ((header >> 6) & 0x3fc);
    }
}
