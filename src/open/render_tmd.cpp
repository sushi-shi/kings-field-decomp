#include <kf/lib/null.h>

#include <kf/lib/render_face.h>
#include <kf/open/render.h>

CVECTOR tmd_textured_primitive_color = {
    KF_TEXTURE_BASE_BRIGHTNESS, KF_TEXTURE_BASE_BRIGHTNESS,
    KF_TEXTURE_BASE_BRIGHTNESS, 0
};

void render_enqueue_tmd(u16 object_index, s16 depth_bias, const MATRIX *lights)
{
    KfTmdObject *object = tmd_get_object(tmd_context(), object_index);
    u32 header;
    u32 remaining = object->primitive_count;
    u8 *packet = (u8 *)open_graphics_runtime.tmd_state.current_asset.data +
        (object->primitive_offset + KF_TMD_HEADER_BYTES);
    SVECTOR *normals = (SVECTOR *)((u8 *)open_graphics_runtime.tmd_state.current_asset.data +
        (object->normal_offset + KF_TMD_HEADER_BYTES));
    KfScreenVertex *vertex0;
    KfScreenVertex *vertex1;
    KfScreenVertex *vertex2;
    KfScreenVertex *vertex3;
    s32 depth;

    for (; remaining-- != 0;
         packet += TMD_PACKET_BODY_BYTES(header)) {
        header = *(u32 *)packet;
        packet = TMD_PACKET_BODY(packet);
        switch (tmd_packet_mode(header)) {
        case KF_TMD_MODE_FT3: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;

            vertex0 = tmd_projected_vertex(polygon->ft3.v0);
            vertex1 = tmd_projected_vertex(polygon->ft3.v1);
            vertex2 = tmd_projected_vertex(polygon->ft3.v2);
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
            colors[0] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->ft3.n0], tmd_textured_primitive_color, (vertex0->p2 + vertex1->p2 + vertex2->p2) / 3);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Flat, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case KF_TMD_MODE_F4: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;

            vertex0 = tmd_projected_vertex(polygon->f4.v0);
            vertex1 = tmd_projected_vertex(polygon->f4.v1);
            vertex2 = tmd_projected_vertex(polygon->f4.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            vertex3 = tmd_projected_vertex(polygon->f4.v3);
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Quad;
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            render_face_vertex(&face, 3, vertex3);
            colors[0] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->f4.n0], polygon->color, (vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Flat, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case KF_TMD_MODE_G3: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;

            vertex0 = tmd_projected_vertex(polygon->g3.v0);
            vertex1 = tmd_projected_vertex(polygon->g3.v1);
            vertex2 = tmd_projected_vertex(polygon->g3.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Triangle;
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            colors[0] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g3.n0], polygon->color, vertex0->p2);
            colors[1] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g3.n1], polygon->color, vertex0->p2);
            colors[2] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g3.n2], polygon->color, vertex0->p2);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Gouraud, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case KF_TMD_MODE_G4: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;

            vertex0 = tmd_projected_vertex(polygon->g4.v0);
            vertex1 = tmd_projected_vertex(polygon->g4.v1);
            vertex2 = tmd_projected_vertex(polygon->g4.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            vertex3 = tmd_projected_vertex(polygon->g4.v3);
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Quad;
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            render_face_vertex(&face, 3, vertex3);
            colors[0] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g4.n0], polygon->color, vertex0->p2);
            colors[1] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g4.n1], polygon->color, vertex0->p2);
            colors[2] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g4.n2], polygon->color, vertex0->p2);
            colors[3] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g4.n3], polygon->color, vertex0->p2);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Gouraud, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case KF_TMD_MODE_GT3: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;

            vertex0 = tmd_projected_vertex(polygon->gt3.v0);
            vertex1 = tmd_projected_vertex(polygon->gt3.v1);
            vertex2 = tmd_projected_vertex(polygon->gt3.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Triangle;
            face.material = render_texture_material(polygon->gt3.tsb, polygon->gt3.cba);
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            render_face_uv(&face, 0, polygon->texture.uv0);
            render_face_uv(&face, 1, polygon->texture.uv1);
            render_face_uv(&face, 2, polygon->texture.uv2);
            colors[0] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->gt3.n0], tmd_textured_primitive_color, vertex0->p2);
            colors[1] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->gt3.n1], tmd_textured_primitive_color, vertex0->p2);
            colors[2] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->gt3.n2], tmd_textured_primitive_color, vertex0->p2);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Gouraud, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case KF_TMD_MODE_GT4: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;

            vertex0 = tmd_projected_vertex(polygon->gt4.v0);
            vertex1 = tmd_projected_vertex(polygon->gt4.v1);
            vertex2 = tmd_projected_vertex(polygon->gt4.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            vertex3 = tmd_projected_vertex(polygon->gt4.v3);
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Quad;
            face.material = render_texture_material(polygon->gt4.tsb, polygon->gt4.cba);
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            render_face_vertex(&face, 3, vertex3);
            render_face_uv(&face, 0, polygon->texture.uv0);
            render_face_uv(&face, 1, polygon->texture.uv1);
            render_face_uv(&face, 2, polygon->texture.uv2);
            render_face_uv(&face, 3, polygon->texture.uv3);
            colors[0] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->gt4.n0], tmd_textured_primitive_color, vertex0->p2);
            colors[1] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->gt4.n1], tmd_textured_primitive_color, vertex0->p2);
            colors[2] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->gt4.n2], tmd_textured_primitive_color, vertex0->p2);
            colors[3] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->gt4.n3], tmd_textured_primitive_color, vertex0->p2);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Gouraud, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case (KF_TMD_MODE_G3 | KF_TMD_MODE_SEMITRANS): {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;

            vertex0 = tmd_projected_vertex(polygon->g3.v0);
            vertex1 = tmd_projected_vertex(polygon->g3.v1);
            vertex2 = tmd_projected_vertex(polygon->g3.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Triangle;
            face.transparency = kf::FaceTransparency::Blend;
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            colors[0] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g3.n0], polygon->color, 0);
            colors[1] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g3.n1], polygon->color, 0);
            colors[2] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g3.n2], polygon->color, 0);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Gouraud, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case KF_TMD_MODE_FT4: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;

            vertex0 = tmd_projected_vertex(polygon->ft4.v0);
            vertex1 = tmd_projected_vertex(polygon->ft4.v1);
            vertex2 = tmd_projected_vertex(polygon->ft4.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            vertex3 = tmd_projected_vertex(polygon->ft4.v3);
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Quad;
            face.material = render_texture_material(polygon->ft4.tsb, polygon->ft4.cba);
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            render_face_vertex(&face, 3, vertex3);
            render_face_uv(&face, 0, polygon->texture.uv0);
            render_face_uv(&face, 1, polygon->texture.uv1);
            render_face_uv(&face, 2, polygon->texture.uv2);
            render_face_uv(&face, 3, polygon->texture.uv3);
            colors[0] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->ft4.n0], tmd_textured_primitive_color, (vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Flat, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case KF_TMD_MODE_F3: {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;

            vertex0 = tmd_projected_vertex(polygon->f3.v0);
            vertex1 = tmd_projected_vertex(polygon->f3.v1);
            vertex2 = tmd_projected_vertex(polygon->f3.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Triangle;
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            colors[0] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->f3.n0], polygon->color, (vertex0->p2 + vertex1->p2 + vertex2->p2) / 3);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Flat, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case (KF_TMD_MODE_G4 | KF_TMD_MODE_SEMITRANS): {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;

            vertex0 = tmd_projected_vertex(polygon->g4.v0);
            vertex1 = tmd_projected_vertex(polygon->g4.v1);
            vertex2 = tmd_projected_vertex(polygon->g4.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            vertex3 = tmd_projected_vertex(polygon->g4.v3);
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Quad;
            face.transparency = kf::FaceTransparency::Blend;
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            render_face_vertex(&face, 3, vertex3);
            colors[0] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g4.n0], polygon->color, vertex0->p2);
            colors[1] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g4.n1], polygon->color, vertex0->p2);
            colors[2] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g4.n2], polygon->color, vertex0->p2);
            colors[3] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->g4.n3], polygon->color, vertex0->p2);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Gouraud, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case (KF_TMD_MODE_F3 | KF_TMD_MODE_SEMITRANS): {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;

            vertex0 = tmd_projected_vertex(polygon->f3.v0);
            vertex1 = tmd_projected_vertex(polygon->f3.v1);
            vertex2 = tmd_projected_vertex(polygon->f3.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Triangle;
            face.transparency = kf::FaceTransparency::Blend;
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            colors[0] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->f3.n0], polygon->color, (vertex0->p2 + vertex1->p2 + vertex2->p2) / 3);
            depth = (((vertex0->sz + vertex1->sz + vertex2->sz) / 3)
                >> KF_GTE_DEPTH_TO_OT_SHIFT) + depth_bias;
            if (depth >= KF_SCENE_MIN_OT_DEPTH) {
                render_face_submit(&face, colors, KfFaceShading::Flat, depth & KF_ORDERING_TABLE_INDEX_MASK);
            }
            break;
        }
        case (KF_TMD_MODE_F4 | KF_TMD_MODE_SEMITRANS): {
            KfTmdPrimitive *polygon = (KfTmdPrimitive *)packet;

            vertex0 = tmd_projected_vertex(polygon->f4.v0);
            vertex1 = tmd_projected_vertex(polygon->f4.v1);
            vertex2 = tmd_projected_vertex(polygon->f4.v2);
            if (render_face_winding(vertex0, vertex1, vertex2) <= 0) {
                continue;
            }
            vertex3 = tmd_projected_vertex(polygon->f4.v3);
            kf::DrawFace face {};
            CVECTOR colors[4] {};
            face.shape = kf::FaceShape::Quad;
            face.transparency = kf::FaceTransparency::Blend;
            render_face_vertex(&face, 0, vertex0);
            render_face_vertex(&face, 1, vertex1);
            render_face_vertex(&face, 2, vertex2);
            render_face_vertex(&face, 3, vertex3);
            colors[0] = kf::render_light_normal(open_graphics_runtime.render_state.lighting, *lights,
                normals[polygon->f4.n0], polygon->color, (vertex0->p2 + vertex1->p2 + vertex2->p2 + vertex3->p2) >> 2);
            depth = ((vertex0->sz + vertex1->sz + vertex2->sz + vertex3->sz)
                >> (KF_GTE_DEPTH_TO_OT_SHIFT + 2)) + depth_bias;
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

void render_tmd_reset_module_state(void)
{
    kf::restore_initial_value<tmd_textured_primitive_color>();
}
