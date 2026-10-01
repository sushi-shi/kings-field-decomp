#include "avatar_decode.h"
#include <algorithm>
#include <memory>

struct KfAvatarPack {
    struct Mesh { KfAvatarMesh info; std::vector<KfAvatarVertex> vertices; std::vector<u8> rgba; };
    std::vector<Mesh> meshes;
};
KfAvatarPack *kf_avatar_open(const u8 *bytes, std::size_t length)
{
    using namespace kf::avatar;
    if (!bytes || length > KF_AVATAR_MAX_BYTES) return nullptr;
    try {
        Reader r({bytes, length});
        require(r.u32_le() == 0x3141464b && r.u16_le() == 1, "Invalid character pack header");
        auto count = r.u16_le(); require(count > 0 && count <= KF_AVATAR_SLOTS, "Invalid character count");
        std::array<bool, KF_AVATAR_SLOTS> seen {};
        auto pack = std::make_unique<KfAvatarPack>();
        for (unsigned i = 0; i < count; ++i) {
            const auto slot = r.u16_le(), height = r.u16_le(); const auto triangles = r.u32_le();
            require(slot < KF_AVATAR_SLOTS && !seen[slot] && height >= 1 && height <= 4096 && triangles >= 1 && triangles <= 4096,
                "Invalid character mesh extent"); seen[slot] = true;
            KfAvatarPack::Mesh mesh {{triangles, slot, height}, {}, {}};
            Reader vertices(r.take(triangles * 60)); mesh.vertices.reserve(triangles * 3);
            for (unsigned v = 0; v < triangles * 3; ++v) {
                KfAvatarVertex vertex {vertices.s16_le(), vertices.s16_le(), vertices.s16_le(), vertices.s16_le(), vertices.s16_le(), vertices.s16_le(),
                    vertices.u16_le(), vertices.u16_le(), vertices.byte(), vertices.byte(), vertices.byte(), vertices.byte()};
                for (auto p : {vertex.x, vertex.y, vertex.z}) require(p >= -8192 && p <= 8192, "Invalid character position");
                for (auto n : {vertex.nx, vertex.ny, vertex.nz}) require(n >= -4096 && n <= 4096, "Invalid character normal");
                require(vertex.u < 256 && vertex.v < height && vertex.unlit <= 1, "Invalid character material"); mesh.vertices.push_back(vertex);
            }
            auto rgba = r.take(256 * height * 4); mesh.rgba.assign(rgba.begin(), rgba.end()); pack->meshes.push_back(std::move(mesh));
        }
        require(!r.remaining(), "Trailing character pack bytes"); return pack.release();
    } catch (const kf::codec::Error &) { return nullptr; }
}
void kf_avatar_close(KfAvatarPack *pack) { delete pack; }
u32 kf_avatar_count(const KfAvatarPack *pack) { return pack ? pack->meshes.size() : 0; }
int kf_avatar_mesh(const KfAvatarPack *pack, u32 index, KfAvatarMesh *mesh)
{
    if (!pack || !mesh || index >= pack->meshes.size()) return 0;
    *mesh = pack->meshes[index].info; return 1;
}
const KfAvatarVertex *kf_avatar_vertices(const KfAvatarPack *pack, u32 index)
{ return pack && index < pack->meshes.size() ? pack->meshes[index].vertices.data() : nullptr; }
const u8 *kf_avatar_rgba(const KfAvatarPack *pack, u32 index)
{ return pack && index < pack->meshes.size() ? pack->meshes[index].rgba.data() : nullptr; }
