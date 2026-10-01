#ifndef KF_PLATFORM_LANGUAGE_RUNTIME_H
#define KF_PLATFORM_LANGUAGE_RUNTIME_H

#include <kf/lib/types.h>
#include <kf/platform/files.h>
#include <kf/platform/language.h>


namespace kf {
bool language_resources_start(const char *data, Language language, const char *japanese_data);
void language_resources_stop();
bool language_request(Language language);
Language language_requested();
bool language_available(Language language);
FileResult language_file_open(DataFile *file, Language language, const char *path);
FileResult language_file_load(Language language, const char *path, std::vector<u8> &destination,
                              std::size_t capacity);
// Apply permanent changes between modal screens or in Configuration.
bool language_apply_pending();
void host_language_status(const char *message);
}

#endif // KF_PLATFORM_LANGUAGE_RUNTIME_H
