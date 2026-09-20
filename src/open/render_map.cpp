#include <kf/lib/render_face.h>
#include <kf/open/render.h>

CVECTOR map_textured_primitive_color = {
    KF_TEXTURE_BASE_BRIGHTNESS, KF_TEXTURE_BASE_BRIGHTNESS,
    KF_TEXTURE_BASE_BRIGHTNESS, 0
};

void render_enqueue_map(u16 object_index, const MATRIX *lights, const MATRIX *model, const kf::Projection &projection)
{
    KfTmdPrimitive *primitive;
    KfTmdObject *object = tmd_get_object(object_index);
    u32 header;
    SVECTOR *normals = (SVECTOR *)((u8 *)open_graphics_runtime.tmd_state.current_asset.data +
        (object->normal_offset + KF_TMD_HEADER_BYTES));
    u8 *packet;
    KfScreenVertex *vertex0;
    KfScreenVertex *vertex1;
    KfScreenVertex *vertex2;
    KfScreenVertex *vertex3;
    CVECTOR shade;
    u32 remaining;

    tmd_project_vertices(object->vertex_count, model, projection);
    remaining = object->primitive_count;
    packet = (u8 *)open_graphics_runtime.tmd_state.current_asset.data +
        (object->primitive_offset + KF_TMD_HEADER_BYTES);
    for (; remaining-- != 0;
         packet += TMD_PACKET_BODY_BYTES(header)) {
        KfScreenVertex *vertices = open_graphics_runtime.tmd_projected_vertices;

        header = *(u32 *)packet;
        packet = TMD_PACKET_BODY(packet);
        primitive = (KfTmdPrimitive *)packet;
        switch (tmd_packet_mode(header)) {
        case KF_TMD_MODE_FT4: {
            s32 depth;

            vertex0 = &vertices[primitive->ft4.v0];
            vertex1 = &vertices[primitive->ft4.v1];
            vertex2 = &vertices[primitive->ft4.v2];
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            vertex3 = &vertices[primitive->ft4.v3];
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Quad;
            face.material = render_texture_material(primitive->ft4.tsb, primitive->ft4.cba);
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            render_face_vertex(&face, 3, vertex3);
            render_face_uv(&face, 0, primitive->texture.uv0);
            render_face_uv(&face, 1, primitive->texture.uv1);
            render_face_uv(&face, 2, primitive->texture.uv2);
            render_face_uv(&face, 3, primitive->texture.uv3);
            shade = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[primitive->ft4.n0], map_textured_primitive_color, 0);
            colors[0] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, vertex0->p2);
            colors[1] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, vertex1->p2);
            colors[2] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, vertex2->p2);
            colors[3] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, vertex3->p2);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2))
                + KF_MAP_OT_DEPTH_BIAS;
            if (depth < KF_ORDERING_TABLE_LENGTH) {
                render_face_submit(&face, colors, KfFaceShading::Gouraud, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case KF_TMD_MODE_FT3: {
            s32 depth;

            vertex0 = &vertices[primitive->ft3.v0];
            vertex1 = &vertices[primitive->ft3.v1];
            vertex2 = &vertices[primitive->ft3.v2];
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Triangle;
            face.material = render_texture_material(primitive->ft3.tsb, primitive->ft3.cba);
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            render_face_uv(&face, 0, primitive->texture.uv0);
            render_face_uv(&face, 1, primitive->texture.uv1);
            render_face_uv(&face, 2, primitive->texture.uv2);
            shade = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[primitive->ft3.n0], map_textured_primitive_color, 0);
            colors[0] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, vertex0->p2);
            colors[1] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, vertex1->p2);
            colors[2] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, vertex2->p2);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + KF_MAP_OT_DEPTH_BIAS;
            if (depth < KF_ORDERING_TABLE_LENGTH) {
                render_face_submit(&face, colors, KfFaceShading::Gouraud, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        default:
            continue;
        }
    }
}


void render_map_reset_module_state(void)
{
    kf::restore_initial_value<map_textured_primitive_color>();
}
