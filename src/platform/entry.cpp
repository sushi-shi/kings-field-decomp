#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <kf/platform/host.hpp>
#include <kf/platform/input.hpp>
#include <kf/platform/files.hpp>
#include <kf/platform/disc.hpp>
#include <kf/platform/saves.hpp>
#include <kf/net/transport.hpp>
#include <kf/platform/assets.hpp>
#include <kf/platform/avatars.hpp>
#include <cstdlib>
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
    const char *avatars = nullptr;
    const char *avatar_disc = nullptr;
    bool data_selected = false, extract_only = false;
    bool skip_intro = false;
    auto &online = kf::net::application_config;
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
        else if (std::strcmp(argv[i], "--avatars") == 0 && i + 1 < argc)
            avatars = argv[++i];
        else if (std::strcmp(argv[i], "--avatar-disc") == 0 && i + 1 < argc)
            avatar_disc = argv[++i];
        else if (std::strcmp(argv[i], "--avatar") == 0 && i + 1 < argc) {
            char *end = nullptr;
            const auto slot = std::strtol(argv[++i], &end, 10);
            if (!*argv[i] || *end || slot < 0 || slot >= KF_AVATAR_SLOTS) return 1;
            online.avatar = static_cast<u8>(slot);
        }
        else if (std::strcmp(argv[i], "--host") == 0)
            online.host = true;
        else if (std::strcmp(argv[i], "--join") == 0 && i + 1 < argc)
            online.room = argv[++i];
        else if (std::strcmp(argv[i], "--signal") == 0 && i + 1 < argc)
            online.signaling_url = argv[++i];
        else if (std::strcmp(argv[i], "--campaign") == 0 && i + 1 < argc &&
                 argv[i + 1][0] >= '1' && argv[i + 1][0] <= '3' && !argv[i + 1][1])
            online.campaign_slot = static_cast<u8>(argv[++i][0] - '0');
        else {
            std::fprintf(stderr, "Usage: kings-field [--data DIRECTORY | --disc IMAGE [--extract-to NEW_DIRECTORY] [--extract-only]] [--saves DIRECTORY] [--skip-intro] [--host | --join ROOM] [--signal URL] [--avatar-disc KFIII_IMAGE | --avatars PACK] [--avatar ID]\n");
            return 1;
        }
    }
    if ((online.campaign_slot && !online.host) || (online.host && !online.room.empty()) ||
        (!online.signaling_url.empty() && !online.host && online.room.empty())) {
        std::fprintf(stderr, "Select --host or --join ROOM for an online session.\n");
        return 1;
    }
    if ((avatars && avatar_disc) || (avatars && !kf::avatars_load(avatars)) ||
        (avatar_disc && !kf::avatars_import_disc(avatar_disc)) ||
        ((avatars || avatar_disc) && online.avatar != 0xff && !kf::avatar_mesh(online.avatar))) {
        std::fprintf(stderr, "Cannot load the character pack or selected character.\n");
        return 1;
    }
    if (online.host || !online.room.empty()) {
        if (!kf::avatar_mesh(online.avatar == 0xff ? 41 : online.avatar)) {
            std::fprintf(stderr, "Multiplayer needs character resources: use --avatar-disc KFIII_IMAGE or --avatars PACK.\n");
            return 1;
        }
        if (online.signaling_url.empty()) online.signaling_url = "ws://127.0.0.1:8787";
        online.avatar_recipe = kf::avatars_hash();
        skip_intro = true;
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
    if (online.host || !online.room.empty()) {
        online.resources = kf::data_files_hash();
        if (online.resources.empty()) {
            std::fprintf(stderr, "Cannot verify multiplayer resources. Use a complete extracted disc directory containing ordinary files.\n");
            return 1;
        }
    }
    // Create browser audio within the launch gesture, before asynchronous storage.
    if (!kf::host_start())
        return 1;
    if (!kf::save_storage_start(saves))
        kf::host_fail("Cannot initialize save storage. Check browser storage permissions or the selected save directory.");
    if (!online.signaling_url.empty()) {
        std::array<u8, 32> credential;
        if (!kf::online_profile_load(credential))
            kf::host_fail("Cannot load the multiplayer profile from save storage.");
        constexpr char digits[] = "0123456789abcdef";
        for (auto byte : credential) {
            online.credential += digits[byte >> 4];
            online.credential += digits[byte & 15];
        }
    }
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
            if (mode == kf::AppMode::Ending && !online.signaling_url.empty()) {
                std::puts("Playing the campaign ending");
                std::fflush(stdout);
            }
            kf_run_opening(mode);
            mode = mode == kf::AppMode::Ending && !online.signaling_url.empty()
                ? kf::AppMode::Exit : kf::AppMode::Gameplay;
            break;
        case kf::AppMode::Exit:
            kf::save_storage_shutdown();
            kf::host_shutdown();
            return 0;
        }
    }
}
