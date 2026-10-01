#pragma once
#include <cstddef>
#include <cstdio>
#include <string>

namespace kf {
enum class FileResult { Ok, NotFound, InvalidPath, IoError, TooLarge, OutOfMemory };

struct DataFile {
    std::FILE *stream;
    std::size_t size;
};

// Paths are ordinary, case-sensitive paths relative to the extracted disc root.
bool data_files_set_root(const char *directory);
// Canonical path/length/content identity of the selected tree; empty on failure.
std::string data_files_hash();
FileResult data_file_open(DataFile *file, const char *path);
FileResult data_file_read(DataFile *file, void *destination, std::size_t capacity);
void data_file_close(DataFile *file);
FileResult data_file_read_into(const char *path, void *destination, std::size_t capacity,
                              std::size_t *loaded_size = nullptr);
void data_file_report_error(FileResult result, const char *path);
}
