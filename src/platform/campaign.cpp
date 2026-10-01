#include <kf/platform/campaign.hpp>
#include <kf/platform/assets.hpp>
#include <kf/platform/avatars.hpp>
#include <kf/net/wire.hpp>
#include <cstring>
#include <cstdio>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace kf {
namespace {
constexpr std::size_t header_bytes = 134;
void compatibility(std::string_view resources, std::string_view recipe, char (&output)[sha256_hex_capacity])
{
    Sha256 hash;
    sha256_init(&hash);
    const u8 zero = 0;
    for (auto text : {resources, recipe}) {
        sha256_update(&hash, reinterpret_cast<const u8 *>(text.data()), text.size());
        sha256_update(&hash, &zero, 1);
    }
    sha256_finish(&hash, output);
}
void checksum(std::span<const u8> bytes, char (&output)[sha256_hex_capacity])
{
    Sha256 hash;
    sha256_init(&hash);
    sha256_update(&hash, bytes.data(), bytes.size());
    sha256_finish(&hash, output);
}
}
std::vector<u8> campaign_file_pack(std::span<const u8> snapshot, std::string_view resources, std::string_view recipe)
{
    if (snapshot.empty() || snapshot.size() > campaign_file_capacity - header_bytes) return {};
    std::vector<u8> bytes(header_bytes);
    net::WireCodec io {std::span<u8>(bytes)};
    u32 magic = 0x3143464b;
    u16 version = 1;
    io.value(magic); io.value(version);
    char compatible[sha256_hex_capacity], digest[sha256_hex_capacity];
    compatibility(resources, recipe, compatible);
    checksum(snapshot, digest);
    std::memcpy(bytes.data() + 6, compatible, 64);
    std::memcpy(bytes.data() + 70, digest, 64);
    bytes.insert(bytes.end(), snapshot.begin(), snapshot.end());
    return bytes;
}
bool campaign_file_unpack(std::span<const u8> file, std::string_view resources, std::string_view recipe,
    std::vector<u8> &snapshot)
{
    if (file.size() <= header_bytes || file.size() > campaign_file_capacity) return false;
    net::WireCodec io {file};
    u32 magic = 0;
    u16 version = 0;
    io.value(magic); io.value(version);
    if (!io.valid() || magic != 0x3143464b || version != 1) return false;
    char compatible[sha256_hex_capacity], digest[sha256_hex_capacity];
    compatibility(resources, recipe, compatible);
    const auto payload = file.subspan(header_bytes);
    checksum(payload, digest);
    if (std::memcmp(compatible, file.data() + 6, 64) || std::memcmp(digest, file.data() + 70, 64)) return false;
    KfNetWorldHeader header {};
    if (kf_net_world_info(payload.data(), payload.size(), &header) != KF_CODEC_OK || !header.full) return false;
    snapshot.assign(payload.begin(), payload.end());
    return true;
}
SaveFileResult campaign_file_summary(SaveSlot slot, std::string_view resources, std::string_view recipe,
    KfNetWorldSummary &summary)
{
    std::vector<u8> file(campaign_file_capacity), snapshot;
    std::size_t size = 0;
    const auto result = campaign_file_read(slot, file.data(), file.size(), &size);
    if (result != SaveFileResult::Ok) return result;
    if (!campaign_file_unpack(std::span<const u8>(file).first(size), resources, recipe, snapshot) ||
        kf_net_world_summary(snapshot.data(), snapshot.size(), &summary) != KF_CODEC_OK)
        return SaveFileResult::Invalid;
    return SaveFileResult::Ok;
}
}

#ifdef __EMSCRIPTEN__
extern "C" EMSCRIPTEN_KEEPALIVE const char *kf_campaign_preview(const char *path)
{
    auto *file = std::fopen(path, "rb");
    if (!file) return "";
    std::vector<u8> bytes(kf::campaign_file_capacity + 1), snapshot;
    const auto size = std::fread(bytes.data(), 1, bytes.size(), file);
    const bool good = !std::ferror(file);
    std::fclose(file);
    KfNetWorldSummary summary {};
    if (!good || !kf::campaign_file_unpack(std::span<const u8>(bytes).first(size), kf::retail_files_sha256,
        kf::avatars_hash(), snapshot) || kf_net_world_summary(snapshot.data(), snapshot.size(), &summary) != KF_CODEC_OK)
        return "";
    char owner[65];
    for (unsigned i = 0; i < 32; ++i) std::snprintf(owner + i*2, 3, "%02x", summary.owner[i]);
    static char result[192];
    std::snprintf(result, sizeof result, "{\"floor\":%u,\"level\":%u,\"owner\":\"%s\"}", summary.floor, summary.level, owner);
    return result;
}
#endif
