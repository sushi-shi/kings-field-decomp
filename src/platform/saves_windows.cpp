#include <kf/platform/saves.h>
#include <kf/platform/windows.h>
#include <SDL3/SDL.h>

namespace kf {
namespace {
std::filesystem::path save_directory;
HANDLE directory_handle = INVALID_HANDLE_VALUE;
unsigned pending_serial;

SaveFileResult io_error() {
    const DWORD error = GetLastError();
    return error == ERROR_DISK_FULL || error == ERROR_HANDLE_DISK_FULL || error == ERROR_DISK_QUOTA_EXCEEDED
        ? SaveFileResult::NoSpace : SaveFileResult::IoError;
}
bool valid_slot(SaveSlot slot) { return slot >= SaveSlot::First && slot <= SaveSlot::Third; }
std::filesystem::path slot_path(SaveSlot slot) {
    return save_directory / (L"slot" + std::to_wstring(static_cast<unsigned>(slot)) + L".kfs");
}
}

bool save_storage_start(const char *directory) {
    save_storage_shutdown();
    char *default_path = directory ? nullptr : SDL_GetPrefPath("KingsField", "SLPS00017");
    const char *path = directory ? directory : default_path;
    if (path) {
        save_directory = windows_path(path);
        directory_handle = windows_open(save_directory.c_str(), true);
    }
    SDL_free(default_path);
    if (directory_handle == INVALID_HANDLE_VALUE)
        std::fprintf(stderr, "Cannot open save directory: %s\n", directory ? directory : "default user storage");
    return directory_handle != INVALID_HANDLE_VALUE;
}

void save_storage_shutdown() {
    if (directory_handle != INVALID_HANDLE_VALUE)
        CloseHandle(directory_handle);
    directory_handle = INVALID_HANDLE_VALUE;
    save_directory.clear();
}

SaveFileResult save_file_read(SaveSlot slot, u8 *data, std::size_t capacity, std::size_t *size) {
    if (size)
        *size = 0;
    if (!valid_slot(slot) || !data || !size || !capacity || capacity > save_file_capacity)
        return SaveFileResult::Invalid;
    if (directory_handle == INVALID_HANDLE_VALUE)
        return SaveFileResult::Unavailable;
    HANDLE file = windows_open(slot_path(slot).c_str(), false);
    if (file == INVALID_HANDLE_VALUE)
        return GetLastError() == ERROR_FILE_NOT_FOUND ? SaveFileResult::Missing : io_error();
    LARGE_INTEGER length{};
    DWORD loaded = 0;
    auto result = SaveFileResult::Ok;
    if (!GetFileSizeEx(file, &length))
        result = io_error();
    else if (length.QuadPart <= 0 || static_cast<unsigned long long>(length.QuadPart) > capacity)
        result = SaveFileResult::Invalid;
    else if (!ReadFile(file, data, static_cast<DWORD>(length.QuadPart), &loaded, nullptr) || loaded != length.QuadPart)
        result = io_error();
    if (!CloseHandle(file))
        result = io_error();
    if (result == SaveFileResult::Ok)
        *size = loaded;
    return result;
}

SaveFileResult save_file_write(SaveSlot slot, const u8 *data, std::size_t size) {
    if (!valid_slot(slot) || !data || !size || size > save_file_capacity)
        return SaveFileResult::Invalid;
    if (directory_handle == INVALID_HANDLE_VALUE)
        return SaveFileResult::Unavailable;
    std::filesystem::path pending;
    HANDLE file = INVALID_HANDLE_VALUE;
    for (unsigned attempt = 0; attempt < 8 && file == INVALID_HANDLE_VALUE; ++attempt) {
        pending = save_directory / (L".slot." + std::to_wstring(GetCurrentProcessId()) + L"." +
            std::to_wstring(SDL_GetTicksNS()) + L"." + std::to_wstring(++pending_serial) + L".tmp");
        file = windows_open(pending.c_str(), false, GENERIC_WRITE, CREATE_NEW);
        if (file == INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_EXISTS)
            return io_error();
    }
    if (file == INVALID_HANDLE_VALUE)
        return SaveFileResult::IoError;
    DWORD written = 0;
    auto result = SaveFileResult::Ok;
    if (!WriteFile(file, data, static_cast<DWORD>(size), &written, nullptr) || written != size || !FlushFileBuffers(file))
        result = io_error();
    if (!CloseHandle(file))
        result = io_error();
    // The pending file is on the same volume. Publish only a complete, flushed save.
    if (result == SaveFileResult::Ok && !MoveFileExW(pending.c_str(), slot_path(slot).c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        result = io_error();
    if (result != SaveFileResult::Ok)
        DeleteFileW(pending.c_str());
    return result;
}
}
