#include <kf/platform/assets.hpp>
#include <bit>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

namespace kf {
bool buffer_resize(ByteBuffer *buffer, std::size_t size) {
    if (size > buffer->capacity) {
        void *allocation = std::realloc(buffer->data, size);
        if (!allocation)
            return false;
        buffer->data = static_cast<u8 *>(allocation);
        buffer->capacity = size;
    }
    buffer->size = size;
    return true;
}
void buffer_release(ByteBuffer *buffer) {
    std::free(buffer->data);
    *buffer = {};
}
void image_release(Image *image) {
    buffer_release(&image->rgba);
    *image = {};
}
bool image_decode_tim(Image *image, const u8 *data, std::size_t size, std::size_t offset,
                      u32 palette) {
    KfTimInfo info{};
    if (kf_tim_info(data, size, offset, &info) != KF_CODEC_OK)
        return false;
    if (info.width > 4096 || info.height > 4096)
        return false;
    Image decoded{info.width, info.height, {}};
    if (!buffer_resize(&decoded.rgba, static_cast<std::size_t>(info.width) * info.height * 4))
        return false;
    if (kf_tim_rgba(data, size, offset, palette, decoded.rgba.data, decoded.rgba.size) !=
        KF_CODEC_OK) {
        image_release(&decoded);
        return false;
    }
    image_release(image);
    *image = decoded;
    return true;
}
bool asset_path(char *output, std::size_t capacity, const char *input) {
    std::size_t size = 0;
    const char prefix[] = "CDROM:";
    std::size_t matched = 0;
    while (matched < 6 && input[matched]) {
        char c = input[matched];
        if (c >= 'a' && c <= 'z')
            c -= 'a' - 'A';
        if (c != prefix[matched])
            break;
        ++matched;
    }
    if (matched == 6)
        input += 6;
    while (*input == '/' || *input == '\\')
        ++input;
    while (*input && *input != ';') {
        if (size + 1 >= capacity)
            return false;
        char c = *input++;
        if (c == '\\')
            c = '/';
        if (c >= 'a' && c <= 'z')
            c -= 'a' - 'A';
        if (static_cast<unsigned char>(c) < 32 || static_cast<unsigned char>(c) >= 127 || c == ':')
            return false;
        output[size++] = c;
    }
    if (!size || !capacity)
        return false;
    output[size] = 0;
    const char *segment = output;
    for (const char *at = output;; ++at) {
        if (*at && *at != '/')
            continue;
        const auto length = at - segment;
        if (!length || (length == 1 && *segment == '.') ||
            (length == 2 && segment[0] == '.' && segment[1] == '.'))
            return false;
        if (!*at)
            break;
        segment = at + 1;
    }
    return true;
}
bool assets_append(AssetTable *table, const char *path, ByteBuffer *bytes) {
    char normalized[128];
    if (!asset_path(normalized, sizeof normalized, path) || assets_find(table, normalized))
        return false;
    if (table->count == table->capacity) {
        const auto capacity = table->capacity ? table->capacity * 2 : 64;
        if (capacity > std::numeric_limits<std::size_t>::max() / sizeof(Asset))
            return false;
        auto *entries =
            static_cast<Asset *>(std::realloc(table->entries, capacity * sizeof(Asset)));
        if (!entries)
            return false;
        table->entries = entries;
        table->capacity = capacity;
    }
    auto *asset = &table->entries[table->count++];
    std::snprintf(asset->path, sizeof asset->path, "%s", normalized);
    asset->bytes = *bytes;
    *bytes = {};
    return true;
}
const Asset *assets_find(const AssetTable *table, const char *path) {
    char normalized[128];
    if (!asset_path(normalized, sizeof normalized, path))
        return nullptr;
    for (std::size_t i = 0; i < table->count; ++i)
        if (std::strcmp(table->entries[i].path, normalized) == 0)
            return &table->entries[i];
    return nullptr;
}
void assets_release(AssetTable *table) {
    for (std::size_t i = 0; i < table->count; ++i)
        buffer_release(&table->entries[i].bytes);
    std::free(table->entries);
    *table = {};
}

static constexpr u32 sha_constants[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
void sha256_init(Sha256 *hash) {
    *hash = {{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab,
              0x5be0cd19},
             {},
             0,
             0};
}
static void sha256_block(Sha256 *hash, const u8 *data) {
    u32 words[64];
    for (unsigned i = 0; i < 16; ++i)
        words[i] = (u32(data[i * 4]) << 24) | (u32(data[i * 4 + 1]) << 16) |
                   (u32(data[i * 4 + 2]) << 8) | data[i * 4 + 3];
    for (unsigned i = 16; i < 64; ++i) {
        const auto a = words[i - 15], b = words[i - 2];
        words[i] = words[i - 16] + (std::rotr(a, 7) ^ std::rotr(a, 18) ^ (a >> 3)) + words[i - 7] +
                   (std::rotr(b, 17) ^ std::rotr(b, 19) ^ (b >> 10));
    }
    u32 a = hash->state[0], b = hash->state[1], c = hash->state[2], d = hash->state[3];
    u32 e = hash->state[4], f = hash->state[5], g = hash->state[6], h = hash->state[7];
    for (unsigned i = 0; i < 64; ++i) {
        const auto t1 = h + (std::rotr(e, 6) ^ std::rotr(e, 11) ^ std::rotr(e, 25)) +
                        ((e & f) ^ (~e & g)) + sha_constants[i] + words[i];
        const auto t2 =
            (std::rotr(a, 2) ^ std::rotr(a, 13) ^ std::rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    const u32 result[8] = {a, b, c, d, e, f, g, h};
    for (unsigned i = 0; i < 8; ++i)
        hash->state[i] += result[i];
}
void sha256_update(Sha256 *hash, const u8 *bytes, std::size_t size) {
    hash->length += size;
    while (size) {
        auto count = sizeof hash->pending - hash->used;
        if (count > size)
            count = size;
        std::memcpy(hash->pending + hash->used, bytes, count);
        hash->used += count;
        bytes += count;
        size -= count;
        if (hash->used == sizeof hash->pending) {
            sha256_block(hash, hash->pending);
            hash->used = 0;
        }
    }
}
void sha256_finish(const Sha256 *input, char output[65]) {
    auto hash = *input;
    const auto bits = hash.length * 8;
    const u8 marker = 0x80, zero = 0;
    sha256_update(&hash, &marker, 1);
    while (hash.used != 56)
        sha256_update(&hash, &zero, 1);
    u8 length[8];
    for (unsigned i = 0; i < 8; ++i)
        length[i] = static_cast<u8>(bits >> (56 - 8 * i));
    sha256_update(&hash, length, sizeof length);
    for (unsigned i = 0; i < 8; ++i)
        std::snprintf(output + i * 8, 9, "%08x", hash.state[i]);
}

static constexpr u32 payload_size = 2048;
static u32 le32(const u8 *p) {
    return u32(p[0]) | (u32(p[1]) << 8) | (u32(p[2]) << 16) | (u32(p[3]) << 24);
}
static u32 be32(const u8 *p) {
    return (u32(p[0]) << 24) | (u32(p[1]) << 16) | (u32(p[2]) << 8) | u32(p[3]);
}
bool assets_match_retail(AssetTable *table) {
    if (table->count != 428)
        return false;
    std::sort(table->entries, table->entries + table->count,
              [](const Asset &a, const Asset &b) { return std::strcmp(a.path, b.path) < 0; });
    Sha256 hash{};
    sha256_init(&hash);
    std::size_t total = 0;
    for (std::size_t i = 0; i < table->count; ++i) {
        const auto &asset = table->entries[i];
        const auto path_size = std::strlen(asset.path);
        if (asset.bytes.size > disc_import_limit - total)
            return false;
        total += asset.bytes.size;
        u8 sizes[8];
        for (unsigned byte = 0; byte < 4; ++byte) {
            sizes[byte] = static_cast<u8>(path_size >> (byte * 8));
            sizes[byte + 4] = static_cast<u8>(asset.bytes.size >> (byte * 8));
        }
        sha256_update(&hash, sizes, sizeof sizes);
        sha256_update(&hash, reinterpret_cast<const u8 *>(asset.path), path_size);
        sha256_update(&hash, asset.bytes.data, asset.bytes.size);
    }
    char digest[65];
    sha256_finish(&hash, digest);
    return std::strcmp(digest, retail_files_sha256) == 0;
}
static void read_extent(DiscImporter *importer, const DiscExtent *file) {
    const auto sectors = (std::uint64_t(file->length) + payload_size - 1) / payload_size;
    const auto offset = std::uint64_t(file->sector) * importer->sector_size;
    const auto size = sectors * importer->sector_size;
    if (file->length > 16 * 1024 * 1024 || offset > importer->disc_size ||
        size > importer->disc_size - offset ||
        (importer->volume_sectors && (file->sector > importer->volume_sectors ||
         sectors > importer->volume_sectors - file->sector))) {
        disc_import_fail(importer, "Invalid or oversized disc extent.");
        return;
    }
    importer->current = *file;
    importer->request = {offset, static_cast<u32>(size)};
}
static void next_file(DiscImporter *importer) {
    if (importer->file_index == importer->file_count) {
        if (!assets_match_retail(&importer->assets)) {
            disc_import_fail(importer, "Extracted files do not match Japanese SLPS-00017.");
            return;
        }
        importer->state = ImportState::complete;
        importer->request = {};
        std::snprintf(importer->message, sizeof importer->message, "Imported %zu resource files.",
                      importer->assets.count);
        return;
    }
    importer->state = ImportState::file;
    read_extent(importer, &importer->files[importer->file_index++]);
}
static void next_directory(DiscImporter *importer) {
    if (!importer->directory_count) {
        next_file(importer);
        return;
    }
    importer->state = ImportState::directory;
    const auto directory = importer->directories[--importer->directory_count];
    read_extent(importer, &directory);
}
void disc_import_fail(DiscImporter *importer, const char *message) {
    std::snprintf(importer->message, sizeof importer->message, "%s", message);
    importer->state = ImportState::failed;
    importer->request = {};
    assets_release(&importer->assets);
}
void disc_import_start(DiscImporter *importer, std::uint64_t disc_size) {
    disc_import_release(importer);
    if (disc_size < 17 * payload_size || disc_size > disc_import_limit) {
        disc_import_fail(importer, "Expected a Japanese SLPS-00017 ISO or BIN image (up to 128 MiB).");
        return;
    }
    importer->disc_size = disc_size;
    importer->state = ImportState::layout;
    importer->request = {0, 2352};
    std::snprintf(importer->message, sizeof importer->message,
                  "Verifying Japanese SLPS-00017 disc...");
}
bool disc_import_waiting(const DiscImporter *importer) {
    return importer->state >= ImportState::layout && importer->state <= ImportState::file;
}
static bool unpack_payload(DiscImporter *importer, const u8 *raw, std::size_t size,
                           ByteBuffer *output) {
    const auto length = importer->current.length;
    if (!buffer_resize(output, length))
        return false;
    for (std::size_t at = 0, sector = 0; at < length; at += payload_size, sector += importer->sector_size) {
        auto count = length - at;
        if (count > payload_size)
            count = payload_size;
        const auto header = importer->sector_size == 2352 ? 24u : 0u;
        if (sector + header + count > size)
            return false;
        if (header && (raw[sector + 15] != 2 || (raw[sector + 18] & 0x20) ||
                       std::memcmp(raw + sector + 16, raw + sector + 20, 4) != 0))
            return false;
        std::memcpy(output->data + at, raw + sector + header, count);
    }
    return true;
}
static bool parse_directory(DiscImporter *importer, const ByteBuffer *bytes) {
    for (std::size_t at = 0; at < bytes->size;) {
        const auto length = bytes->data[at];
        if (!length) {
            at = (at / payload_size + 1) * payload_size;
            continue;
        }
        if (length < 34 || length > bytes->size - at || at % payload_size + length > payload_size)
            return false;
        const auto *row = bytes->data + at;
        const auto name_length = row[32];
        if (33u + name_length > length)
            return false;
        if (name_length == 1 && row[33] <= 1) {
            at += length;
            continue;
        }
        // Multi-extent/interleaved files are not present on the supported disc.
        if ((row[25] & 0x80) || row[1] || row[26] || row[27] ||
            le32(row + 2) != be32(row + 6) || le32(row + 10) != be32(row + 14))
            return false;
        char name[128]{};
        if (!name_length || name_length >= sizeof name)
            return false;
        std::memcpy(name, row + 33, name_length);
        for (unsigned i = 0; i < name_length; ++i)
            if (name[i] < 32 || name[i] >= 127 || name[i] == '/' || name[i] == '\\')
                return false;
        if (const char *version = std::strchr(name, ';');
            version && ((row[25] & 2) || std::strcmp(version, ";1") != 0))
            return false;
        DiscExtent file{};
        const int written =
            std::snprintf(file.path, sizeof file.path, "%s%s%s", importer->current.path,
                          importer->current.path[0] ? "/" : "", name);
        if (written < 0 || std::size_t(written) >= sizeof file.path)
            return false;
        file.sector = le32(row + 2);
        file.length = le32(row + 10);
        char normalized[128];
        if (!asset_path(normalized, sizeof normalized, file.path))
            return false;
        std::snprintf(file.path, sizeof file.path, "%s", normalized);
        if (row[25] & 2) {
            for (std::size_t i = 0; i < importer->visited_count; ++i)
                if (importer->visited_directories[i] == file.sector)
                    return false;
            if (importer->visited_count == 128 || importer->directory_count == 128)
                return false;
            importer->visited_directories[importer->visited_count++] = file.sector;
            importer->directories[importer->directory_count++] = file;
        } else {
            if (importer->file_count == 4096 || file.length > disc_import_limit - importer->file_bytes)
                return false;
            importer->file_bytes += file.length;
            importer->files[importer->file_count++] = file;
        }
        at += length;
    }
    return true;
}
void disc_import_supply(DiscImporter *importer, const u8 *bytes, std::size_t size) {
    if (!disc_import_waiting(importer))
        return;
    if (size != importer->request.length || (!bytes && size)) {
        disc_import_fail(importer, "Disc read was incomplete.");
        return;
    }
    if (importer->state == ImportState::layout) {
        constexpr u8 sync[] = {0,255,255,255,255,255,255,255,255,255,255,0};
        importer->sector_size = std::memcmp(bytes, sync, sizeof sync) == 0 ? 2352 : payload_size;
        if (importer->disc_size % importer->sector_size != 0) {
            disc_import_fail(importer, "Image has an incomplete data sector.");
            return;
        }
        importer->state = ImportState::volume;
        const DiscExtent volume{"", 16, payload_size};
        read_extent(importer, &volume);
        return;
    }
    ByteBuffer payload{};
    if (!unpack_payload(importer, bytes, size, &payload)) {
        buffer_release(&payload);
        disc_import_fail(importer, "Invalid data sector or allocation failure.");
        return;
    }
    if (importer->state == ImportState::volume) {
        if (payload.size != payload_size || std::memcmp(payload.data, "\1CD001\1", 7) != 0 ||
            std::memcmp(payload.data + 40, "SLPS-00017                      ", 32) != 0 ||
            payload.data[128] != 0 || payload.data[129] != 8 ||
            payload.data[130] != 8 || payload.data[131] != 0 ||
            le32(payload.data + 80) != be32(payload.data + 84) ||
            le32(payload.data + 80) > importer->disc_size / importer->sector_size) {
            disc_import_fail(importer, "Unsupported ISO9660 volume; Japanese SLPS-00017 is required.");
        } else {
            const auto *root = payload.data + 156;
            if (root[0] < 34 || root[1] || !(root[25] & 2) || root[26] || root[27] ||
                le32(root + 2) != be32(root + 6) || le32(root + 10) != be32(root + 14)) {
                disc_import_fail(importer, "Invalid ISO9660 root directory.");
            } else {
                importer->volume_sectors = le32(payload.data + 80);
                importer->directories[importer->directory_count++] = {
                    {}, le32(root + 2), le32(root + 10)};
                importer->visited_directories[importer->visited_count++] = le32(root + 2);
                next_directory(importer);
            }
        }
    } else if (importer->state == ImportState::directory) {
        if (!parse_directory(importer, &payload))
            disc_import_fail(importer, "Invalid ISO9660 directory.");
        else
            next_directory(importer);
    } else {
        if (!assets_append(&importer->assets, importer->current.path, &payload))
            disc_import_fail(importer, "Duplicate resource name or allocation failure.");
        else
            next_file(importer);
    }
    buffer_release(&payload);
}
double disc_import_progress(const DiscImporter *importer) {
    if (!importer->disc_size)
        return 0;
    if (importer->state == ImportState::complete)
        return 1;
    return 0.1 + (importer->file_count
                      ? 0.9 * double(importer->assets.count) / double(importer->file_count)
                      : 0);
}
void disc_import_release(DiscImporter *importer) {
    assets_release(&importer->assets);
    *importer = {};
}
} // namespace kf
