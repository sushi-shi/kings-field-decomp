#include "../src/game/save_system.cpp"

#include <kf/platform/disc.h>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <vector>

KfGraphicsRuntimeGame game_graphics_runtime;

namespace {
struct Frame { bool compare; u32 buttons; };
constexpr auto confirm = kf::button_mask(kf::Button::Confirm);
constexpr std::array frames = {
    Frame{false, confirm}, Frame{false, 0}, Frame{true, 0},
    Frame{true, confirm}, Frame{false, confirm}, Frame{false, 0},
    Frame{true, 0}, Frame{false, 0}, Frame{false, confirm}
};
std::size_t frame;
bool available;
u8 displayed;
kf::Language selected;
kf::InputContext context;
std::filesystem::path english;
std::vector<u8> uploads;
u32 unavailable_messages;
u32 restorations;
}

namespace kf {
Renderer *host_renderer() { return nullptr; }
bool renderer_capture_frame(const Renderer *, Image &image) { image.rgba = {'B'}; return true; }
bool renderer_restore_frame(const Renderer *, const Image &image)
{
    assert(image.rgba == ByteBuffer{'B'});
    ++restorations;
    return true;
}
[[noreturn]] void host_fail(const char *) { std::abort(); }
bool translation_available() { return available; }
bool disc_verify_directory(const char *, Language, Language *) { return true; }
bool disc_prepare_directory(const char *, const char *destination, Language language)
{
    assert(language == Language::English);
    std::filesystem::copy(english, destination, std::filesystem::copy_options::recursive);
    return true;
}
void host_language_status(const char *message)
{
    if (std::strstr(message, "unavailable for this page"))
        ++unavailable_messages;
}
u32 host_read_buttons() { return frames.at(frame).buttons; }
bool host_action_held(Action action)
{
    assert(action == Action::compare_language);
    return frames.at(frame).compare;
}
InputContext host_set_input_context(InputContext next)
{
    const auto previous = context;
    context = next;
    return previous;
}
}

kf::FrameTask<void> game_wait_frame()
{
    co_await kf::FrameDelay{1};
    ++frame;
}

kf::FrameTask<void> game_wait_buttons_released(u32)
{
    assert(frame == frames.size());
    co_return;
}

void tim_upload_images(const u8 *data, std::size_t size)
{
    assert(size == 1);
    displayed = *data;
    uploads.push_back(displayed);
}
void display_present_system_screen(s32)
{
    assert(context == kf::InputContext::Menu);
    assert(kf::game_language() == selected);
    const auto base = selected == kf::Language::Japanese ? 'J' : 'E';
    const auto other = selected == kf::Language::Japanese ? 'E' : 'J';
    assert(displayed == (available && frames.at(frame).compare ? other : base));
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    const auto japanese = std::filesystem::path(argv[1]) / "ja";
    english = std::filesystem::path(argv[1]) / "en";
    for (const auto &root : {japanese, english}) {
        std::filesystem::create_directories(root / "KF/TALK");
        std::ofstream(root / "KF/TALK/PAGE.TIM") << (root == japanese ? 'J' : 'E');
    }
    std::array<u8, 16> buffer;
    game_graphics_runtime.display_state.asset_load_buffer = buffer.data();
    game_graphics_runtime.display_state.asset_load_capacity = buffer.size();
    for (const auto language : {kf::Language::Japanese, kf::Language::English}) {
        for (const bool has_alternate : {true, false}) {
            selected = language;
            available = has_alternate;
            frame = 0;
            uploads.clear();
            unavailable_messages = 0;
            restorations = 0;
            context = kf::InputContext::Scripted;
            const auto &root = language == kf::Language::Japanese ? japanese : english;
            assert(kf::language_resources_start(root.c_str(), language,
                available ? japanese.c_str() : nullptr));
            const auto other = language == kf::Language::Japanese
                ? kf::Language::English : kf::Language::Japanese;
            if (available)
                assert(kf::language_request(other));
            auto page = screen_show_image_until_input("TALK/PAGE.TIM");
            unsigned advances = 0;
            while (!page.done()) {
                assert(++advances <= frames.size() + 1);
                page.advance();
            }
            assert(frame == frames.size() && context == kf::InputContext::Scripted);
            assert(kf::game_language() == language);
            assert(kf::language_requested() == (available ? other : language));
            assert(uploads.size() == (available ? 5u : 1u));
            assert(restorations == (available ? 4u : 0u));
            assert(unavailable_messages == (available ? 0u : 1u));
            u8 active;
            assert(kf::data_file_read_into("KF/TALK/PAGE.TIM", &active, 1) == kf::FileResult::Ok);
            assert(active == (language == kf::Language::Japanese ? 'J' : 'E'));
            kf::DataFile file{};
            assert(kf::data_file_open_at(&file, root.c_str(), "../en/KF/TALK/PAGE.TIM") ==
                kf::FileResult::InvalidPath);
            kf::language_resources_stop();
        }
    }
}
