#include <kf/platform/language_runtime.hpp>
#include <kf/platform/disc.hpp>
#include <kf/platform/files.hpp>
#include <kf/platform/translation.hpp>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace kf {
namespace {
std::string roots[2];
bool verified[2];
std::string temporary_root;
Language requested = Language::Japanese;

unsigned index(Language language) { return language == Language::English ? 1 : 0; }

void remove_temporary_resources() {
    if (!temporary_root.empty()) {
        std::error_code error;
        std::filesystem::remove_all(temporary_root, error);
        temporary_root.clear();
    }
}

bool prepare_english() {
    std::error_code error;
    auto path = (std::filesystem::temp_directory_path(error) / "kings-field-language-XXXXXX").string();
    if (error || !::mkdtemp(path.data()))
        return false;
    temporary_root = path;
    const auto destination = path + "/en";
    if (!disc_prepare_directory(roots[0].c_str(), destination.c_str(), Language::English)) {
        remove_temporary_resources();
        return false;
    }
    roots[1] = destination;
    verified[1] = true;
    return true;
}
}

bool language_resources_start(const char *data, Language language, const char *japanese_data) {
    language_resources_stop();
    if (!data_files_set_root(data))
        return false;
    if (japanese_data)
        roots[0] = japanese_data;
    roots[index(language)] = data;
    // main has already verified the active tree. An alternate tree is verified
    // before its first use; newly generated English is verified by the importer.
    verified[index(language)] = true;
    requested = language;
    game_set_language(language);
    return true;
}

void language_resources_stop() {
    remove_temporary_resources();
    roots[0].clear();
    roots[1].clear();
    verified[0] = verified[1] = false;
}

bool language_available(Language language) {
    return !roots[index(language)].empty() ||
        (language == Language::English && !roots[0].empty() && translation_available());
}

bool language_request(Language language) {
    if (!language_available(language)) {
        host_language_status(language == Language::Japanese
            ? "Japanese resources are unavailable. Launch with --japanese-data DIRECTORY."
            : "English translation data is unavailable in this build.");
        return false;
    }
    requested = language;
    host_language_status(requested == game_language()
        ? "Language unchanged."
        : "Language change queued until gameplay or Configuration resumes.");
    return true;
}

Language language_requested() { return requested; }

bool language_apply_pending() {
    if (requested == game_language())
        return false;
    const auto target = requested;
    const auto slot = index(target);
    requested = game_language();
    if ((roots[slot].empty() && !prepare_english()) ||
        (!verified[slot] && !disc_verify_directory(roots[slot].c_str(), target)) ||
        !data_files_set_root(roots[slot].c_str())) {
        host_language_status("Cannot prepare the selected language. Current language retained.");
        return false;
    }
    verified[slot] = true;
    requested = target;
    game_set_language(target);
    return true;
}
}
