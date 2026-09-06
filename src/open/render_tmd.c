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
    u8 *packet = (u8 *)open_graphics_runtime.tmd_state.current_asset +
        (object->primitive_offset + 12);
    u8 *normals = (u8 *)open_graphics_runtime.tmd_state.current_asset +
        (object->normal_offset + 12);
    KfScreenVertex *vertex0;
    KfScreenVertex *vertex1;
    KfScreenVertex *vertex2;
    KfScreenVertex *vertex3;
    u32 header;
    s32 depth;

    while (remaining-- != 0) {
        u8 *vertices = (u8 *)open_graphics_runtime.tmd_projected_vertices;

        header = *(u32 *)packet;
        packet += 4;
        switch (header >> 24) {
        case 0x24: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuFT3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->ft3.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->ft3.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->ft3.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_FT3));
            SetPolyFT3(&prim->sdk);
            prim->packed.clut = polygon->ft3.cba;
            prim->packed.tpage = polygon->ft3.tsb;
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.uv0 = polygon->texture.uv0;
            prim->packed.uv1 = polygon->texture.uv1;
            prim->packed.uv2 = polygon->texture.uv2;
            tmd_textured_primitive_color.cd = prim->sdk.code;
            NormalColorDpq((SVECTOR *)(normals + polygon->ft3.n0), &tmd_textured_primitive_color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2) / 3,
                           &prim->packed.color0);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2) + depth_bias;
            if (depth >= 5) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        case 0x28: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuF4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->f4.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->f4.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->f4.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->f4.v3);
            prim = primitive_buffer_allocate(sizeof(POLY_F4));
            SetPolyF4(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.xy3 = vertex3->sxy;
            NormalColorDpq((SVECTOR *)(normals + polygon->f4.n0), &polygon->color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2,
                           &prim->packed.color0);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4) + depth_bias;
            if (depth >= 5) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        case 0x30: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuG3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->g3.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->g3.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->g3.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_G3));
            SetPolyG3(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            NormalColorDpq3((SVECTOR *)(normals + polygon->g3.n0),
                            (SVECTOR *)(normals + polygon->g3.n1),
                            (SVECTOR *)(normals + polygon->g3.n2), &polygon->color,
                            vertex0->p2, &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2) + depth_bias;
            if (depth >= 5) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        case 0x38: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuG4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->g4.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->g4.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->g4.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->g4.v3);
            prim = primitive_buffer_allocate(sizeof(POLY_G4));
            SetPolyG4(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.xy3 = vertex3->sxy;
            NormalColorDpq3((SVECTOR *)(normals + polygon->g4.n0),
                            (SVECTOR *)(normals + polygon->g4.n1),
                            (SVECTOR *)(normals + polygon->g4.n2), &polygon->color,
                            vertex0->p2, &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            NormalColorDpq((SVECTOR *)(normals + polygon->g4.n3), &polygon->color,
                           vertex0->p2, &prim->packed.color3);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4) + depth_bias;
            if (depth >= 5) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        case 0x34: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuGT3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->gt3.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->gt3.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->gt3.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_GT3));
            SetPolyGT3(&prim->sdk);
            prim->packed.clut = polygon->gt3.cba;
            prim->packed.tpage = polygon->gt3.tsb;
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.uv0 = polygon->texture.uv0;
            prim->packed.uv1 = polygon->texture.uv1;
            prim->packed.uv2 = polygon->texture.uv2;
            tmd_textured_primitive_color.cd = prim->sdk.code;
            NormalColorDpq3((SVECTOR *)(normals + polygon->gt3.n0),
                            (SVECTOR *)(normals + polygon->gt3.n1),
                            (SVECTOR *)(normals + polygon->gt3.n2), &tmd_textured_primitive_color,
                            vertex0->p2, &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2) + depth_bias;
            if (depth >= 5) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        case 0x3c: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuGT4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->gt4.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->gt4.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->gt4.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->gt4.v3);
            prim = primitive_buffer_allocate(sizeof(POLY_GT4));
            SetPolyGT4(&prim->sdk);
            prim->packed.clut = polygon->gt4.cba;
            prim->packed.tpage = polygon->gt4.tsb;
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.xy3 = vertex3->sxy;
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
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4) + depth_bias;
            if (depth >= 5) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        case 0x32: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuG3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->g3.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->g3.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->g3.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_G3));
            SetPolyG3(&prim->sdk);
            SetSemiTrans(&prim->sdk, 1);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            NormalColorCol3((SVECTOR *)(normals + polygon->g3.n0),
                            (SVECTOR *)(normals + polygon->g3.n1),
                            (SVECTOR *)(normals + polygon->g3.n2), &polygon->color,
                            &prim->packed.color0,
                            &prim->packed.color1, &prim->packed.color2);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2) + depth_bias;
            if (depth >= 5) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        case 0x2c: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuFT4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->ft4.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->ft4.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->ft4.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->ft4.v3);
            prim = primitive_buffer_allocate(sizeof(POLY_FT4));
            SetPolyFT4(&prim->sdk);
            prim->packed.clut = polygon->ft4.cba;
            prim->packed.tpage = polygon->ft4.tsb;
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.xy3 = vertex3->sxy;
            prim->packed.uv0 = polygon->texture.uv0;
            prim->packed.uv1 = polygon->texture.uv1;
            prim->packed.uv2 = polygon->texture.uv2;
            prim->packed.uv3 = polygon->texture.uv3;
            tmd_textured_primitive_color.cd = prim->sdk.code;
            NormalColorDpq((SVECTOR *)(normals + polygon->ft4.n0), &tmd_textured_primitive_color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2,
                           &prim->packed.color0);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4) + depth_bias;
            if (depth >= 5) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        case 0x20: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuF3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->f3.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->f3.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->f3.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_F3));
            SetPolyF3(&prim->sdk);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            NormalColorDpq((SVECTOR *)(normals + polygon->f3.n0), &polygon->color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2) / 3,
                           &prim->packed.color0);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2) + depth_bias;
            if (depth >= 5) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        case 0x3a: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuG4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->g4.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->g4.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->g4.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->g4.v3);
            prim = primitive_buffer_allocate(sizeof(POLY_G4));
            SetPolyG4(&prim->sdk);
            SetSemiTrans(&prim->sdk, 1);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.xy3 = vertex3->sxy;
            NormalColorDpq((SVECTOR *)(normals + polygon->g4.n0), &polygon->color,
                           vertex0->p2, &prim->packed.color0);
            NormalColorDpq((SVECTOR *)(normals + polygon->g4.n1), &polygon->color,
                           vertex0->p2, &prim->packed.color1);
            NormalColorDpq((SVECTOR *)(normals + polygon->g4.n2), &polygon->color,
                           vertex0->p2, &prim->packed.color2);
            NormalColorDpq((SVECTOR *)(normals + polygon->g4.n3), &polygon->color,
                           vertex0->p2, &prim->packed.color3);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4) + depth_bias;
            if (depth >= 5) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        case 0x22: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuF3 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->f3.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->f3.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->f3.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            prim = primitive_buffer_allocate(sizeof(POLY_F3));
            SetPolyF3(&prim->sdk);
            SetSemiTrans(&prim->sdk, 1);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            NormalColorDpq((SVECTOR *)(normals + polygon->f3.n0), &polygon->color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2) / 3,
                           &prim->packed.color0);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3) >> 2) + depth_bias;
            if (depth >= 5) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
            }
            break;
        }
        case 0x2a: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;
            KfGpuF4 *prim;

            vertex0 = (KfScreenVertex *)(vertices + polygon->f4.v0);
            vertex1 = (KfScreenVertex *)(vertices + polygon->f4.v1);
            vertex2 = (KfScreenVertex *)(vertices + polygon->f4.v2);
            if (NormalClip(vertex0->sxy, vertex1->sxy, vertex2->sxy) <= 0) {
                goto next_packet;
            }
            vertex3 = (KfScreenVertex *)(vertices + polygon->f4.v3);
            prim = primitive_buffer_allocate(sizeof(POLY_F4));
            SetPolyF4(&prim->sdk);
            SetSemiTrans(&prim->sdk, 1);
            prim->packed.xy0 = vertex0->sxy;
            prim->packed.xy1 = vertex1->sxy;
            prim->packed.xy2 = vertex2->sxy;
            prim->packed.xy3 = vertex3->sxy;
            NormalColorDpq((SVECTOR *)(normals + polygon->f4.n0), &polygon->color,
                           (vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2,
                           &prim->packed.color0);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz) >> 4) + depth_bias;
            if (depth >= 5) {
                AddPrim(
                    &open_graphics_runtime.ordering_table[depth & KF_ORDERING_TABLE_INDEX_MASK],
                    &prim->sdk);
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
