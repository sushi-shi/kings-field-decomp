#include "../src/platform/host.cpp"
#include "../src/platform/controls.cpp"

#include <cassert>

static bool russian_layout = true;

// Substitute the OS layout lookup; exercise the actual host event handler.
extern "C" SDL_Keycode SDL_GetKeyFromScancode(SDL_Scancode code, SDL_Keymod, bool)
{
    switch (code) {
    case SDL_SCANCODE_W: return russian_layout ? U'ц' : SDLK_W;
    case SDL_SCANCODE_A: return russian_layout ? U'ф' : SDLK_A;
    case SDL_SCANCODE_S: return russian_layout ? U'ы' : SDLK_S;
    case SDL_SCANCODE_D: return russian_layout ? U'в' : SDLK_D;
    case SDL_SCANCODE_E: return russian_layout ? U'у' : SDLK_E;
    case SDL_SCANCODE_Q: return russian_layout ? U'й' : SDLK_Q;
    case SDL_SCANCODE_R: return russian_layout ? U'к' : SDLK_R;
    case SDL_SCANCODE_P: return russian_layout ? U'з' : SDLK_P;
    case SDL_SCANCODE_UP: return SDLK_UP;
    case SDL_SCANCODE_ESCAPE:
    case SDL_SCANCODE_CAPSLOCK: return SDLK_ESCAPE;
    default: return SDLK_UNKNOWN;
    }
}

static kf::InputFrame key(SDL_Scancode code, bool down, bool repeat = false)
{
    SDL_KeyboardEvent event{};
    event.scancode = code;
    event.key = SDL_GetKeyFromScancode(code, SDL_KMOD_NONE, false);
    event.mod = SDL_KMOD_SHIFT | SDL_KMOD_CAPS;
    event.down = down;
    event.repeat = repeat;
    kf::process_keyboard_event(event);
    return kf::input_take(&kf::host.input);
}

int main()
{
    using kf::Action;
    kf::bind_inputs();
    struct Shortcut { SDL_Scancode key; Action action; };
    for (const auto shortcut : {
            Shortcut{SDL_SCANCODE_W, Action::forward},
            Shortcut{SDL_SCANCODE_A, Action::strafe_left},
            Shortcut{SDL_SCANCODE_S, Action::backward},
            Shortcut{SDL_SCANCODE_D, Action::strafe_right},
            Shortcut{SDL_SCANCODE_E, Action::interact},
            Shortcut{SDL_SCANCODE_Q, Action::magic},
            Shortcut{SDL_SCANCODE_R, Action::compare_language},
            Shortcut{SDL_SCANCODE_P, Action::pause}}) {
        for (bool russian : {false, true}) {
            russian_layout = russian;
            const auto bit = kf::action_bit(shortcut.action);
            const auto pressed = key(shortcut.key, true);
            assert(pressed.held == bit && pressed.pressed == bit);
            assert(key(shortcut.key, true, true).pressed == 0);
            russian_layout = !russian;
            const auto released = key(shortcut.key, false);
            assert(released.held == 0 && released.released == bit);
        }
    }

    key(SDL_SCANCODE_W, true);
    key(SDL_SCANCODE_UP, true);
    assert(key(SDL_SCANCODE_W, false).held == kf::action_bit(Action::forward));
    assert(key(SDL_SCANCODE_UP, false).held == 0);

    const auto escape = kf::action_bit(Action::pause_or_back);
    assert(key(SDL_SCANCODE_CAPSLOCK, true).held == escape);
    key(SDL_SCANCODE_ESCAPE, true);
    assert(key(SDL_SCANCODE_CAPSLOCK, false).held == escape);
    assert(key(SDL_SCANCODE_ESCAPE, false).held == 0);
}
