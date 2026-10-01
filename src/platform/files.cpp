#include <kf/platform/files.h>
#include <kf/platform/assets.h>
#include <algorithm>
#include <array>
#include <filesystem>
#include <vector>
#include <kf/platform/files.h>

#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>

namespace kf {
static constexpr unsigned ascii_first_printable = 32;
static std::string data_root;

bool data_files_set_root(const char *directory) {
    struct stat info {};
    if (!directory || stat(directory, &info) != 0 || !S_ISDIR(info.st_mode)) {
        std::fprintf(stderr, "Resource directory is unavailable: %s\n", directory ? directory : "(null)");
        return false;
    }
    try {
        data_root = directory;
    } catch (const std::bad_alloc &) {
        return false;
    } catch (const std::length_error &) {
        return false;
    }
    return true;
}

static bool valid_relative_path(const char *path) {
    if (!path || !*path)
        return false;
    const char *segment = path;
    for (const char *at = path;; ++at) {
        if (*at == '\\' || *at == ':' || *at == ';')
            return false;
        if (*at && static_cast<unsigned char>(*at) < ascii_first_printable)
            return false;
        if (*at && *at != '/')
            continue;
        const auto length = at - segment;
        if (!length || (length == 1 && segment[0] == '.') ||
            (length == 2 && segment[0] == '.' && segment[1] == '.'))
            return false;
        if (!*at)
            return true;
        segment = at + 1;
    }
}

std::string data_files_hash() {
    if (data_root.empty()) return {};
    namespace fs = std::filesystem;
    std::error_code error;
    const fs::path root(data_root);
    std::vector<std::string> paths;
    std::size_t directories = 0;
    fs::recursive_directory_iterator at(root, error), end;
    if (error) return {};
    while (at != end) {
        const auto status = at->symlink_status(error);
        if (error || at.depth() > 8) return {};
        if (fs::is_directory(status)) {
            if (++directories > disc_directory_capacity) return {};
        } else if (fs::is_regular_file(status)) {
            const auto path = at->path().lexically_relative(root).generic_string();
            if (path.size() >= asset_path_capacity || !valid_relative_path(path.c_str()) ||
                paths.size() == disc_file_capacity) return {};
            paths.push_back(path);
        } else return {}; // Do not traverse links, devices or pipes as resources.
        at.increment(error);
        if (error) return {};
    }
    if (paths.empty()) return {};
    std::sort(paths.begin(), paths.end());
    Sha256 hash {};
    sha256_init(&hash);
    std::array<u8, 4 * 1024> buffer;
    std::size_t total = 0;
    for (const auto &path : paths) {
        DataFile file {};
        if (data_file_open(&file, path.c_str()) != FileResult::Ok) return {};
        bool ok = file.size <= disc_import_limit - total;
        if (ok) {
            total += file.size;
            u8 sizes[8];
            for (unsigned byte = 0; byte < 4; ++byte) {
                sizes[byte] = static_cast<u8>(path.size() >> (byte * 8));
                sizes[byte + 4] = static_cast<u8>(file.size >> (byte * 8));
            }
            sha256_update(&hash, sizes);
            sha256_update(&hash, {reinterpret_cast<const u8 *>(path.data()), path.size()});
            for (std::size_t remaining = file.size; remaining && ok;) {
                const auto count = std::min(remaining, buffer.size());
                ok = std::fread(buffer.data(), 1, count, file.stream) == count;
                if (ok) sha256_update(&hash, {buffer.data(), count});
                remaining -= count;
            }
            ok = ok && std::fgetc(file.stream) == EOF && !std::ferror(file.stream);
        }
        data_file_close(&file);
        if (!ok) return {};
    }
    char digest[sha256_hex_capacity];
    sha256_finish(&hash, digest);
    return digest;
}

FileResult data_file_open(DataFile *file, const char *path) {
    *file = {};
    if (data_root.empty() || !valid_relative_path(path))
        return FileResult::InvalidPath;
    const auto root_size = data_root.size();
    const auto path_size = std::strlen(path);
    if (root_size > std::numeric_limits<std::size_t>::max() - path_size - 2)
        return FileResult::TooLarge;
    std::string full_path;
    try {
        full_path = data_root + '/' + path;
    } catch (const std::bad_alloc &) {
        return FileResult::OutOfMemory;
    } catch (const std::length_error &) {
        return FileResult::TooLarge;
    }
    auto *stream = std::fopen(full_path.c_str(), "rb");
    const int open_error = errno;
    if (!stream)
        return open_error == ENOENT ? FileResult::NotFound : FileResult::IoError;
    struct stat info {};
    if (fstat(fileno(stream), &info) != 0 || !S_ISREG(info.st_mode) || info.st_size < 0) {
        std::fclose(stream);
        return FileResult::IoError;
    }
    if (static_cast<unsigned long long>(info.st_size) > std::numeric_limits<std::size_t>::max()) {
        std::fclose(stream);
        return FileResult::TooLarge;
    }
    *file = {stream, static_cast<std::size_t>(info.st_size)};
    return FileResult::Ok;
}

FileResult data_file_read(DataFile *file, void *destination, std::size_t capacity) {
    if (!file->size)
        return FileResult::IoError;
    if (file->size > capacity)
        return FileResult::TooLarge;
    if (!file->stream || (!destination && file->size) || std::fseek(file->stream, 0, SEEK_SET) != 0)
        return FileResult::IoError;
    if (file->size && std::fread(destination, 1, file->size, file->stream) != file->size)
        return FileResult::IoError;
    // Detect files changed between opening and reading; never report a partial read as success.
    if (std::fgetc(file->stream) != EOF || std::ferror(file->stream))
        return FileResult::IoError;
    return FileResult::Ok;
}

void data_file_close(DataFile *file) {
    if (file->stream)
        std::fclose(file->stream);
    *file = {};
}

FileResult data_file_read_into(const char *path, void *destination, std::size_t capacity,
                              std::size_t *loaded_size) {
    if (loaded_size)
        *loaded_size = 0;
    DataFile file {};
    auto result = data_file_open(&file, path);
    if (result == FileResult::Ok)
        result = data_file_read(&file, destination, capacity);
    if (result == FileResult::Ok && loaded_size)
        *loaded_size = file.size;
    data_file_close(&file);
    if (result != FileResult::Ok)
        data_file_report_error(result, path);
    return result;
}

void data_file_report_error(FileResult result, const char *path) {
    const char *reason = "unknown error";
    switch (result) {
    case FileResult::Ok: return;
    case FileResult::NotFound: reason = "not found"; break;
    case FileResult::InvalidPath: reason = "invalid relative path or unconfigured resource directory"; break;
    case FileResult::IoError: reason = "file I/O failed"; break;
    case FileResult::TooLarge: reason = "file exceeds destination capacity"; break;
    case FileResult::OutOfMemory: reason = "out of memory"; break;
    }
    std::fprintf(stderr, "Resource %s: %s\n", path ? path : "(null)", reason);
}
}
