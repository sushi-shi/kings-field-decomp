#include <kf/platform/disc.h>
#include <kf/platform/files.h>
#include <kf/platform/host.h>
#include <kf/platform/input.h>
#include <kf/platform/language_runtime.h>
#include <kf/platform/saves.h>

#include <kf/net/transport.hpp>
#include <kf/platform/avatars.hpp>
#include <array>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef __EMSCRIPTEN__
#include <kf/platform/assets.h>

#include <emscripten.h>

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
    if (!kf::net::application_config.signaling_url.empty()) return 0;
    kf::Language language;
    return kf::language_parse(code, &language) && kf::language_request(language);
}
extern "C" EMSCRIPTEN_KEEPALIVE const char *kf_current_language() {
    return kf::language_code(kf::game_language());
}
EM_JS(void, browser_game_started, (), { Module['gameStarted'](); });
#endif

#include <kf/cutscene/playback.h>
#include <kf/game/session.h>

int main(int argc, char **argv) {
    const char *data = "data";
    const char *saves = nullptr;
    const char *disc = nullptr;
    const char *extracted = nullptr;
    const char *avatars = nullptr, *avatar_disc = nullptr;
    bool skip_intro = false;
    auto &online = kf::net::application_config;
    const char *japanese_data = nullptr;
    bool data_selected = false, extract_only = false;
    const char *language_code = std::getenv("KF_LANGUAGE");
    if (!language_code)
        language_code = "en";
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
        else if (std::strcmp(argv[i], "--language") == 0 && i + 1 < argc)
            language_code = argv[++i];
        else if (std::strcmp(argv[i], "--japanese-data") == 0 && i + 1 < argc)
            japanese_data = argv[++i];
        else {
            const bool help = std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0;
            std::fprintf(help ? stdout : stderr, "Usage: kings-field [--data DIRECTORY | --disc IMAGE] [--extract-to NEW_DIRECTORY] [--extract-only] [--language ja|en] [--japanese-data DIRECTORY] [--saves DIRECTORY] [--host | --join CODE] [--signal URL] [--avatars PACK | --avatar-disc IMAGE] [--avatar ID] [--campaign 1|2|3] [--skip-intro]\nLanguage defaults to KF_LANGUAGE, or en. Change it during play in Configuration. When starting from an English tree, --japanese-data supplies the original resources for switching back.\n");
            return help ? 0 : 1;
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
    kf::Language resource_language = language;
    if (!disc && !kf::disc_verify_directory(data, language, &resource_language))
        return 1;
    if (!kf::language_resources_start(data, resource_language, japanese_data))
        return 1;
    if (resource_language != language &&
        (!kf::language_request(language) || !kf::language_apply_pending())) {
        kf::language_resources_stop();
        return 1;
    }
    std::printf("Starting King's Field (%s).\n", kf::language_name(language));
    if (online.host || !online.room.empty()) {
        online.resources = kf::data_files_hash();
        if (online.resources.empty()) {
            std::fprintf(stderr, "Cannot verify multiplayer resources. Use a complete extracted disc directory containing ordinary files.\n");
            return 1;
        }
    }
    // Create browser audio within the launch gesture, before asynchronous storage.
    if (!kf::host_start()) {
        kf::language_resources_stop();
        return 1;
    }
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
    if (!skip_intro) cutscene_play(Cutscene::Intro);
    for (;;) {
        const GameResult result = game_play();
        if (result == GameResult::Completed) cutscene_play(Cutscene::Ending);
        else if (online.signaling_url.empty()) cutscene_play(Cutscene::Intro);
        if (!online.signaling_url.empty()) {
            kf::save_storage_shutdown(); kf::host_shutdown(); return 0;
        }
    }
}
