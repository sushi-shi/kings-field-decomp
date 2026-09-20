#include <kf/lib/null.h>

#include <kf/lib/render_face.h>
#include <kf/open/render.h>

#define VTX(index) (&open_graphics_runtime.tmd_projected_vertices[(index)])

void render_enqueue_unlit_triangles(u16 object_index, s16 depth_bias)
{
    KfTmdPrimitive *polygon;
    KfTmdObject *object = tmd_get_object(object_index);
    u32 remaining = object->primitive_count;
    u8 *packet = (u8 *)open_graphics_runtime.tmd_state.current_asset.data +
        (object->primitive_offset + KF_TMD_HEADER_BYTES);
    KfScreenVertex *vertex0;
    KfScreenVertex *vertex1;
    KfScreenVertex *vertex2;
    u32 header;
    s32 depth;

    for (; remaining-- != 0;
         packet += TMD_PACKET_BODY_BYTES(header)) {
        header = *(u32 *)packet;
        packet = TMD_PACKET_BODY(packet);
        polygon = (KfTmdPrimitive *)packet;
        switch (tmd_packet_mode(header)) {
        case KF_TMD_MODE_FT3: {
            vertex0 = VTX(polygon->ft3.v0);
            vertex1 = VTX(polygon->ft3.v1);
            vertex2 = VTX(polygon->ft3.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Triangle;
            face.material = render_texture_material(polygon->ft3.tsb, polygon->ft3.cba);
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            render_face_uv(&face, 0, polygon->texture.uv0);
            render_face_uv(&face, 1, polygon->texture.uv1);
            render_face_uv(&face, 2, polygon->texture.uv2);
            colors[0] = {open_graphics_runtime.floor_item_state.material.color.r, open_graphics_runtime.floor_item_state.material.color.g, open_graphics_runtime.floor_item_state.material.color.b, 0};
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Flat, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case KF_TMD_MODE_F3: {
            vertex0 = VTX(polygon->f3.v0);
            vertex1 = VTX(polygon->f3.v1);
            vertex2 = VTX(polygon->f3.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Triangle;
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            colors[0] = {polygon->f3.r, polygon->f3.g, polygon->f3.b, 0};
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Flat, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        default:
            continue;
        }
    }
}
