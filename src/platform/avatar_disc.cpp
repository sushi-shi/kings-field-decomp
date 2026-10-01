#include "avatar_decode.h"

#include <algorithm>
#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>

using namespace kf::avatar;
namespace {
constexpr std::size_t sector_size = 2048;
constexpr std::array<std::string_view, 4> archive_paths {"CD/COM/MO.T", "CD/COM/RTIM.T", "CD/COM/FDAT.T", "CD/COM/MOF.T"};
u32 both32(Bytes bytes, std::size_t offset)
{
    Reader r(slice(bytes, offset, 8)); auto value = r.u32_le();
    require(value == r.u32_be(), "ISO byte-order copies disagree"); return value;
}
bool equal(Bytes bytes, std::string_view text)
{ return bytes.size() == text.size() && std::equal(bytes.begin(), bytes.end(), reinterpret_cast<const u8 *>(text.data())); }
struct Extent { std::string path; u32 sector {}; std::size_t size {}; };
enum class Stage { Iso, Bin, Directory, Archive, Complete, Failed };
}
struct KfAvatarImport {
    u32 size, volume_sectors = 0;
    std::size_t stride = sector_size, payload = 0, total_bytes = 0;
    Stage stage = Stage::Iso;
    Extent current {{}, 16, sector_size};
    std::deque<Extent> directories;
    std::set<u32> visited;
    std::map<std::string, Extent> files;
    std::vector<std::vector<u8>> archives;
    std::vector<u8> output;
    std::string error;
    explicit KfAvatarImport(u32 bytes) : size(bytes) {}
    KfAvatarRead request() const {
        require(stage != Stage::Complete && stage != Stage::Failed, "Character import is not reading");
        require(current.size && current.size <= KF_AVATAR_MAX_BYTES, "Invalid disc read size");
        const auto sectors = (current.size + sector_size - 1) / sector_size;
        require(!volume_sectors || std::uint64_t(current.sector) + sectors <= volume_sectors, "Disc extent exceeds its volume");
        const auto offset = std::uint64_t(current.sector) * stride + payload;
        const auto length = (sectors - 1) * stride + (current.size - (sectors - 1) * sector_size);
        require(offset + length <= size, "Disc extent exceeds its data track");
        return {static_cast<u32>(offset), static_cast<u32>(length)};
    }
    void queue_directory(Extent extent) {
        require(extent.size && extent.size <= 1024 * 1024 && std::count(extent.path.begin(), extent.path.end(), '/') <= 8 &&
            visited.size() < 128 && visited.insert(extent.sector).second, "Cyclic or oversized ISO directory");
        directories.push_back(std::move(extent));
    }
    void advance() {
        if (!directories.empty()) { current = std::move(directories.front()); directories.pop_front(); stage = Stage::Directory; }
        else if (archives.size() < archive_paths.size()) {
            auto found = files.find(std::string(archive_paths[archives.size()]));
            require(found != files.end(), "KFIII character archive is missing"); current = found->second; stage = Stage::Archive;
        } else {
            output = convert(archives[0], archives[1], archives[2], archives[3]); archives.clear(); stage = Stage::Complete;
        }
    }
    void volume(Bytes bytes) {
        require(equal(slice(bytes, 0, 7), std::string_view("\1CD001\1", 7)) && equal(slice(bytes, 40, 32), "SLUS-00255                      "),
            "Select US King's Field II (SLUS-00255), the western KFIII release");
        volume_sectors = both32(bytes, 80);
        require(volume_sectors > 16 && std::uint64_t(volume_sectors) * stride <= size && u16_at(bytes, 128) == 2048 &&
            Reader(slice(bytes, 130, 2)).u16_be() == 2048, "Invalid ISO volume size or block size");
        auto root = slice(bytes, 156, 34);
        require(root[0] >= 34 && root[1] == 0 && root[25] == 2 && root[26] == 0 && root[27] == 0, "Invalid ISO root directory");
        queue_directory({{}, both32(root, 2), both32(root, 10)}); advance();
    }
    void directory(Bytes bytes) {
        std::size_t at = 0;
        while (at < bytes.size()) {
            const auto length = bytes[at];
            if (!length) { at = (at / sector_size + 1) * sector_size; continue; }
            require(length >= 34 && at % sector_size + length <= sector_size, "Invalid ISO directory record");
            auto row = slice(bytes, at, length); at += length;
            require(row[1] == 0 && !(row[25] & ~3) && row[26] == 0 && row[27] == 0, "Unsupported ISO file extent");
            auto raw = slice(row, 33, row[32]); if (raw.size() == 1 && raw[0] <= 1) continue;
            std::string name(raw.begin(), raw.end()); if (name.ends_with(";1")) name.resize(name.size() - 2);
            require(!name.empty() && name != "." && name != ".." && std::all_of(name.begin(), name.end(), [](char c) {
                return (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.';
            }), "Invalid ISO file name");
            auto path = current.path.empty() ? name : current.path + '/' + name;
            Extent extent {path, both32(row, 2), both32(row, 10)};
            if (row[25] & 2) { if (path == "CD" || path == "CD/COM") queue_directory(std::move(extent)); }
            else if (std::find(archive_paths.begin(), archive_paths.end(), path) != archive_paths.end()) {
                require(extent.size && extent.size <= KF_AVATAR_MAX_BYTES - total_bytes && !files.contains(path), "Duplicate or oversized character archive");
                total_bytes += extent.size; files.emplace(path, std::move(extent));
            }
        }
        advance();
    }
    void supply(Bytes bytes) {
        require(bytes.size() == request().length, "Incomplete disc read");
        std::vector<u8> data; data.reserve(current.size);
        for (std::size_t at = 0; at < bytes.size(); at += stride) {
            auto chunk = slice(bytes, at, std::min(sector_size, bytes.size() - at)); data.insert(data.end(), chunk.begin(), chunk.end());
        }
        if (stage == Stage::Iso && (data.size() < 7 || !equal(Bytes(data).first(7), std::string_view("\1CD001\1", 7)))) {
            stage = Stage::Bin; stride = 2352; payload = 24;
        } else if (stage == Stage::Iso || stage == Stage::Bin) volume(data);
        else if (stage == Stage::Directory) directory(data);
        else if (stage == Stage::Archive) { archives.push_back(std::move(data)); advance(); }
        else require(false, "Invalid character import state");
        if (stage != Stage::Complete) request();
    }
};
KfAvatarImport *kf_avatar_import_open(u32 disc_size)
{
    if (disc_size < 17 * 2352 || disc_size > 800 * 1024 * 1024) return nullptr;
    return new KfAvatarImport(disc_size);
}
void kf_avatar_import_close(KfAvatarImport *importer) { delete importer; }
int kf_avatar_import_request(const KfAvatarImport *importer, KfAvatarRead *read)
{
    if (!importer || !read || importer->stage == Stage::Failed) return -1;
    if (importer->stage == Stage::Complete) return 0;
    try { *read = importer->request(); return 1; } catch (const kf::codec::Error &) { return -1; }
}
int kf_avatar_import_supply(KfAvatarImport *importer, const u8 *bytes, std::size_t length)
{
    if (!importer) return 0;
    try { require(bytes || !length, "Missing disc bytes"); importer->supply({bytes, length}); return 1; }
    catch (const kf::codec::Error &error) { importer->stage = Stage::Failed; importer->error = error.message; return 0; }
}
const u8 *kf_avatar_import_result(const KfAvatarImport *importer, u32 *length)
{
    if (!importer || !length || importer->stage != Stage::Complete) return nullptr;
    *length = importer->output.size(); return importer->output.data();
}
void kf_avatar_import_error(const KfAvatarImport *importer, u8 *message, std::size_t capacity)
{
    if (!message || !capacity) return;
    std::string_view error = importer ? std::string_view(importer->error) : std::string_view("Invalid character importer");
    auto count = std::min(capacity - 1, error.size()); std::copy_n(error.begin(), count, message); message[count] = 0;
}
