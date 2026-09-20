#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <kf/platform/host.hpp>
#include <kf/platform/input.hpp>
#include <kf/platform/files.hpp>
#include <kf/platform/disc.hpp>
#include <kf/platform/saves.hpp>
#include <cstdio>
#include <cstring>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <kf/platform/assets.hpp>

extern "C" EMSCRIPTEN_KEEPALIVE int kf_extract_disc(const char *source, const char *destination) {
    return kf::disc_extract(source, destination);
}
extern "C" EMSCRIPTEN_KEEPALIVE const char *kf_retail_files_hash() {
    return kf::retail_files_sha256;
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
    bool data_selected = false, extract_only = false;
    bool skip_intro = false;
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
        else {
            std::fprintf(stderr, "Usage: kings-field [--data DIRECTORY | --disc IMAGE [--extract-to NEW_DIRECTORY] [--extract-only]] [--saves DIRECTORY] [--skip-intro]\n");
            return 1;
        }
    }
    if ((disc && data_selected) || (!disc && (extracted || extract_only))) {
        std::fprintf(stderr, "Use --data for existing files, or --disc with optional --extract-to/--extract-only.\n");
        return 1;
    }
    if (disc) {
        data = extracted ? extracted : "data";
        if (!kf::disc_extract(disc, data))
            return 1;
        if (extract_only)
            return 0;
    }
    if (!kf::data_files_set_root(data))
        return 1;
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
