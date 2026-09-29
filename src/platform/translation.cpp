#include <kf/platform/translation.hpp>
#include <cstring>
#include "english_patch.inc"

namespace kf {
namespace {
struct PatchReader {
    std::span<const u8> remaining;
    bool word(u32 *value) {
        if (remaining.size() < 4)
            return false;
        *value = u32(remaining[0]) | (u32(remaining[1]) << 8) |
                 (u32(remaining[2]) << 16) | (u32(remaining[3]) << 24);
        remaining = remaining.subspan(4);
        return true;
    }
};
}

bool translation_apply(AssetTable *assets, std::span<const u8> patch) {
    constexpr u8 magic[] = {'K', 'F', 'E', 'N', 1, 0, 0, 0};
    if (patch.size() < sizeof magic || std::memcmp(patch.data(), magic, sizeof magic))
        return false;
    PatchReader reader{patch.subspan(sizeof magic)};
    u32 files;
    if (!reader.word(&files) || !files || files > assets->count)
        return false;
    char previous[asset_path_capacity]{};
    for (u32 file = 0; file < files; ++file) {
        u32 name_length, file_size, spans;
        if (!reader.word(&name_length) || !reader.word(&file_size) || !reader.word(&spans) ||
            !name_length || name_length >= asset_path_capacity ||
            name_length > reader.remaining.size() || !spans || spans > file_size)
            return false;
        char name[asset_path_capacity]{};
        std::memcpy(name, reader.remaining.data(), name_length);
        reader.remaining = reader.remaining.subspan(name_length);
        char normalized[asset_path_capacity];
        if (std::memchr(name, 0, name_length) || !asset_path(normalized, sizeof normalized, name) ||
            std::strcmp(name, normalized) || std::strcmp(previous, name) >= 0)
            return false;
        std::strcpy(previous, name);
        const auto *asset = assets_find(assets, name);
        if (!asset || asset->bytes.size != file_size)
            return false;
        std::size_t end = 0;
        for (u32 span = 0; span < spans; ++span) {
            u32 offset, size;
            if (!reader.word(&offset) || !reader.word(&size) || !size || offset < end ||
                offset > file_size || size > file_size - offset || size > reader.remaining.size())
                return false;
            std::memcpy(asset->bytes.data + offset, reader.remaining.data(), size);
            reader.remaining = reader.remaining.subspan(size);
            end = std::size_t(offset) + size;
        }
    }
    return reader.remaining.empty();
}

const char *assets_prepare_language(AssetTable *assets, Language language) {
    if (assets_match_language(assets, language))
        return nullptr;
    if (language != Language::English || !assets_match_language(assets, Language::Japanese))
        return "Resources do not match the supported Japanese SLPS-00017 disc or selected language.";
    if (sizeof english_patch_data == 1)
        return "English translation data is not included in this build. Japanese remains available.";
    if (!translation_apply(assets, {english_patch_data, sizeof english_patch_data - 1}) ||
        !assets_match_language(assets, Language::English))
        return "English resource generation failed verification. No cache was published.";
    return nullptr;
}
}
