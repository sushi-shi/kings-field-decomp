#pragma once
#include <cstddef>

namespace kf {
// The supported retail CUE is one MODE2/2352 track at file offset zero.
bool disc_cue_image(const char *text, char *filename, std::size_t capacity);
// Verify first, then write unchanged files into a new directory. Never overwrite.
bool disc_extract(const char *source, const char *destination);
}
