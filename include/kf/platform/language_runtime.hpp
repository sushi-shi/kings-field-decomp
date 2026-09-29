#pragma once
#include <kf/platform/language.hpp>

namespace kf {
bool language_resources_start(const char *data, Language language, const char *japanese_data);
void language_resources_stop();
bool language_request(Language language);
Language language_requested();
bool language_available(Language language);
// Call only where no menu/dialogue holds copies of the old language's labels.
bool language_apply_pending();
void host_language_status(const char *message);
}
