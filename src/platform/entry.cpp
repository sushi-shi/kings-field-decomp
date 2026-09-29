#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <kf/platform/host.hpp>
#include <kf/platform/input.hpp>
#include <kf/platform/files.hpp>
#include <kf/platform/disc.hpp>
#include <kf/platform/saves.hpp>
#include <kf/platform/language_runtime.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <kf/platform/assets.hpp>

extern "C" EMSCRIPTEN_KEEPALIVE int kf_extract_disc(const char *source, const char *destination,
                                                  const char *code) {
    kf::Language language;
    return kf::language_parse(code, &language) && kf::disc_extract(source, destination, language);
}
extern "C" EMSCRIPTEN_KEEPALIVE int kf_prepare_resources(const char *source, const char *destination,
                                                       const char *code) {
    kf::Language language;
    return kf::language_parse(code, &language) && kf::disc_prepare_directory(source, destination, language);
}
extern "C" EMSCRIPTEN_KEEPALIVE const char *kf_resource_files_hash(const char *code) {
    kf::Language language;
    return kf::language_parse(code, &language) ? kf::assets_language_hash(language) : "";
}
extern "C" EMSCRIPTEN_KEEPALIVE int kf_request_language(const char *code) {
    kf::Language language;
    return kf::language_parse(code, &language) && kf::language_request(language);
}
extern "C" EMSCRIPTEN_KEEPALIVE const char *kf_current_language() {
    return kf::language_code(kf::game_language());
}
EM_JS(void, browser_game_started, (), { Module['gameStarted'](); });
#endif

extern "C" kf::AppMode kf_run_game();
extern "C" void kf_run_opening(kf::AppMode mode);

int main(int argc, char **argv) {
    const char *data = "data";
    const char *saves = nullptr;
    const char *disc = nullptr;
    const char *extracted = nullptr;
    const char *japanese_data = nullptr;
    bool data_selected = false, extract_only = false;
    bool skip_intro = false;
    const char *language_code = std::getenv("KF_LANGUAGE");
    if (!language_code)
        language_code = "ja";
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--data") == 0 && i + 1 < argc) {
            data = argv[++i];
            data_selected = true;
        }
        else if (std::strcmp(argv[i], "--disc") == 0 && i + 1 < argc)
            disc = argv[++i];
        else if (std::strcmp(argv[i], "--extract-to") == 0 && i + 1 < argc)
            extracted = argv[++i];
        else if (std::strcmp(argv[i], "--extract-only") == 0)
            extract_only = true;
        else if (std::strcmp(argv[i], "--saves") == 0 && i + 1 < argc)
            saves = argv[++i];
        else if (std::strcmp(argv[i], "--skip-intro") == 0)
            skip_intro = true;
        else if (std::strcmp(argv[i], "--language") == 0 && i + 1 < argc)
            language_code = argv[++i];
        else if (std::strcmp(argv[i], "--japanese-data") == 0 && i + 1 < argc)
            japanese_data = argv[++i];
        else {
            const bool help = std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0;
            std::fprintf(help ? stdout : stderr, "Usage: kings-field [--data DIRECTORY | --disc IMAGE] [--extract-to NEW_DIRECTORY] [--extract-only] [--language ja|en] [--japanese-data DIRECTORY] [--saves DIRECTORY] [--skip-intro]\nLanguage defaults to KF_LANGUAGE, or ja. Change it during play in Configuration. When starting from an English tree, --japanese-data supplies the original resources for switching back.\n");
            return help ? 0 : 1;
        }
    }
    kf::Language language;
    if (!kf::language_parse(language_code, &language)) {
        std::fprintf(stderr, "Unsupported language: %s. Use ja or en.\n", language_code);
        return 1;
    }
    kf::game_set_language(language);
    if ((disc && data_selected) || (!disc && !data_selected && (extracted || extract_only)) ||
        (data_selected && extract_only && !extracted)) {
        std::fprintf(stderr, "Use --data or --disc. To prepare an existing tree, use --data with --extract-to NEW_DIRECTORY.\n");
        return 1;
    }
    if (disc) {
        data = extracted ? extracted : "data";
        if (!kf::disc_extract(disc, data, language))
            return 1;
        if (extract_only)
            return 0;
    }
    if (data_selected && extracted) {
        if (!kf::disc_prepare_directory(data, extracted, language))
            return 1;
        data = extracted;
        if (extract_only)
            return 0;
    }
    if (!disc && !kf::disc_verify_directory(data, language))
        return 1;
    if (!kf::language_resources_start(data, language, japanese_data))
        return 1;
    std::printf("Starting King's Field (%s).\n", kf::language_name(language));
    // Create browser audio within the launch gesture, before asynchronous storage.
    if (!kf::host_start())
        return 1;
    if (!kf::save_storage_start(saves))
        kf::host_fail("Cannot initialize save storage. Check browser storage permissions or the selected save directory.");
#ifdef __EMSCRIPTEN__
    browser_game_started();
#endif
    kf::AppMode mode = skip_intro ? kf::AppMode::Gameplay : kf::AppMode::Opening;
    for (;;) {
        switch (mode) {
        case kf::AppMode::Gameplay:
            kf::host_set_input_context(kf::InputContext::Gameplay);
            mode = kf_run_game();
            break;
        case kf::AppMode::Opening:
        case kf::AppMode::Ending:
            kf::host_set_input_context(kf::InputContext::Opening);
            kf_run_opening(mode);
            mode = kf::AppMode::Gameplay;
            break;
        }
    }
}
