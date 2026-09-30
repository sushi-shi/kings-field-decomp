#ifndef KF_PLATFORM_WINDOWS_H
#define KF_PLATFORM_WINDOWS_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdio>
#include <cerrno>
#include <algorithm>
#include <sys/stat.h>
#include <filesystem>
#include <string>
#include <fcntl.h>
#include <io.h>

namespace kf {
inline std::wstring windows_path(const char *utf8) {
    if (!utf8 || !*utf8)
        return {};
    const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, nullptr, 0);
    if (!size)
        return {};
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, result.data(), size);
    result.pop_back();
    std::replace(result.begin(), result.end(), L'/', L'\\');
    if (result.starts_with(L"\\\\?\\"))
        return result;
    const auto separator = result.find_last_of(L"\\:");
    const auto leaf = separator == std::wstring::npos ? result : result.substr(separator + 1);
    const auto parent = separator == std::wstring::npos ? L".\\" : result.substr(0, separator + 1);
    const DWORD capacity = GetFullPathNameW(parent.c_str(), 0, nullptr, nullptr);
    if (!capacity)
        return {};
    std::wstring absolute(capacity, L'\0');
    const DWORD length = GetFullPathNameW(parent.c_str(), capacity, absolute.data(), nullptr);
    if (!length || length >= capacity)
        return {};
    absolute.resize(length);
    if (absolute.back() != L'\\')
        absolute += L'\\';
    absolute += leaf;
    // Extended paths preserve the disc's real trailing-dot names (E0., END., ...).
    return absolute.starts_with(L"\\\\") ? L"\\\\?\\UNC\\" + absolute.substr(2) : L"\\\\?\\" + absolute;
}

inline std::string utf8_path(const std::filesystem::path &path) {
    const auto text = path.u8string();
    return {reinterpret_cast<const char *>(text.data()), text.size()};
}

inline std::filesystem::path windows_temporary_directory(const wchar_t *prefix) {
    std::error_code error;
    const auto directory = std::filesystem::temp_directory_path(error);
    if (error)
        return {};
    const std::filesystem::path base(windows_path(utf8_path(directory).c_str()));
    for (unsigned attempt = 0; attempt < 128; ++attempt) {
        const auto candidate = base / (std::wstring(prefix) + std::to_wstring(GetCurrentProcessId()) +
            L"-" + std::to_wstring(GetTickCount64()) + L"-" + std::to_wstring(attempt));
        if (CreateDirectoryW(candidate.c_str(), nullptr))
            return candidate;
        if (GetLastError() != ERROR_ALREADY_EXISTS)
            break;
    }
    return {};
}

// Open the object itself and reject junctions, symlinks and device handles.
inline HANDLE windows_open(const wchar_t *path, bool directory, DWORD access = GENERIC_READ,
                           DWORD creation = OPEN_EXISTING) {
    HANDLE file = CreateFileW(path, access, FILE_SHARE_READ, nullptr, creation,
        FILE_FLAG_OPEN_REPARSE_POINT | (directory ? FILE_FLAG_BACKUP_SEMANTICS : 0), nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return file;
    BY_HANDLE_FILE_INFORMATION info{};
    if (!GetFileInformationByHandle(file, &info) || GetFileType(file) != FILE_TYPE_DISK ||
        (info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ||
        bool(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != directory) {
        CloseHandle(file);
        SetLastError(ERROR_INVALID_DATA);
        return INVALID_HANDLE_VALUE;
    }
    return file;
}

inline FILE *windows_fopen(const char *path) {
    HANDLE file = windows_open(windows_path(path).c_str(), false);
    if (file == INVALID_HANDLE_VALUE) {
        errno = GetLastError() == ERROR_FILE_NOT_FOUND ? ENOENT : EIO;
        return nullptr;
    }
    const int descriptor = _open_osfhandle(reinterpret_cast<intptr_t>(file), _O_RDONLY | _O_BINARY);
    if (descriptor < 0) {
        CloseHandle(file);
        return nullptr;
    }
    FILE *stream = _fdopen(descriptor, "rb");
    if (!stream)
        _close(descriptor);
    return stream;
}
}

#endif // KF_PLATFORM_WINDOWS_H
