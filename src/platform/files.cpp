#include <kf/platform/files.hpp>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <sys/stat.h>

namespace kf {
static constexpr unsigned ascii_first_printable = 32;
static char *data_root;

bool data_files_set_root(const char *directory) {
    struct stat info {};
    if (!directory || stat(directory, &info) != 0 || !S_ISDIR(info.st_mode)) {
        std::fprintf(stderr, "Resource directory is unavailable: %s\n", directory ? directory : "(null)");
        return false;
    }
    const auto length = std::strlen(directory);
    auto *copy = static_cast<char *>(std::malloc(length + 1));
    if (!copy)
        return false;
    std::memcpy(copy, directory, length + 1);
    std::free(data_root);
    data_root = copy;
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

FileResult data_file_open(DataFile *file, const char *path) {
    *file = {};
    if (!data_root || !valid_relative_path(path))
        return FileResult::InvalidPath;
    const auto root_size = std::strlen(data_root);
    const auto path_size = std::strlen(path);
    if (root_size > std::numeric_limits<std::size_t>::max() - path_size - 2)
        return FileResult::TooLarge;
    const auto size = root_size + path_size + 2;
    auto *full_path = static_cast<char *>(std::malloc(size));
    if (!full_path)
        return FileResult::OutOfMemory;
    std::snprintf(full_path, size, "%s/%s", data_root, path);
    auto *stream = std::fopen(full_path, "rb");
    const int open_error = errno;
    std::free(full_path);
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
