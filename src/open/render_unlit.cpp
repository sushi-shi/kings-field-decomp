#include <kf/lib/null.h>

#include <kf/lib/render_face.h>
#include <kf/open/render.h>

void render_enqueue_unlit_triangles(u16 object_index, s16 depth_bias)
{
    KfTmdPrimitive *polygon;
    KfTmdObject *object = tmd_get_object(tmd_context(), object_index);
    u32 remaining = object->primitive_count;
    u8 *packet = (u8 *)open_graphics_runtime.tmd_state.current_asset.data +
        (object->primitive_offset + KF_TMD_HEADER_BYTES);
    u32 header;
    s32 depth;

    for (; remaining-- != 0;
         packet += TMD_PACKET_BODY_BYTES(header)) {
        header = *(u32 *)packet;
        packet = TMD_PACKET_BODY(packet);
        polygon = (KfTmdPrimitive *)packet;
        switch (tmd_packet_mode(header)) {
        case KF_TMD_MODE_FT3: {
            auto projected = render_projected_triangle(
                open_graphics_runtime.tmd_projected_vertices,
                polygon->ft3.v0, polygon->ft3.v1, polygon->ft3.v2);
            if (!projected) {
                continue;
            }
            auto face = projected->draw_face();
            CVECTOR colors[4] {};
            face.material = render_texture_material(polygon->ft3.tsb, polygon->ft3.cba);
            render_face_uvs(face, {polygon->texture.uv0, polygon->texture.uv1, polygon->texture.uv2});
            colors[0] = {open_graphics_runtime.floor_item_state.material.color.r, open_graphics_runtime.floor_item_state.material.color.g, open_graphics_runtime.floor_item_state.material.color.b, 0};
            depth = projected->ordering_depth() + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, kf::FaceShading::Flat, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case KF_TMD_MODE_F3: {
            auto projected = render_projected_triangle(
                open_graphics_runtime.tmd_projected_vertices,
                polygon->f3.v0, polygon->f3.v1, polygon->f3.v2);
            if (!projected) {
                continue;
            }
            auto face = projected->draw_face();
            CVECTOR colors[4] {};
            colors[0] = {polygon->f3.r, polygon->f3.g, polygon->f3.b, 0};
            depth = projected->ordering_depth() + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, kf::FaceShading::Flat, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        default:
            continue;
        }
    }
}
