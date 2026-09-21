#include <kf/lib/render_face.h>
#include <kf/open/render.h>

CVECTOR map_textured_primitive_color = {
    KF_TEXTURE_BASE_BRIGHTNESS, KF_TEXTURE_BASE_BRIGHTNESS,
    KF_TEXTURE_BASE_BRIGHTNESS, 0
};

void render_enqueue_map(u16 object_index, const MATRIX *lights, const MATRIX *model, const kf::Projection &projection)
{
    KfTmdPrimitive *primitive;
    KfTmdObject *object = tmd_get_object(tmd_context(), object_index);
    u32 header;
    SVECTOR *normals = (SVECTOR *)((u8 *)open_graphics_runtime.tmd_state.current_asset.data +
        (object->normal_offset + KF_TMD_HEADER_BYTES));
    u8 *packet;
    CVECTOR shade;
    u32 remaining;

    tmd_project_vertices(object->vertex_count, model, projection);
    remaining = object->primitive_count;
    packet = (u8 *)open_graphics_runtime.tmd_state.current_asset.data +
        (object->primitive_offset + KF_TMD_HEADER_BYTES);
    for (; remaining-- != 0;
         packet += TMD_PACKET_BODY_BYTES(header)) {
        header = *(u32 *)packet;
        packet = TMD_PACKET_BODY(packet);
        primitive = (KfTmdPrimitive *)packet;
        switch (tmd_packet_mode(header)) {
        case KF_TMD_MODE_FT4: {
            s32 depth;

            auto projected = render_projected_triangle(
                open_graphics_runtime.tmd_projected_vertices,
                primitive->ft4.v0, primitive->ft4.v1, primitive->ft4.v2);
            if (!projected) {
                continue;
            }
            projected->complete_quad(open_graphics_runtime.tmd_projected_vertices[primitive->ft4.v3]);
            auto face = projected->draw_face();
            CVECTOR colors[4] {};
            face.material = render_texture_material(primitive->ft4.tsb, primitive->ft4.cba);
            render_face_uvs(face, {primitive->texture.uv0, primitive->texture.uv1, primitive->texture.uv2, primitive->texture.uv3});
            shade = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[primitive->ft4.n0], map_textured_primitive_color, 0);
            colors[0] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, projected->fog(0));
            colors[1] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, projected->fog(1));
            colors[2] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, projected->fog(2));
            colors[3] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, projected->fog(3));
            depth = projected->ordering_depth()
                + KF_MAP_OT_DEPTH_BIAS;
            if (depth < KF_ORDERING_TABLE_LENGTH) {
                render_face_submit(&face, colors, kf::FaceShading::Gouraud, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case KF_TMD_MODE_FT3: {
            s32 depth;

            auto projected = render_projected_triangle(
                open_graphics_runtime.tmd_projected_vertices,
                primitive->ft3.v0, primitive->ft3.v1, primitive->ft3.v2);
            if (!projected) {
                continue;
            }
            auto face = projected->draw_face();
            CVECTOR colors[4] {};
            face.material = render_texture_material(primitive->ft3.tsb, primitive->ft3.cba);
            render_face_uvs(face, {primitive->texture.uv0, primitive->texture.uv1, primitive->texture.uv2});
            shade = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[primitive->ft3.n0], map_textured_primitive_color, 0);
            colors[0] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, projected->fog(0));
            colors[1] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, projected->fog(1));
            colors[2] = kf::render_fog_color(open_graphics_runtime.render_state.lighting, shade, projected->fog(2));
            depth = projected->ordering_depth() + KF_MAP_OT_DEPTH_BIAS;
            if (depth < KF_ORDERING_TABLE_LENGTH) {
                render_face_submit(&face, colors, kf::FaceShading::Gouraud, depth & KF_ORDERING_TABLE_INDEX_MASK);
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
