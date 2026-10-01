#ifndef KF_PLATFORM_DISC_WINDOWS_H
#define KF_PLATFORM_DISC_WINDOWS_H

// Included inside namespace kf by disc.cpp; the importer and CUE parser are shared.
static bool write_resource_tree(const char *destination, const AssetTable &assets, Language language) {
    const std::filesystem::path root(windows_path(destination));
    if (!CreateDirectoryW(root.c_str(), nullptr)) {
        std::fprintf(stderr, "Cannot create %s. Extraction requires a new directory; use --data to reuse one.\n", destination);
        return false;
    }
    bool ok = true;
    for (std::size_t i = 0; ok && i < assets.size(); ++i) {
        const auto &asset = assets[i];
        const auto path = (root / std::filesystem::path(reinterpret_cast<const char8_t *>(asset.path.data()))).make_preferred();
        std::error_code error;
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) { ok = false; break; }
        HANDLE file = windows_open(path.c_str(), false, GENERIC_WRITE, CREATE_NEW);
        if (file == INVALID_HANDLE_VALUE) { ok = false; break; }
        DWORD written = 0;
        ok = WriteFile(file, asset.bytes.data(), static_cast<DWORD>(asset.bytes.size()), &written, nullptr) &&
            written == asset.bytes.size();
        if (!CloseHandle(file))
            ok = false;
    }
    if (ok)
        std::printf("Extracted %zu verified %s files to %s\n", assets.size(), language_name(language), destination);
    else
        std::fprintf(stderr, "Extraction failed; partial files remain in %s. No existing directory was replaced.\n", destination);
    return ok;
}

static bool read_resource_tree(const std::filesystem::path &root, const char *prefix, AssetTable &assets,
                               std::size_t *total, unsigned *directories) {
    if (++*directories > 128)
        return false;
    HANDLE directory = windows_open(root.c_str(), true);
    if (directory == INVALID_HANDLE_VALUE)
        return false;
    std::error_code error;
    std::filesystem::directory_iterator iterator(root, error), end;
    bool ok = !error;
    while (ok && iterator != end) {
        const auto entry = iterator->path();
        const auto name = utf8_path(entry.filename());
        std::array<char, asset_path_capacity> path, normalized;
        const int length = std::snprintf(path.data(), path.size(), "%s%s", prefix, name.c_str());
        const DWORD attributes = GetFileAttributesW(entry.c_str());
        if (length < 0 || static_cast<std::size_t>(length) >= path.size() - 1 ||
            !asset_path(normalized, path.data()) || std::strcmp(path.data(), normalized.data()) ||
            attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
            ok = false;
        } else if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
            path[length] = '/';
            path[length + 1] = 0;
            ok = read_resource_tree(entry, path.data(), assets, total, directories);
        } else {
            HANDLE file = windows_open(entry.c_str(), false);
            LARGE_INTEGER size{};
            ok = file != INVALID_HANDLE_VALUE && GetFileSizeEx(file, &size) && size.QuadPart >= 0 &&
                size.QuadPart <= 16 * 1024 * 1024 && assets.size() < 428 &&
                static_cast<std::uint64_t>(size.QuadPart) <= disc_import_limit - *total;
            ByteBuffer bytes{};
            DWORD loaded = 0;
            if (ok) {
                try {
                    bytes.resize(static_cast<std::size_t>(size.QuadPart));
                } catch (const std::bad_alloc &) {
                    ok = false;
                }
            }
            if (ok)
                ok = ReadFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &loaded, nullptr) &&
                    loaded == bytes.size();
            if (ok) {
                *total += bytes.size();
                ok = assets_append(assets, path.data(), std::move(bytes));
            }
            if (file != INVALID_HANDLE_VALUE && !CloseHandle(file))
                ok = false;
        }
        iterator.increment(error);
        if (error)
            ok = false;
    }
    if (!CloseHandle(directory))
        ok = false;
    return ok;
}

#endif // KF_PLATFORM_DISC_WINDOWS_H
