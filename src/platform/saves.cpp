#include <kf/platform/saves.h>

#include <SDL3/SDL.h>

#include <array>
#include <cstdio>
#include <cstring>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
EM_JS_DEPS(save_strings, "$UTF8ToString");

EM_ASYNC_JS(int, browser_saves_open, (), {
    const saveSchemaVersion = 1;
    try {
        const db = await new Promise((resolve, reject) => {
            const request = indexedDB.open('kings-field-saves', saveSchemaVersion);
            let failed = false;
            request.onupgradeneeded = () => request.result.createObjectStore('files');
            request.onblocked = () => { failed = true; reject(new Error('Save database upgrade is blocked')); };
            request.onerror = () => { failed = true; reject(request.error); };
            request.onsuccess = () => {
                if (failed) request.result.close();
                else resolve(request.result);
            };
        });
        db.onversionchange = () => { db.close(); Module['kfSaveDb'] = null; };
        Module['kfSaveDb'] = db;
        return 1;
    } catch (error) {
        console.error('Save storage:', error);
        return 0;
    }
});

EM_JS(void, browser_saves_close, (), {
    if (Module['kfSaveDb']) Module['kfSaveDb'].close();
    Module['kfSaveDb'] = null;
});

EM_ASYNC_JS(int, browser_save_read, (const char *name, u8 *output, unsigned capacity, unsigned *size), {
    // Wire values mirror kf::SaveFileResult.
    const Result = {Ok: 0, Missing: 1, Invalid: 2, IoError: 3, NoSpace: 4, Unavailable: 5};
    const db = Module['kfSaveDb'];
    if (!db) return Result.Unavailable;
    try {
        const data = await new Promise((resolve, reject) => {
            const transaction = db.transaction('files', 'readonly');
            const request = transaction.objectStore('files').get(UTF8ToString(name));
            transaction.onabort = () => reject(transaction.error);
            transaction.oncomplete = () => resolve(request.result);
        });
        if (data === undefined) return Result.Missing;
        if (!(data instanceof Uint8Array) || !data.length || data.length > capacity) return Result.Invalid;
        HEAPU8.set(data, output);
        HEAPU32[size >> 2] = data.length;
        return Result.Ok;
    } catch (error) {
        console.error('Read save:', error);
        return Result.IoError;
    }
});

EM_ASYNC_JS(int, browser_save_write, (const char *name, const u8 *input, unsigned size), {
    // Wire values mirror kf::SaveFileResult.
    const Result = {Ok: 0, Missing: 1, Invalid: 2, IoError: 3, NoSpace: 4, Unavailable: 5};
    const db = Module['kfSaveDb'];
    if (!db) return Result.Unavailable;
    try {
        const data = HEAPU8.slice(input, input + size);
        await new Promise((resolve, reject) => {
            const transaction = db.transaction('files', 'readwrite', {durability: 'strict'});
            transaction.onabort = () => reject(transaction.error || new Error('Save transaction aborted'));
            transaction.oncomplete = () => resolve();
            // Do not silently downgrade the requested persistence guarantee.
            if (transaction.durability !== 'strict') {
                transaction.abort();
                return;
            }
            transaction.objectStore('files').put(data, UTF8ToString(name));
        });
        return Result.Ok;
    } catch (error) {
        console.error('Write save:', error);
        return error && error.name === 'QuotaExceededError' ? Result.NoSpace : Result.IoError;
    }
});
EM_JS(void, browser_campaign_write_begin, (int slot, const u8 *input, unsigned size), {
    Module['kfCampaignWrite'] = 6;
    const db = Module['kfSaveDb'];
    if (!db) { Module['kfCampaignWrite'] = 5; return; }
    let transaction;
    try {
        const data = HEAPU8.slice(input, input + size);
        transaction = db.transaction('files', 'readwrite', {durability: 'strict'});
        transaction.onabort = () => { Module['kfCampaignWrite'] = transaction.error?.name === 'QuotaExceededError' ? 4 : 3; };
        transaction.oncomplete = () => { Module['kfCampaignWrite'] = 0; };
        if (transaction.durability !== 'strict') { transaction.abort(); return; }
        transaction.objectStore('files').put(data, 'party' + slot + '.kfc');
    } catch (error) {
        if (transaction) {
            // A failed request must not later publish success or overwrite a retry.
            transaction.onabort = null;
            transaction.oncomplete = null;
            try { transaction.abort(); } catch {}
        }
        Module['kfCampaignWrite'] = error?.name === 'QuotaExceededError' ? 4 : 3;
    }
});
EM_JS(int, browser_campaign_write_poll, (), { return Module['kfCampaignWrite'] ?? 5; });
EM_ASYNC_JS(int, browser_online_profile, (u8 *output), {
    const db = Module['kfSaveDb'];
    if (!db) return 0;
    try {
        const credential = await new Promise((resolve, reject) => {
            const transaction = db.transaction('files', 'readwrite', {durability:'strict'});
            let value;
            transaction.onabort = () => reject(transaction.error);
            transaction.oncomplete = () => resolve(value);
            if (transaction.durability !== 'strict') { transaction.abort(); return; }
            const store = transaction.objectStore('files');
            const request = store.get('online-profile');
            request.onsuccess = () => {
                value = request.result;
                if (value === undefined) {
                    value = crypto.getRandomValues(new Uint8Array(32));
                    store.put(value, 'online-profile');
                }
                if (!(value instanceof Uint8Array) || value.length !== 32 || !value.some(b => b !== 0))
                    transaction.abort();
            };
        });
        HEAPU8.set(credential, output);
        return 1;
    } catch { return 0; }
});
#else
#include <sys/stat.h>

#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>

#endif

namespace kf {
#ifndef __EMSCRIPTEN__
static constexpr unsigned pending_name_capacity = 64;
static constexpr unsigned pending_name_attempts = 8;
static int save_directory = -1;
static unsigned pending_serial;
static SaveFileResult io_error() {
    return errno == ENOSPC || errno == EDQUOT ? SaveFileResult::NoSpace : SaveFileResult::IoError;
}
static bool sync_file(int descriptor) {
    int result;
    do { result = ::fsync(descriptor); } while (result != 0 && errno == EINTR);
    return result == 0;
}
static bool sync_directory_path(int directory) {
    int child = ::dup(directory);
    if (child < 0)
        return false;
    for (;;) {
        const int parent = ::openat(child, "..", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        struct stat child_info{}, parent_info{};
        const bool ok = parent >= 0 && ::fstat(child, &child_info) == 0
            && ::fstat(parent, &parent_info) == 0 && sync_file(child);
        ::close(child);
        if (!ok) {
            if (parent >= 0)
                ::close(parent);
            return false;
        }
        if (child_info.st_dev == parent_info.st_dev && child_info.st_ino == parent_info.st_ino) {
            ::close(parent);
            return true;
        }
        child = parent;
    }
}
#endif

static bool valid_slot(SaveSlot slot) {
    return slot >= SaveSlot::First && slot <= SaveSlot::Third;
}

bool save_storage_start(const char *directory) {
#ifdef __EMSCRIPTEN__
    // Browser slots are file byte streams in a dedicated IndexedDB transaction.
    // Resource extraction/cache uses a separate store and cannot delay their commit.
    return !directory && browser_saves_open() != 0;
#else
    char *default_path = directory ? nullptr : SDL_GetPrefPath("KingsField", "SLPS00017");
    const char *path = directory ? directory : default_path;
    if (path)
        save_directory = ::open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    SDL_free(default_path);
    // SDL may have created several preference directories. Publish their parent
    // entries before claiming that a first save in the leaf can be durable.
    if (!directory && save_directory >= 0 && !sync_directory_path(save_directory)) {
        ::close(save_directory);
        save_directory = -1;
    }
    if (save_directory < 0)
        std::fprintf(stderr, "Cannot open save directory: %s\n", directory ? directory : "default user storage");
    return save_directory >= 0;
#endif
}

void save_storage_shutdown() {
#ifdef __EMSCRIPTEN__
    browser_saves_close();
#else
    if (save_directory >= 0)
        ::close(save_directory);
    save_directory = -1;
#endif
}

static SaveFileResult read_named_file(const char *name, u8 *data, std::size_t capacity, std::size_t *size) {
    if (size)
        *size = 0;
    if (!data || !size || !capacity || capacity > campaign_file_capacity)
        return SaveFileResult::Invalid;
#ifdef __EMSCRIPTEN__
    unsigned loaded = 0;
    const auto result = static_cast<SaveFileResult>(browser_save_read(name, data, capacity, &loaded));
    *size = loaded;
    return result;
#else
    if (save_directory < 0)
        return SaveFileResult::Unavailable;
    const int file = ::openat(save_directory, name, O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK);
    if (file < 0)
        return errno == ENOENT ? SaveFileResult::Missing : io_error();
    struct stat info{};
    auto result = SaveFileResult::Ok;
    if (::fstat(file, &info) != 0)
        result = io_error();
    else if (!S_ISREG(info.st_mode) || info.st_size <= 0 || static_cast<unsigned long long>(info.st_size) > capacity)
        result = SaveFileResult::Invalid;
    std::size_t loaded = 0;
    while (result == SaveFileResult::Ok && loaded < static_cast<std::size_t>(info.st_size)) {
        const auto count = ::read(file, data + loaded, info.st_size - loaded);
        if (count > 0)
            loaded += count;
        else if (count == 0 || errno != EINTR)
            result = SaveFileResult::IoError;
    }
    if (result == SaveFileResult::Ok) {
        u8 extra;
        ssize_t count;
        do { count = ::read(file, &extra, 1); } while (count < 0 && errno == EINTR);
        if (count != 0)
            result = SaveFileResult::IoError;
    }
    if (::close(file) != 0)
        result = io_error();
    if (result == SaveFileResult::Ok)
        *size = loaded;
    return result;
#endif
}

static SaveFileResult write_named_file(const char *name, const u8 *data, std::size_t size) {
    if (!data || !size || size > campaign_file_capacity)
        return SaveFileResult::Invalid;
#ifdef __EMSCRIPTEN__
    return static_cast<SaveFileResult>(browser_save_write(name, data, size));
#else
    if (save_directory < 0)
        return SaveFileResult::Unavailable;
    char pending[pending_name_capacity];
    int file = -1;
    for (unsigned attempt = 0; attempt < pending_name_attempts && file < 0; ++attempt) {
        std::snprintf(pending, sizeof pending, ".save.%llx.%u.tmp",
            static_cast<unsigned long long>(SDL_GetTicksNS()), ++pending_serial);
        file = ::openat(save_directory, pending, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, S_IRUSR | S_IWUSR);
        if (file < 0 && errno != EEXIST)
            return io_error();
    }
    if (file < 0)
        return SaveFileResult::IoError;
    auto result = SaveFileResult::Ok;
    std::size_t written = 0;
    while (written < size) {
        const auto count = ::write(file, data + written, size - written);
        if (count > 0)
            written += count;
        else if (count == 0 || errno != EINTR) {
            result = io_error();
            break;
        }
    }
    if (result == SaveFileResult::Ok && !sync_file(file))
        result = io_error();
    if (::close(file) != 0)
        result = io_error();
    if (result == SaveFileResult::Ok && ::renameat(save_directory, pending, save_directory, name) != 0)
        result = io_error();
    if (result != SaveFileResult::Ok) {
        // Only the uniquely created, unpublished temporary file is removed.
        ::unlinkat(save_directory, pending, 0);
        return result;
    }
    // The rename is atomic; synchronizing its directory makes it durable too.
    return sync_file(save_directory) ? SaveFileResult::Ok : io_error();
#endif
}

static SaveFileResult read_file(SaveSlot slot, bool campaign, u8 *data, std::size_t capacity, std::size_t *size) {
    if (size) *size = 0;
    if (!valid_slot(slot) || capacity > (campaign ? campaign_file_capacity : save_file_capacity))
        return SaveFileResult::Invalid;
    char name[16];
    std::snprintf(name, sizeof name, campaign ? "party%u.kfc" : "slot%u.kfs", static_cast<unsigned>(slot));
    return read_named_file(name, data, capacity, size);
}
static SaveFileResult write_file(SaveSlot slot, bool campaign, const u8 *data, std::size_t size) {
    if (!valid_slot(slot) || size > (campaign ? campaign_file_capacity : save_file_capacity))
        return SaveFileResult::Invalid;
    char name[16];
    std::snprintf(name, sizeof name, campaign ? "party%u.kfc" : "slot%u.kfs", static_cast<unsigned>(slot));
    return write_named_file(name, data, size);
}

bool online_profile_load(std::array<u8, 32> &credential) {
    credential = {};
#ifdef __EMSCRIPTEN__
    return browser_online_profile(credential.data()) != 0;
#else
    // Serialize creation across two clients sharing a save directory. Publish
    // only a complete, durable private file; never replace a damaged profile.
    if (save_directory < 0 || ::flock(save_directory, LOCK_EX) != 0) return false;
    std::size_t size = 0;
    auto result = read_named_file("online-profile", credential.data(), credential.size(), &size);
    if (result == SaveFileResult::Missing) {
        if (::getentropy(credential.data(), credential.size()) == 0) {
            result = write_named_file("online-profile", credential.data(), credential.size());
            size = credential.size();
        }
    }
    ::flock(save_directory, LOCK_UN);
    if (result == SaveFileResult::Ok && size == credential.size() && credential != std::array<u8,32>{}) return true;
    credential = {};
    return false;
#endif
}

SaveFileResult save_file_read(SaveSlot slot, u8 *data, std::size_t capacity, std::size_t *size) {
    return read_file(slot, false, data, capacity, size);
}
SaveFileResult save_file_write(SaveSlot slot, const u8 *data, std::size_t size) {
    return write_file(slot, false, data, size);
}
SaveFileResult campaign_file_read(SaveSlot slot, u8 *data, std::size_t capacity, std::size_t *size) {
    return read_file(slot, true, data, capacity, size);
}
#ifndef __EMSCRIPTEN__
static SaveFileResult campaign_write_result = SaveFileResult::Unavailable;
#endif
bool campaign_file_write_begin(SaveSlot slot, const u8 *data, std::size_t size) {
    if (campaign_file_write_poll() == SaveFileResult::Pending || !valid_slot(slot) || !data || !size || size > campaign_file_capacity)
        return false;
#ifdef __EMSCRIPTEN__
    browser_campaign_write_begin(static_cast<int>(slot), data, size);
#else
    campaign_write_result = write_file(slot, true, data, size);
#endif
    return true;
}
SaveFileResult campaign_file_write_poll() {
#ifdef __EMSCRIPTEN__
    return static_cast<SaveFileResult>(browser_campaign_write_poll());
#else
    return campaign_write_result;
#endif
}
}
