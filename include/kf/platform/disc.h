#ifndef KF_PLATFORM_DISC_H
#define KF_PLATFORM_DISC_H

#include <cstddef>
#include <kf/platform/language.h>

namespace kf {
// The supported retail CUE is one MODE2/2352 track at file offset zero.
bool disc_cue_image(const char *text, char *filename, std::size_t capacity);
// Verify and prepare the selected language in a new directory. Never overwrite.
bool disc_extract(const char *source, const char *destination, Language language);
bool disc_prepare_directory(const char *source, const char *destination, Language language);
// Startup-only verification also prevents a --data tree from mixing languages.
bool disc_verify_directory(const char *directory, Language language);
}

#endif // KF_PLATFORM_DISC_H
