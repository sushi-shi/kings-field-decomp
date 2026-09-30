#ifdef NDEBUG
#undef NDEBUG
#endif
#include <kf/platform/windows.h>
#include <kf/platform/files.h>
#include <kf/platform/saves.h>
#include <kf/renderer/renderer.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cassert>
#include <cstring>

namespace kf {
void host_language_status(const char *) {}
}

int main(int, char **) {
    const auto temporary = kf::windows_temporary_directory(L"kf-windows-test-");
    assert(!temporary.empty());
    const auto root = temporary / L"日本語 saves";
    assert(CreateDirectoryW(root.c_str(), nullptr));
    const auto path = kf::utf8_path(root);
    const auto dotted = kf::windows_path((path + "/E0.").c_str());
    std::fprintf(stderr, "Path: %s\n", kf::utf8_path(dotted).c_str());
    HANDLE file = kf::windows_open(dotted.c_str(), false, GENERIC_WRITE, CREATE_NEW);
    assert(file != INVALID_HANDLE_VALUE);
    DWORD written = 0;
    const u8 fixture[] = {0, 10, 13, 26, 255};
    assert(WriteFile(file, fixture, sizeof fixture, &written, nullptr) && written == sizeof fixture);
    assert(CloseHandle(file));
    assert(std::filesystem::directory_iterator(root)->path().filename() == L"E0.");
    assert(kf::data_files_set_root(path.c_str()));
    u8 data[kf::save_file_capacity]{};
    std::size_t size = 0;
    assert(kf::data_file_read_into("E0.", data, sizeof data, &size) == kf::FileResult::Ok);
    assert(size == sizeof fixture && std::memcmp(data, fixture, size) == 0);
    kf::DataFile resource{};
    assert(kf::data_file_open(&resource, "../escape") == kf::FileResult::InvalidPath);
    assert(kf::save_storage_start(path.c_str()));
    using Result = kf::SaveFileResult;
    using Slot = kf::SaveSlot;
    assert(kf::save_file_read(Slot::First, data, sizeof data, &size) == Result::Missing);
    for (const auto slot : {Slot::First, Slot::Second, Slot::Third}) {
        assert(kf::save_file_write(slot, fixture, sizeof fixture) == Result::Ok);
        assert(kf::save_file_write(slot, fixture, sizeof fixture - 1) == Result::Ok);
        assert(kf::save_file_read(slot, data, sizeof data, &size) == Result::Ok);
        assert(size == sizeof fixture - 1 && std::memcmp(data, fixture, size) == 0);
    }
    const auto slot = kf::windows_path((path + "/slot1.kfs").c_str());
    file = kf::windows_open(slot.c_str(), false);
    assert(file != INVALID_HANDLE_VALUE);
    assert(kf::save_file_write(Slot::First, fixture, sizeof fixture) == Result::IoError);
    assert(CloseHandle(file));
    assert(kf::save_file_read(Slot::First, data, sizeof data, &size) == Result::Ok && size == sizeof fixture - 1);
    for (const auto &entry : std::filesystem::directory_iterator(root))
        assert(entry.path().extension() != L".tmp");
    assert(kf::save_file_write(static_cast<Slot>(99), fixture, sizeof fixture) == Result::Invalid);
    kf::save_storage_shutdown();
    assert(kf::save_file_write(Slot::First, fixture, sizeof fixture) == Result::Unavailable);
    std::error_code error;
    std::filesystem::remove_all(temporary, error);
    assert(!error && !std::filesystem::exists(temporary));

    assert(SDL_Init(SDL_INIT_VIDEO));
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    auto *window = SDL_CreateWindow("King's Field Windows regression", 320, 240, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    assert(window);
    auto context = SDL_GL_CreateContext(window);
    assert(context);
    auto *renderer = new kf::Renderer{};
    char message[2048]{};
    const bool initialized = kf::renderer_init(renderer, message, sizeof message);
    if (!initialized)
        std::fprintf(stderr, "%s\n", message);
    assert(initialized);
    kf::renderer_present_retained(renderer, 320, 240);
    assert(SDL_GL_SwapWindow(window));
    kf::renderer_release(renderer);
    delete renderer;
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::puts("Windows Unicode/trailing-dot I/O, save replacement/failure cleanup and GL 3.3 smoke checks passed.");
    return 0;
}
