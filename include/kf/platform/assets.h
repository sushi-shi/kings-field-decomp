#ifndef KF_PLATFORM_ASSETS_H
#define KF_PLATFORM_ASSETS_H

#include <kf/lib/codec.h>
#include <kf/lib/types.h>
#include <kf/platform/language.h>
#include <array>
#include <cstddef>
#include <span>

namespace kf {
inline constexpr std::size_t asset_path_capacity = 128;
inline constexpr std::size_t sha256_state_words = 8;
inline constexpr std::size_t sha256_block_bytes = 64;
inline constexpr std::size_t sha256_hex_capacity = 65;
inline constexpr std::size_t disc_directory_capacity = 128;
inline constexpr std::size_t disc_file_capacity = 4096;
inline constexpr std::size_t disc_status_capacity = 256;
inline constexpr std::size_t retail_resource_file_count = 428;
struct ByteBuffer {
    u8 *data;
    std::size_t size, capacity;
};
bool buffer_resize(ByteBuffer *buffer, std::size_t size);
void buffer_release(ByteBuffer *buffer);

struct Image {
    u32 width, height;
    ByteBuffer rgba;
};
bool image_decode_tim(Image *image, const u8 *data, std::size_t size, std::size_t offset = 0,
                      u32 palette = 0);
void image_release(Image *image);
bool asset_path(std::span<char> output, const char *input);

struct Sha256 {
    std::array<u32, sha256_state_words> state;
    std::array<u8, sha256_block_bytes> pending;
    std::uint64_t length;
    std::size_t used;
};
void sha256_init(Sha256 *hash);
void sha256_update(Sha256 *hash, std::span<const u8> bytes);
void sha256_finish(const Sha256 *hash, std::span<char, sha256_hex_capacity> output);

struct Asset {
    std::array<char, asset_path_capacity> path;
    ByteBuffer bytes;
};
struct AssetTable {
    Asset *entries;
    std::size_t count, capacity;
};
bool assets_append(AssetTable *table, const char *path, ByteBuffer *bytes);
const Asset *assets_find(const AssetTable *table, const char *path);
void assets_release(AssetTable *table);

inline constexpr std::size_t disc_import_limit = 128 * 1024 * 1024;

enum class ImportState : u8 { idle, layout, volume, directory, file, complete, failed };
struct ReadRequest {
    std::uint64_t offset;
    u32 length;
};
struct DiscExtent {
    std::array<char, asset_path_capacity> path;
    u32 sector, length;
};
struct DiscImporter {
    ImportState state;
    Language language;
    ReadRequest request;
    std::uint64_t disc_size;
    u32 sector_size, volume_sectors;
    std::size_t file_bytes;
    std::array<char, disc_status_capacity> message;
    std::array<DiscExtent, disc_directory_capacity> directories;
    std::size_t directory_count;
    std::array<u32, disc_directory_capacity> visited_directories;
    std::size_t visited_count;
    std::array<DiscExtent, disc_file_capacity> files;
    std::size_t file_count, file_index;
    DiscExtent current;
    AssetTable assets;
};
// Sorted paths: LE32 path length, LE32 file length, path bytes, then file bytes.
// Identity metadata only; extracted resources remain ordinary, unchanged files.
inline constexpr const char *retail_files_sha256 =
    "450b9f09ca34bedc1c8bc150e01b79bfe108c700dad2b2783246b926953deee2";
inline constexpr const char *english_v1_files_sha256 =
    "697b2b80d13a3e6e54b2d72f90a49e29ae6a31d6d45970b40aee908b94208595";
const char *assets_language_hash(Language language);
bool assets_match_language(AssetTable *table, Language language);
void disc_import_start(DiscImporter *importer, std::uint64_t disc_size, Language language);
bool disc_import_waiting(const DiscImporter *importer);
void disc_import_supply(DiscImporter *importer, const u8 *bytes, std::size_t size);
void disc_import_fail(DiscImporter *importer, const char *message);
double disc_import_progress(const DiscImporter *importer);
void disc_import_release(DiscImporter *importer);
} // namespace kf

#endif // KF_PLATFORM_ASSETS_H
