#ifndef KF_PLATFORM_DISC_H
#define KF_PLATFORM_DISC_H

#include <kf/platform/language.h>

#include <span>

namespace kf {
// The supported retail CUE is one MODE2/2352 track at file offset zero.
bool disc_cue_image(const char *text, std::span<char> filename);
// Verify and prepare the selected language in a new directory. Never overwrite.
bool disc_extract(const char *source, const char *destination, Language language);
bool disc_prepare_directory(const char *source, const char *destination, Language language);
// Startup-only verification also prevents a --data tree from mixing languages.
// With actual supplied, English startup may also accept a Japanese base to convert.
bool disc_verify_directory(const char *directory, Language language, Language *actual = nullptr);
}

#endif // KF_PLATFORM_DISC_H
