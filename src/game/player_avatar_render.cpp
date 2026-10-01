#include <kf/platform/prelude.h>
#include <kf/game/world.h>
#include <kf/game/party_runtime.h>
#include <kf/game/graphics.h>
#include <kf/game/render.h>
#include <kf/game/avatar.h>
#include <kf/lib/math.h>
#include <kf/platform/avatars.hpp>
#include <kf/renderer/renderer.h>
#include <kf/lib/render_face.h>
#include <kf/lib/resource_file.h>
#include <algorithm>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace {
std::array<std::vector<u8>, KF_ITEM_COUNT> equipment_meshes;

void render_equipment(KfObjectId item, bool shield, const AvatarPose &pose,
    const MATRIX &body, const MATRIX &body_lights)
{
    const auto id=kf_enum_encode<unsigned>(item);
    if (!pose.rigged || (shield ? (id<26 || id>30) : id>11)) return;
    auto &bytes=equipment_meshes[id];
    if (bytes.empty()) {
        char path[32];
        std::snprintf(path,sizeof path,"ITEM%u/I%03u.TMD",id/30+1,id+1);
        kf::DataFile file{};
        const auto opened=resource_file_open(&file,path);
        if (opened!=kf::FileResult::Ok || !file.size || file.size>256*1024) {
            kf::data_file_close(&file);
            kf::host_fail("Cannot load an equipped party item model");
        }
        bytes.resize(file.size);
        const auto read=kf::data_file_read(&file,bytes.data(),bytes.size());
        kf::data_file_close(&file);
        if (read!=kf::FileResult::Ok) kf::host_fail("Cannot read an equipped party item model");
    }
    const auto attachment=avatar_equipment_transform(pose,shield);
    MATRIX model{}, lights{};
    kf::matrix_multiply_rotation(body,attachment,model);
    kf::matrix_multiply_rotation(body_lights,attachment,lights);
    const auto position=kf::render_transform_point(body,
        {static_cast<s16>(attachment.t[0]),static_cast<s16>(attachment.t[1]),static_cast<s16>(attachment.t[2])});
    model.t[0]=position.vx; model.t[1]=position.vy; model.t[2]=position.vz;
    auto context=tmd_context();
    const auto saved=context.current_tmd;
    auto *saved_vertices=context.current_vertices;
    context.current_tmd=tmd_resource_view(bytes.data(),bytes.size());
    const auto count=context.current_tmd.data->object_count;
    for (u16 object=0; object<count; ++object) {
        tmd_select_object_vertices(context,object);
        tmd_project_vertices(tmd_get_object(context,object)->vertex_count,&model,game_graphics_runtime.render_state.projection);
        render_enqueue_tmd(object,0,&lights);
    }
    context.current_tmd=saved;
    context.current_vertices=saved_vertices;
}
}

void player_avatar_render_reset_module_state()
{
    for (auto &mesh : equipment_meshes) std::vector<u8>{}.swap(mesh);
}

void render_party(WorldState &world, u8 camera_slot)
{
    if (!world.party.enabled) return;
    const auto &view = game_graphics_runtime.render_state;
    for (u8 slot = 0; slot < party_capacity; ++slot) {
        const auto &member = world.party.members[slot];
        if (slot == camera_slot || !party_member_alive(member)) continue;
        const auto *mesh = kf::avatar_mesh(member.avatar);
        if (!mesh) continue;
        const auto &player = member.player.state;
        const auto body = party_present_entity(world.presentation ? &world.presentation->players[slot] : nullptr,
            party_player_pose(member));
        AvatarPose pose;
        avatar_build_pose(member.avatar,{player.view_bob_phase,player.motion_state.forward_velocity,
            player.motion_state.strafe_velocity,player.weapon_attack_phase,body.rotation.vx,
            player.equipped_weapon_id==KF_ITEM_COLICHEMARDE,member.player.cast_pose_ticks},pose);
        const VECTOR delta {body.position.vx - view.view_position.vx,
            body.position.vy - view.view_position.vy, body.position.vz - view.view_position.vz};
        if (std::abs(delta.vx) > 16000 || std::abs(delta.vy) > 16000 || std::abs(delta.vz) > 16000) continue;
        MATRIX model{}, lights{};
        // KFIII base bodies face -Z; KF1's zero-yaw movement faces +Z.
        matrix_set_rotation_y(2048 - body.rotation.vy, &model);
        kf::matrix_multiply_rotation(render_light_matrices[KF_RENDER_LIGHT_ACTOR], model, lights);
        kf::render_place_model(model, view.view_matrix, delta.narrowed());
        kf::matrix_multiply_rotation(view.view_matrix, model, model);
        const auto texture = kf::avatar_texture(member.avatar);
        if (!texture) kf::host_fail("Cannot upload character texture");
        for (unsigned triangle = 0; triangle < mesh->info.triangles; ++triangle) {
            kf::DrawFace face{};
            face.shape = kf::FaceShape::Triangle;
            face.material.kind = kf::SurfaceKind::Texture;
            face.material.blend = kf::BlendMode::average;
            face.material.texture = texture;
            face.transparency = kf::FaceTransparency::Opaque;
            face.shading = kf::FaceShading::Gouraud;
            CVECTOR colors[4]{};
            bool visible = true;
            for (unsigned corner = 0; corner < 3; ++corner) {
                const auto &v = mesh->vertices[triangle * 3 + corner];
                SVECTOR position, normal;
                avatar_pose_vertex(pose,v,position,normal);
                const auto p = kf::render_project_point(model, view.projection, position);
                visible &= p.depth >= 100;
                face.depth += p.depth;
                const CVECTOR base{v.r, v.g, v.b, 0};
                colors[corner] = v.unlit ? kf::render_fog_color(view.lighting, base, p.fog)
                    : kf::render_light_normal(view.lighting, lights, normal, base, p.fog);
                face.vertices[corner] = {float(p.x), float(p.y), (v.u + .5f) / 256.f,
                    (v.v + .5f) / mesh->info.height,0,0,0,1};
            }
            face.depth = (face.depth / 3) >> KF_GTE_DEPTH_TO_OT_SHIFT;
            if (visible) render_face_submit(&face,colors,kf::FaceShading::Gouraud,face.depth);
        }
        render_equipment(player.equipped_weapon_id,false,pose,model,lights);
        render_equipment(player.equipped_shield_id,true,pose,model,lights);
    }
}

#ifdef __EMSCRIPTEN__
extern "C" EMSCRIPTEN_KEEPALIVE bool kf_avatar_has_rig(unsigned slot)
{
    return avatar_has_rig(slot);
}
// Upload view for the lobby's separate WebGL context. No file parsing or game
// state changes: use the same skinning as remote players, on a copied mesh.
extern "C" EMSCRIPTEN_KEEPALIVE const KfAvatarVertex *kf_avatar_preview_vertices(unsigned slot, unsigned motion, unsigned phase)
{
    static_assert(sizeof(KfAvatarVertex) == 20 && offsetof(KfAvatarVertex,nx) == 6 &&
        offsetof(KfAvatarVertex,u) == 12 && offsetof(KfAvatarVertex,r) == 16);
    static std::vector<KfAvatarVertex> vertices;
    const auto *mesh = kf::avatar_mesh(slot);
    if (!mesh) return nullptr;
    phase=std::min(phase,4096u);
    AvatarMotion preview{static_cast<u16>(phase),0,0,-1,0,false};
    switch (motion) {
    case 0: break;
    case 1: preview.forward=180; break;
    case 2: preview.strafe=180; break;
    case 3: preview.attack_phase=static_cast<s16>(phase); break;
    case 4: preview.attack_phase=static_cast<s16>(phase); preview.thrust=true; break;
    case 5: preview.cast_ticks=static_cast<u8>(avatar_cast_ticks-phase*avatar_cast_ticks/4096); break;
    default: return nullptr;
    }
    AvatarPose pose;
    avatar_build_pose(slot,preview,pose);
    vertices.assign(mesh->vertices,mesh->vertices+mesh->info.triangles*3);
    for (auto &vertex : vertices) {
        SVECTOR position, normal;
        avatar_pose_vertex(pose,vertex,position,normal);
        vertex.x=position.vx; vertex.y=position.vy; vertex.z=position.vz;
        vertex.nx=normal.vx; vertex.ny=normal.vy; vertex.nz=normal.vz;
    }
    return vertices.data();
}
#endif
