#include <kf/platform/avatars.hpp>
#include <kf/platform/assets.hpp>
#include <kf/renderer/renderer.hpp>
#include <kf/lib/fixed_math.hpp>
#include <algorithm>
#include <cstdio>
#include <vector>
#include <memory>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace kf {
static constexpr char avatar_presentation_recipe[] = "kf3-standing-pose-v13";
static KfAvatarPack *pack;
static AvatarMesh meshes[KF_AVATAR_SLOTS];
static std::vector<KfAvatarVertex> curated_vertices[KF_AVATAR_SLOTS];
static unsigned textures[KF_AVATAR_SLOTS];
static char hash[sha256_hex_capacity] = "no-avatars-v1";

static bool carried_book(const KfAvatarVertex *triangle) {
    // Slot 24's closed book is a separate 16-triangle component. Remove its
    // covers/pages so the playable body's hand can hold the equipped KF1 item.
    constexpr s16 outline[][2] {{-355,-782},{-282,-769},{-236,-1032},{-309,-1045},{-269,-1057}};
    for (unsigned corner=0; corner<3; ++corner) {
        const auto &v=triangle[corner];
        if (v.z!=-184 && v.z!=184) return false;
        bool found=false;
        for (const auto &p : outline) found |= v.x==p[0] && v.y==p[1];
        if (!found) return false;
    }
    return true;
}
static bool carried_cane(const KfAvatarVertex *triangle) {
    // Slot 14's separate 54-triangle cane; the adjacent hand lies outside this
    // complete-component bound. The equipped KF1 item takes its place.
    for (unsigned corner=0; corner<3; ++corner) {
        const auto &v=triangle[corner];
        if (v.x<-509 || v.x>-313 || v.y<-1066 || v.y>-1 || v.z<-689 || v.z>-564) return false;
    }
    return true;
}
static bool carried_tool(const KfAvatarVertex *triangle) {
    // Slot 22's separate tool has 16 positions and 28 triangles. A bounding
    // box also contains parts of the hand and coat, so use its actual outline.
    constexpr s16 outline[][3] {
        {257,-1042,-550},{272,-684,-588},{275,-999,-753},{285,-768,-778},
        {309,-889,-567},{311,-840,-572},{314,-894,-839},{316,-868,500},
        {316,-864,547},{320,-778,492},{321,-729,486},{326,-637,476},
        {326,-633,521},{340,-895,-840},{341,-865,-569},{352,-754,489},
    };
    for (unsigned corner=0; corner<3; ++corner) {
        const auto &v=triangle[corner];
        bool found=false;
        for (const auto &p : outline) found |= v.x==p[0] && v.y==p[1] && v.z==p[2];
        if (!found) return false;
    }
    return true;
}
static bool carried_pipe(const KfAvatarVertex *triangle) {
    // Slot 5's 80-triangle pipe overlaps the fist's position bounds. Its atlas
    // region distinguishes it from the two adjacent hand faces in that box.
    for (unsigned corner=0; corner<3; ++corner) {
        const auto &v=triangle[corner];
        if (v.x<-328 || v.x>-151 || v.y<-1726 || v.y>-1660 || v.z<-816 || v.z>-727 ||
            v.u>31 || v.v<64 || v.v>265) return false;
    }
    return true;
}
static bool carried_sword(const KfAvatarVertex *triangle) {
    // Slot 27's separate 74-triangle sword. The hand material shares its
    // position bounds, so exclude the skin atlas region explicitly.
    for (unsigned corner=0; corner<3; ++corner) {
        const auto &v=triangle[corner];
        if (v.x<-323 || v.x>323 || v.y<-1267 || v.y>0 || v.z<-273 || v.z>-198 ||
            v.v>=255 || (v.v>=32 && v.v<=63)) return false;
    }
    return true;
}
static void open_armored_hands(KfAvatarVertex &vertex) {
    const bool hand=vertex.y>-1440 && vertex.y<-1190 && vertex.v>=32 && vertex.v<=63 && vertex.z<-110;
    const bool forearm=vertex.y>-1410 && vertex.y<-1190 && vertex.v>=255 && vertex.v<=446 &&
        (std::abs(vertex.x)>260 || vertex.z<-130);
    if (!hand && !forearm) return;
    // The gripping fingers cross the center line. Their height/depth separates
    // the two complete hand components; only the elbow joins receive blending.
    const bool left=hand ? vertex.y<-1345 || vertex.x>90 || (vertex.x>=0 && vertex.z<-330) : vertex.x>0;
    const SVECTOR elbow{static_cast<s16>(left ? 375 : -375),-1350,-40};
    const auto weight=hand ? 4096 : std::min(std::clamp((375-std::abs(vertex.x))*4096/150,0,4096),
        std::clamp((vertex.y+1420)*4096/90,0,4096));
    MATRIX rotation{};
    matrix_set_rotation_xyz({static_cast<s16>((left ? 160 : 100)*weight/4096),0,
        static_cast<s16>((left ? -1000 : 1000)*weight/4096)},rotation);
    const auto position=matrix_apply_rotation(rotation,
        {static_cast<s16>(vertex.x-elbow.vx),static_cast<s16>(vertex.y-elbow.vy),static_cast<s16>(vertex.z-elbow.vz)});
    const auto normal=matrix_apply_rotation(rotation,{vertex.nx,vertex.ny,vertex.nz});
    vertex.x=static_cast<s16>(position.vx+elbow.vx);
    vertex.y=static_cast<s16>(position.vy+elbow.vy);
    vertex.z=static_cast<s16>(position.vz+elbow.vz);
    vertex.nx=static_cast<s16>(normal.vx);
    vertex.ny=static_cast<s16>(normal.vy);
    vertex.nz=static_cast<s16>(normal.vz);
}
static void lower_raised_forearm(KfAvatarVertex &vertex) {
    // Slot 5's forearm folds upwards in the source. Lower it around its elbow
    // before skinning. The depth blend preserves the sleeve join and excludes
    // the torso and head.
    if (vertex.x<-140 && vertex.y<-1100 && vertex.z<-580) {
        const auto weight=std::clamp((-580-vertex.z)*4096/100,0,4096);
        MATRIX rotation{};
        matrix_set_rotation_xyz({static_cast<s16>(1760*weight/4096),0,0},rotation);
        const auto position=matrix_apply_rotation(rotation,
            {static_cast<s16>(vertex.x+250),static_cast<s16>(vertex.y+1310),static_cast<s16>(vertex.z+630)});
        const auto normal=matrix_apply_rotation(rotation,{vertex.nx,vertex.ny,vertex.nz});
        vertex.x=static_cast<s16>(position.vx-250);
        vertex.y=static_cast<s16>(position.vy-1310);
        vertex.z=static_cast<s16>(position.vz-630);
        vertex.nx=static_cast<s16>(normal.vx);
        vertex.ny=static_cast<s16>(normal.vy);
        vertex.nz=static_cast<s16>(normal.vz);
    }
    // This source body is offset behind its origin. Center its standing hips
    // and feet on the player's local frame; keep the original pack unchanged.
    vertex.z=static_cast<s16>(vertex.z+450);
}
static void open_clasped_hands(KfAvatarVertex &vertex) {
    // Slot 19's separate forearms meet in front of the dress. Include their
    // sleeve joins, but keep the waist and skirt outside the correction.
    if (vertex.y<=-1290 || vertex.y>=-900 ||
        !((std::abs(vertex.x)>159 && vertex.y<-1100) || vertex.z<-130)) return;
    // The clasped fingers cross the center line; depth distinguishes the two
    // overlapping hands before they are moved to their respective sides.
    const bool left=vertex.x>45 || vertex.z<-225 ||
        (vertex.x>=0 && vertex.y>-960 && vertex.z<-205) ||
        (vertex.x>=0 && vertex.y<-1035 && vertex.z<-200);
    const s16 elbow_x=left ? 235 : -235;
    const auto weight=std::clamp((vertex.y+1290)*4096/110,0,4096);
    MATRIX rotation{};
    matrix_set_rotation_xyz({static_cast<s16>((left ? 220 : 160)*weight/4096),0,
        static_cast<s16>((left ? -440 : 440)*weight/4096)},rotation);
    const auto position=matrix_apply_rotation(rotation,
        {static_cast<s16>(vertex.x-elbow_x),static_cast<s16>(vertex.y+1240),static_cast<s16>(vertex.z+105)});
    const auto normal=matrix_apply_rotation(rotation,{vertex.nx,vertex.ny,vertex.nz});
    vertex.x=static_cast<s16>(position.vx+elbow_x);
    vertex.y=static_cast<s16>(position.vy-1240);
    vertex.z=static_cast<s16>(position.vz-105);
    vertex.nx=static_cast<s16>(normal.vx);
    vertex.ny=static_cast<s16>(normal.vy);
    vertex.nz=static_cast<s16>(normal.vz);
}
static void open_elf_forearms(KfAvatarVertex &vertex) {
    // Slot 1's crossed arms share skin/bracelet atlas regions with its torso.
    // This height/depth mask covers the forearms while excluding the long cloth.
    if (vertex.y<=-1390 || vertex.y>=-875 ||
        !((vertex.v>=190 && vertex.v<=221) || (vertex.v>=350 && vertex.v<=413)) ||
        !(std::abs(vertex.x)>175 || (std::abs(vertex.x)>130 && vertex.z<-60) ||
          (std::abs(vertex.x)>160 && vertex.y<-1250) || vertex.z<-120)) return;
    const bool left=vertex.x>=0 && !(vertex.y<-1110 && vertex.z<-160);
    const SVECTOR elbow=left ? SVECTOR{215,-1290,-40} : SVECTOR{-240,-1280,-95};
    const auto weight=left ? std::clamp((vertex.y+1390)*4096/230,0,4096) :
        std::clamp((vertex.x+240)*4096/200,0,4096);
    MATRIX rotation{};
    matrix_set_rotation_xyz({static_cast<s16>((left ? 350 : 240)*weight/4096),0,
        static_cast<s16>((left ? -250 : 850)*weight/4096)},rotation);
    const auto position=matrix_apply_rotation(rotation,
        {static_cast<s16>(vertex.x-elbow.vx),static_cast<s16>(vertex.y-elbow.vy),static_cast<s16>(vertex.z-elbow.vz)});
    const auto normal=matrix_apply_rotation(rotation,{vertex.nx,vertex.ny,vertex.nz});
    vertex.x=static_cast<s16>(position.vx+elbow.vx);
    vertex.y=static_cast<s16>(position.vy+elbow.vy);
    vertex.z=static_cast<s16>(position.vz+elbow.vz);
    vertex.nx=static_cast<s16>(normal.vx);
    vertex.ny=static_cast<s16>(normal.vy);
    vertex.nz=static_cast<s16>(normal.vz);
}
static void open_vested_hands(KfAvatarVertex &vertex) {
    // Slot 26's arm/hand atlas regions also occur on the chest ornament and
    // shoes. The forearm-height band excludes those other components.
    if (vertex.y<=-1425 || vertex.y>=-1160 || vertex.v<60 || vertex.v>155) return;
    // The source hands overlap across X=0. Their height and fingertip bounds
    // distinguish the two complete hand components before opening the elbows.
    const bool left=vertex.v<92 ? vertex.x>0 :
        vertex.y<-1255 || vertex.x>100 ||
        (vertex.x>-50 && vertex.x<0 && vertex.y<-1230) ||
        (vertex.y>-1255 && vertex.y<-1250 && vertex.x>0);
    const SVECTOR elbow=left ? SVECTOR{340,-1350,-135} : SVECTOR{-305,-1360,-160};
    const auto inward=left ? elbow.vx-vertex.x : vertex.x-elbow.vx;
    const auto weight=std::min(std::clamp(inward*4096/170,0,4096),
        std::clamp((vertex.y+1425)*4096/90,0,4096));
    MATRIX rotation{};
    matrix_set_rotation_xyz({0,0,static_cast<s16>((left ? -830 : 780)*weight/4096)},rotation);
    const auto position=matrix_apply_rotation(rotation,
        {static_cast<s16>(vertex.x-elbow.vx),static_cast<s16>(vertex.y-elbow.vy),static_cast<s16>(vertex.z-elbow.vz)});
    const auto normal=matrix_apply_rotation(rotation,{vertex.nx,vertex.ny,vertex.nz});
    vertex.x=static_cast<s16>(position.vx+elbow.vx);
    vertex.y=static_cast<s16>(position.vy+elbow.vy);
    vertex.z=static_cast<s16>(position.vz+elbow.vz);
    vertex.nx=static_cast<s16>(normal.vx);
    vertex.ny=static_cast<s16>(normal.vy);
    vertex.nz=static_cast<s16>(normal.vz);
}
static bool install(const u8 *bytes, std::size_t size) {
    auto *next = kf_avatar_open(bytes, size);
    if (!next) return false;
    avatars_release();
    pack = next;
    for (unsigned i = 0; i < kf_avatar_count(pack); ++i) {
        KfAvatarMesh mesh{};
        kf_avatar_mesh(pack, i, &mesh);
        meshes[mesh.slot] = {mesh, kf_avatar_vertices(pack, i), kf_avatar_rgba(pack, i)};
        if (mesh.slot==1 || mesh.slot==5 || mesh.slot==14 || mesh.slot==19 || mesh.slot==22 || mesh.slot==24 || mesh.slot==26 || mesh.slot==27) {
            auto &curated=curated_vertices[mesh.slot];
            const auto *vertices=meshes[mesh.slot].vertices;
            for (unsigned triangle=0; triangle<mesh.triangles; ++triangle) {
                const auto *face=vertices+triangle*3;
                const bool carried=mesh.slot==5 ? carried_pipe(face) : mesh.slot==14 ? carried_cane(face) :
                    mesh.slot==22 ? carried_tool(face) : mesh.slot==24 ? carried_book(face) : mesh.slot==27 && carried_sword(face);
                if (!carried) curated.insert(curated.end(),face,face+3);
            }
            if (mesh.slot==1) for (auto &vertex : curated) open_elf_forearms(vertex);
            if (mesh.slot==5) for (auto &vertex : curated) lower_raised_forearm(vertex);
            if (mesh.slot==19) for (auto &vertex : curated) open_clasped_hands(vertex);
            if (mesh.slot==26) for (auto &vertex : curated) open_vested_hands(vertex);
            if (mesh.slot==27) for (auto &vertex : curated) open_armored_hands(vertex);
            meshes[mesh.slot].vertices=curated.data();
            meshes[mesh.slot].info.triangles=static_cast<u32>(curated.size()/3);
        }
    }
    Sha256 digest{};
    sha256_init(&digest);
    sha256_update(&digest,reinterpret_cast<const u8 *>(avatar_presentation_recipe),sizeof avatar_presentation_recipe);
    sha256_update(&digest, bytes, size);
    sha256_finish(&digest, hash);
    return true;
}
static bool read_file(const char *path, std::vector<u8> &bytes, std::size_t limit) {
    auto *file = std::fopen(path, "rb");
    if (!file) return false;
    if (std::fseek(file, 0, SEEK_END)) { std::fclose(file); return false; }
    const auto length = std::ftell(file);
    if (length <= 0 || static_cast<std::size_t>(length) > limit || std::fseek(file, 0, SEEK_SET)) {
        std::fclose(file); return false;
    }
    bytes.resize(static_cast<std::size_t>(length));
    const bool read = std::fread(bytes.data(), 1, bytes.size(), file) == bytes.size();
    std::fclose(file);
    return read;
}
bool avatars_load(const char *path) {
    std::vector<u8> bytes;
    return read_file(path, bytes, KF_AVATAR_MAX_BYTES) && install(bytes.data(), bytes.size());
}
using ImportOwner = std::unique_ptr<KfAvatarImport, decltype(&kf_avatar_import_close)>;
bool avatars_import_disc(const char *path) {
    auto *file = std::fopen(path, "rb");
    if (!file) return false;
    if (std::fseek(file, 0, SEEK_END)) { std::fclose(file); return false; }
    const auto size = std::ftell(file);
    if (size < 0 || size > 800*1024*1024) { std::fclose(file); return false; }
    ImportOwner importer(kf_avatar_import_open(static_cast<u32>(size)), kf_avatar_import_close);
    KfAvatarRead request{};
    std::vector<u8> bytes;
    bool good = true;
    int state;
    while ((state = kf_avatar_import_request(importer.get(), &request)) == 1) {
        bytes.resize(request.length);
        if (std::fseek(file, request.offset, SEEK_SET) || std::fread(bytes.data(), 1, bytes.size(), file) != bytes.size() ||
            !kf_avatar_import_supply(importer.get(), bytes.data(), bytes.size())) { good = false; break; }
    }
    std::fclose(file);
    u32 length = 0;
    const auto *result = kf_avatar_import_result(importer.get(), &length);
    if (!good || state != 0 || !result) {
        u8 message[256]{};
        kf_avatar_import_error(importer.get(), message, sizeof message);
        std::fprintf(stderr, "Character import: %s\n", message);
        return false;
    }
    return install(result, length);
}
void avatars_release() {
    for (auto &texture : textures) {
        if (texture) renderer_delete_texture(texture);
        texture = 0;
    }
    for (auto &mesh : meshes) mesh = {};
    for (auto &vertices : curated_vertices) std::vector<KfAvatarVertex>{}.swap(vertices);
    kf_avatar_close(pack);
    pack = nullptr;
    std::snprintf(hash, sizeof hash, "%s", "no-avatars-v1");
}
const AvatarMesh *avatar_mesh(unsigned slot) {
    return slot < KF_AVATAR_SLOTS && meshes[slot].info.triangles ? &meshes[slot] : nullptr;
}
unsigned avatar_texture(unsigned slot) {
    const auto *mesh = avatar_mesh(slot);
    if (!mesh) return 0;
    if (!textures[slot]) {
        const auto size = static_cast<std::size_t>(256 * mesh->info.height * 4);
        const Image image{256, mesh->info.height, {const_cast<u8 *>(mesh->rgba), size, size}};
        textures[slot] = renderer_upload(&image);
    }
    return textures[slot];
}
const char *avatars_hash() { return hash; }
}

#ifdef __EMSCRIPTEN__
static kf::ImportOwner browser_import(nullptr, kf_avatar_import_close);
extern "C" EMSCRIPTEN_KEEPALIVE int kf_avatar_disc_start(unsigned size) {
    browser_import.reset(kf_avatar_import_open(size));
    return browser_import != nullptr;
}
extern "C" EMSCRIPTEN_KEEPALIVE int kf_avatar_disc_read() {
    KfAvatarRead read{};
    const auto state = kf_avatar_import_request(browser_import.get(), &read);
    return state == 1 ? static_cast<int>(read.length) : state;
}
extern "C" EMSCRIPTEN_KEEPALIVE unsigned kf_avatar_disc_offset() {
    KfAvatarRead read{};
    kf_avatar_import_request(browser_import.get(), &read);
    return read.offset;
}
extern "C" EMSCRIPTEN_KEEPALIVE int kf_avatar_disc_supply(const char *path) {
    std::vector<u8> bytes;
    return kf::read_file(path, bytes, 40*1024*1024) && kf_avatar_import_supply(browser_import.get(), bytes.data(), bytes.size());
}
extern "C" EMSCRIPTEN_KEEPALIVE const char *kf_avatar_disc_error() {
    static u8 message[256];
    kf_avatar_import_error(browser_import.get(), message, sizeof message);
    return reinterpret_cast<const char *>(message);
}
extern "C" EMSCRIPTEN_KEEPALIVE int kf_avatar_disc_finish(const char *path) {
    u32 length = 0;
    const auto *bytes = kf_avatar_import_result(browser_import.get(), &length);
    if (!bytes) return false;
    auto *file = std::fopen(path, "wb");
    if (!file) return false;
    const bool written = std::fwrite(bytes, 1, length, file) == length;
    const bool closed = std::fclose(file) == 0;
    return written && closed;
}
extern "C" EMSCRIPTEN_KEEPALIVE void kf_avatar_disc_cancel() { browser_import.reset(); }
extern "C" EMSCRIPTEN_KEEPALIVE int kf_load_avatars(const char *path) { return kf::avatars_load(path); }
extern "C" EMSCRIPTEN_KEEPALIVE int kf_has_avatar(int slot) { return kf::avatar_mesh(slot) != nullptr; }
extern "C" EMSCRIPTEN_KEEPALIVE unsigned kf_avatar_preview_count(unsigned slot) {
    const auto *mesh = kf::avatar_mesh(slot);
    return mesh ? mesh->info.triangles*3 : 0;
}
extern "C" EMSCRIPTEN_KEEPALIVE unsigned kf_avatar_preview_height(unsigned slot) {
    const auto *mesh = kf::avatar_mesh(slot);
    return mesh ? mesh->info.height : 0;
}
extern "C" EMSCRIPTEN_KEEPALIVE const uint8_t *kf_avatar_preview_rgba(unsigned slot) {
    const auto *mesh = kf::avatar_mesh(slot);
    return mesh ? mesh->rgba : nullptr;
}
#endif
