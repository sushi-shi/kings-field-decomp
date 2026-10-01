#include <kf/audio/sound.h>
#include <kf/platform/controls.h>
#include <kf/platform/host.h>
#include <kf/platform/input.h>
#include <kf/platform/language_runtime.h>
#include <kf/platform/saves.h>
#include <kf/renderer/renderer.h>

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>

EM_JS(void, host_browser_status, (const char *message), {
    document.getElementById('status').textContent = UTF8ToString(message);
});
EM_JS(int, host_browser_mouse_captured, (), {
    return document.pointerLockElement === Module.canvas;
});
EM_JS(void, host_browser_language, (const char *current, const char *requested, const char *message), {
    Module['languageChanged'](UTF8ToString(current), UTF8ToString(requested), UTF8ToString(message));
});
#endif

namespace kf {
struct HostState {
    SDL_Window *window;
    SDL_GLContext context;
    SDL_Gamepad *gamepad;
    Renderer renderer;
    std::vector<DrawFace> frame_faces;
    InputState input;
    InputContext input_context;
    std::array<SDL_Keycode, SDL_SCANCODE_COUNT> pressed_keys;
    Uint64 epoch, paused_ns, pause_start;
    double look_remainder_x, look_remainder_y;
    bool focused, mouse_captured, resume_mouse_capture;
};
static HostState host;
static constexpr Uint64 ns_per_second = 1000000000;
static constexpr Uint64 ns_per_millisecond = 1000000;
static constexpr Uint64 clock_ticks_per_second = 60;
static constexpr Uint64 unfocused_poll_interval_ns = 16 * ns_per_millisecond;
static constexpr Uint64 maximum_wait_slice_ns = 8 * ns_per_millisecond;
static constexpr u32 gamepad_axis_control_first = 100;
static constexpr u32 gamepad_axis_positive_first = gamepad_axis_control_first + 1;
static constexpr int gamepad_axis_deadzone = 16000;
static constexpr std::size_t renderer_error_capacity = 2048;
static constexpr double mouse_degrees_per_pixel = 0.12;
static constexpr double angle_units_per_turn = 4096;
static constexpr double degrees_per_turn = 360;

static void present_retained_frame() {
    int width = 0, height = 0;
    SDL_GetWindowSizeInPixels(host.window, &width, &height);
    if (width > 0 && height > 0) {
        renderer_present_retained(&host.renderer, width, height);
        SDL_GL_SwapWindow(host.window);
    }
}

static void platform_yield(Uint64 nanoseconds) {
#ifdef __EMSCRIPTEN__
    emscripten_sleep(static_cast<unsigned>((nanoseconds + ns_per_millisecond - 1) / ns_per_millisecond));
#else
    SDL_DelayNS(nanoseconds);
#endif
}

static void bind_inputs() {
    auto *input = &host.input;
    struct KeyBinding { SDL_Keycode code; Action action; };
    constexpr std::array<KeyBinding, 19> keys = {
        KeyBinding{SDLK_W, Action::forward}, KeyBinding{SDLK_UP, Action::forward},
        KeyBinding{SDLK_S, Action::backward}, KeyBinding{SDLK_DOWN, Action::backward},
        KeyBinding{SDLK_A, Action::strafe_left}, KeyBinding{SDLK_D, Action::strafe_right},
        KeyBinding{SDLK_LEFT, Action::turn_left}, KeyBinding{SDLK_RIGHT, Action::turn_right},
        KeyBinding{SDLK_PAGEUP, Action::look_up}, KeyBinding{SDLK_PAGEDOWN, Action::look_down},
        KeyBinding{SDLK_E, Action::interact}, KeyBinding{SDLK_RETURN, Action::confirm},
        KeyBinding{SDLK_SPACE, Action::attack}, KeyBinding{SDLK_Q, Action::magic},
        KeyBinding{SDLK_TAB, Action::inventory}, KeyBinding{SDLK_BACKSPACE, Action::back},
        KeyBinding{SDLK_ESCAPE, Action::pause_or_back}, KeyBinding{SDLK_P, Action::pause},
        KeyBinding{SDLK_R, Action::compare_language}
    };
    for (const auto &key : keys)
        input_bind(input, {InputDevice::keyboard, static_cast<u32>(key.code)}, key.action);
    input_bind(input, {InputDevice::mouse, SDL_BUTTON_LEFT}, Action::attack);
    input_bind(input, {InputDevice::mouse, SDL_BUTTON_RIGHT}, Action::magic);
    struct ButtonBinding { SDL_GamepadButton code; Action action; };
    constexpr std::array<ButtonBinding, 13> buttons = {
        ButtonBinding{SDL_GAMEPAD_BUTTON_DPAD_UP, Action::forward}, ButtonBinding{SDL_GAMEPAD_BUTTON_DPAD_DOWN, Action::backward},
        ButtonBinding{SDL_GAMEPAD_BUTTON_DPAD_LEFT, Action::turn_left}, ButtonBinding{SDL_GAMEPAD_BUTTON_DPAD_RIGHT, Action::turn_right},
        ButtonBinding{SDL_GAMEPAD_BUTTON_NORTH, Action::attack}, ButtonBinding{SDL_GAMEPAD_BUTTON_WEST, Action::magic},
        ButtonBinding{SDL_GAMEPAD_BUTTON_EAST, Action::interact}, ButtonBinding{SDL_GAMEPAD_BUTTON_SOUTH, Action::back},
        ButtonBinding{SDL_GAMEPAD_BUTTON_START, Action::inventory}, ButtonBinding{SDL_GAMEPAD_BUTTON_BACK, Action::pause},
        ButtonBinding{SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, Action::strafe_left},
        ButtonBinding{SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, Action::strafe_right},
        ButtonBinding{SDL_GAMEPAD_BUTTON_RIGHT_STICK, Action::compare_language}
    };
    for (const auto &button : buttons)
        input_bind(input, {InputDevice::gamepad, static_cast<u32>(button.code)}, button.action);
    // Axis directions use distinct controls so releasing a stick cannot release a held D-pad.
    input_bind(input, {InputDevice::gamepad, gamepad_axis_control_first + SDL_GAMEPAD_AXIS_LEFTX * 2}, Action::turn_left);
    input_bind(input, {InputDevice::gamepad, gamepad_axis_positive_first + SDL_GAMEPAD_AXIS_LEFTX * 2}, Action::turn_right);
    input_bind(input, {InputDevice::gamepad, gamepad_axis_control_first + SDL_GAMEPAD_AXIS_LEFTY * 2}, Action::forward);
    input_bind(input, {InputDevice::gamepad, gamepad_axis_positive_first + SDL_GAMEPAD_AXIS_LEFTY * 2}, Action::backward);
    input_bind(input, {InputDevice::gamepad, gamepad_axis_positive_first + SDL_GAMEPAD_AXIS_LEFT_TRIGGER * 2}, Action::look_up);
    input_bind(input, {InputDevice::gamepad, gamepad_axis_positive_first + SDL_GAMEPAD_AXIS_RIGHT_TRIGGER * 2}, Action::look_down);
}

static void open_available_gamepad() {
    int count = 0;
    auto *ids = SDL_GetGamepads(&count);
    for (int i = 0; i < count && !host.gamepad; ++i)
        host.gamepad = SDL_OpenGamepad(ids[i]);
    SDL_free(ids);
}

bool host_start() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
        std::fprintf(stderr, "%s\n", SDL_GetError());
        return false;
    }
#ifdef _WIN32
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
    host.window = SDL_CreateWindow("King's Field", 960, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (host.window)
        host.context = SDL_GL_CreateContext(host.window);
    if (!host.window || !host.context) {
        std::fprintf(stderr, "%s\n", SDL_GetError());
        host_shutdown();
        return false;
    }
    // All frame pacing uses the absolute clock below, not a second swap-interval wait.
    SDL_GL_SetSwapInterval(0);
    std::array<char, renderer_error_capacity> error{};
    if (!renderer_init(&host.renderer, error.data(), error.size())) {
        std::fprintf(stderr, "%s\n", error.data());
        host_shutdown();
        return false;
    }
    bind_inputs();
    open_available_gamepad();
    host.epoch = SDL_GetTicksNS();
    host.focused = true;
    if (!sound_start()) {
        host_shutdown();
        return false;
    }
    return true;
}

void host_shutdown() {
    language_resources_stop();
    sound_shutdown();
    save_storage_shutdown();
    if (host.gamepad)
        SDL_CloseGamepad(host.gamepad);
    if (host.context) {
        renderer_release(&host.renderer);
        SDL_GL_DestroyContext(host.context);
    }
    if (host.window)
        SDL_DestroyWindow(host.window);
    host = {};
    SDL_Quit();
}

void host_language_status(const char *message) {
    std::printf("%s (%s)\n", message, language_name(game_language()));
    std::fflush(stdout);
#ifdef __EMSCRIPTEN__
    host_browser_language(language_code(game_language()), language_code(language_requested()), message);
#endif
}

[[noreturn]] void host_fail(const char *message) {
    std::fprintf(stderr, "%s\n", message);
    std::fflush(stderr);
#ifdef __EMSCRIPTEN__
    host_browser_status(message);
#else
    if (host.window) {
        SDL_SetWindowRelativeMouseMode(host.window, false);
        sound_set_paused(true);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "King's Field — Error", message, host.window);
    }
#endif
    host_shutdown();
    std::exit(1);
}

static bool mouse_is_captured() {
    if (!host.window || !SDL_GetWindowRelativeMouseMode(host.window))
        return false;
#ifdef __EMSCRIPTEN__
    return host_browser_mouse_captured() != 0;
#else
    return true;
#endif
}

static void refresh_mouse_capture() {
    const bool captured = mouse_is_captured();
    if (!captured && host.mouse_captured) {
        if (host.input_context == InputContext::Gameplay || host.input_context == InputContext::Scripted)
            host.resume_mouse_capture = false;
        input_disconnect(&host.input, InputDevice::mouse);
        host.input.pending.look_x = host.input.pending.look_y = 0;
        host.look_remainder_x = host.look_remainder_y = 0;
    }
    host.mouse_captured = captured;
}

static void process_keyboard_event(const SDL_KeyboardEvent &event) {
    if (event.repeat || event.scancode <= SDL_SCANCODE_UNKNOWN || event.scancode >= SDL_SCANCODE_COUNT)
        return;
    auto &pressed_key = host.pressed_keys[event.scancode];
    if (event.down) {
        if (pressed_key != SDLK_UNKNOWN)
            return;
        // Letter shortcuts follow physical QWERTY positions in every layout.
        // Other keys retain logical remaps, including Caps Lock to Escape.
        if (event.scancode >= SDL_SCANCODE_A && event.scancode <= SDL_SCANCODE_Z)
            pressed_key = SDLK_A + static_cast<SDL_Keycode>(event.scancode - SDL_SCANCODE_A);
        else
            pressed_key = SDL_GetKeyFromScancode(event.scancode, SDL_KMOD_NONE, false);
        if (pressed_key != SDLK_UNKNOWN)
            input_button(&host.input, {InputDevice::keyboard, pressed_key}, true);
    } else {
        // Remember the key-down mapping even if the layout changes while held.
        const auto released_key = pressed_key;
        pressed_key = SDLK_UNKNOWN;
        if (released_key == SDLK_UNKNOWN)
            return;
        // Two physical keys can both map to Escape; releasing one must not
        // release the logical action while the other is still down.
        for (const auto held_key : host.pressed_keys)
            if (held_key == released_key)
                return;
        input_button(&host.input, {InputDevice::keyboard, released_key}, false);
    }
}

static void process_event(const SDL_Event &event) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        host_shutdown();
        std::exit(0);
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        if (host.focused)
            process_keyboard_event(event.key);
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (!host.focused)
            break;
        if (event.button.down && host.input_context == InputContext::Gameplay &&
                (!host.mouse_captured || !mouse_is_captured())) {
            SDL_SetWindowRelativeMouseMode(host.window, true);
            break;
        }
        input_button(&host.input, {InputDevice::mouse, event.button.button}, event.button.down);
        break;
    case SDL_EVENT_MOUSE_MOTION:
        if (host.focused && host.input_context == InputContext::Gameplay &&
                mouse_is_captured()) {
            double x = event.motion.xrel, y = event.motion.yrel;
#ifdef __EMSCRIPTEN__
            double css_width = 0, css_height = 0;
            int width = 0, height = 0;
            SDL_GetWindowSize(host.window, &width, &height);
            if (width > 0 && height > 0 && emscripten_get_element_css_size("#canvas", &css_width, &css_height) == EMSCRIPTEN_RESULT_SUCCESS) {
                x *= css_width / width;
                y *= css_height / height;
            }
#endif
            input_motion(&host.input, x, y);
        }
        break;
    case SDL_EVENT_GAMEPAD_ADDED:
        if (!host.gamepad)
            host.gamepad = SDL_OpenGamepad(event.gdevice.which);
        break;
    case SDL_EVENT_GAMEPAD_REMOVED:
        if (host.gamepad && event.gdevice.which == SDL_GetGamepadID(host.gamepad)) {
            SDL_CloseGamepad(host.gamepad);
            host.gamepad = nullptr;
            input_disconnect(&host.input, InputDevice::gamepad);
            open_available_gamepad();
        }
        break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
        if (host.focused && host.gamepad && event.gbutton.which == SDL_GetGamepadID(host.gamepad))
            input_button(&host.input, {InputDevice::gamepad, event.gbutton.button}, event.gbutton.down);
        break;
    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        if (host.focused && host.gamepad && event.gaxis.which == SDL_GetGamepadID(host.gamepad)) {
            input_button(&host.input, {InputDevice::gamepad, gamepad_axis_control_first + event.gaxis.axis * 2u}, event.gaxis.value < -gamepad_axis_deadzone);
            input_button(&host.input, {InputDevice::gamepad, gamepad_axis_positive_first + event.gaxis.axis * 2u}, event.gaxis.value > gamepad_axis_deadzone);
        }
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        if (host.focused) {
            host.pause_start = SDL_GetTicksNS();
            host.focused = false;
            host.resume_mouse_capture = false;
            sound_set_paused(true);
            input_clear(&host.input);
            host.pressed_keys.fill(SDLK_UNKNOWN);
            host.look_remainder_x = host.look_remainder_y = 0;
            SDL_SetWindowRelativeMouseMode(host.window, false);
        }
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        if (!host.focused) {
            host.paused_ns += SDL_GetTicksNS() - host.pause_start;
            host.focused = true;
            sound_set_paused(false);
        }
        break;
    case SDL_EVENT_WINDOW_EXPOSED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
    case SDL_EVENT_WINDOW_RESTORED:
        // Modal waits still service presentation. Never resubmit their blended
        // draw commands here: the offscreen frame already contains the result.
        if (event.window.windowID == SDL_GetWindowID(host.window))
            present_retained_frame();
        break;
    default: break;
    }
}

void host_poll() {
    // Only platform work runs here. In the browser each yield suspends the existing
    // game stack; event callbacks never enter a second gameplay call chain.
    for (;;) {
        SDL_Event event;
        while (SDL_PollEvent(&event))
            process_event(event);
        refresh_mouse_capture();
        sound_poll();
        if (host.focused)
            return;
        platform_yield(unfocused_poll_interval_ns);
    }
}

std::uint64_t host_clock_ns() {
    const auto now = host.focused ? SDL_GetTicksNS() : host.pause_start;
    return now - host.epoch - host.paused_ns;
}

std::uint64_t host_clock_tick() {
    const auto elapsed = host_clock_ns();
    return elapsed / ns_per_second * clock_ticks_per_second + elapsed % ns_per_second * clock_ticks_per_second / ns_per_second;
}

static void wait_until_ns(std::uint64_t target) {
    for (;;) {
        host_poll();
        const auto now = host_clock_ns();
        if (now >= target)
            return;
        platform_yield(std::min<Uint64>(target - now, maximum_wait_slice_ns));
    }
}

void host_wait_until_tick(std::uint64_t deadline) {
    const auto target = deadline / clock_ticks_per_second * ns_per_second +
                        (deadline % clock_ticks_per_second * ns_per_second + clock_ticks_per_second - 1) / clock_ticks_per_second;
    wait_until_ns(target);
}

void host_wait_frame() {
    host_wait_until_tick(host_clock_tick() + 1);
}

UpdatePacer host_begin_update_pacer(u32 steps_per_second) {
    if (steps_per_second == 0 || steps_per_second > ns_per_second)
        host_fail("Invalid update frequency");
    host_poll();
    return {host_clock_ns(), steps_per_second, 0};
}

void host_wait_update(UpdatePacer &pacer) {
    const auto rate = pacer.steps_per_second;
    if (rate == 0 || rate > ns_per_second)
        host_fail("Uninitialized update pacer");
    // Carry fractional nanoseconds; work and presentation consume this interval.
    const auto fraction = pacer.fractional_ns + ns_per_second % rate;
    pacer.deadline_ns += ns_per_second / rate + fraction / rate;
    pacer.fractional_ns = static_cast<u32>(fraction % rate);
    wait_until_ns(pacer.deadline_ns);
    const auto now = host_clock_ns();
    if (now - pacer.deadline_ns >= (ns_per_second + rate - 1) / rate) {
        // A long stall must not cause a burst of catch-up game steps.
        pacer.deadline_ns = now;
        pacer.fractional_ns = 0;
    }
}

bool host_action_held(Action action) {
    host_poll();
    return (host.input.pending.held & action_bit(action)) != 0;
}

u32 host_read_buttons() {
    host_poll();
    // Original game code samples held buttons and derives its own edges.
    const u32 actions = host.input.pending.held;
    host.input.pending.pressed = host.input.pending.released = 0;
    struct ButtonAction { Action action; Button button; };
    constexpr std::array<ButtonAction, 15> mapping = {
        ButtonAction{Action::forward, Button::Up}, ButtonAction{Action::backward, Button::Down},
        ButtonAction{Action::turn_left, Button::Left}, ButtonAction{Action::turn_right, Button::Right},
        ButtonAction{Action::strafe_left, Button::StrafeLeft}, ButtonAction{Action::strafe_right, Button::StrafeRight},
        ButtonAction{Action::look_up, Button::LookUp}, ButtonAction{Action::look_down, Button::LookDown},
        ButtonAction{Action::attack, Button::Attack}, ButtonAction{Action::magic, Button::Magic},
        ButtonAction{Action::interact, Button::Confirm}, ButtonAction{Action::confirm, Button::Confirm},
        ButtonAction{Action::back, Button::Back}, ButtonAction{Action::inventory, Button::Start}, ButtonAction{Action::pause, Button::Select}
    };
    u32 buttons = 0;
    for (const auto &entry : mapping)
        if (actions & action_bit(entry.action))
            buttons |= button_mask(entry.button);
    if (actions & action_bit(Action::pause_or_back))
        buttons |= button_mask(host.input_context == InputContext::Menu ? Button::Back : Button::Select);
    return buttons;
}

void host_wait_buttons_released(u32 mask) {
    while (host_read_buttons() & mask)
        host_wait_frame();
}

void host_wait_button_press() {
    while (!host_read_buttons())
        host_wait_frame();
}

InputContext host_set_input_context(InputContext context) {
    refresh_mouse_capture();
    const auto previous = host.input_context;
    // Nested Scripted -> Menu -> Scripted transitions retain the gameplay
    // capture request until the outer interaction returns to gameplay.
    if (previous == InputContext::Gameplay && context != InputContext::Gameplay)
        host.resume_mouse_capture = mouse_is_captured();
    if (context == InputContext::Opening)
        host.resume_mouse_capture = false;
    host.input_context = context;
    host.input.pending.look_x = host.input.pending.look_y = 0;
    host.look_remainder_x = host.look_remainder_y = 0;
    if ((context == InputContext::Opening || context == InputContext::Menu) && host.window)
        SDL_SetWindowRelativeMouseMode(host.window, false);
    if (context == InputContext::Gameplay && previous != InputContext::Gameplay) {
        if (host.resume_mouse_capture && host.focused && host.window && !mouse_is_captured()) {
            if (!SDL_SetWindowRelativeMouseMode(host.window, true))
                std::fprintf(stderr, "Cannot restore mouse capture: %s\n", SDL_GetError());
        }
        host.resume_mouse_capture = false;
    }
    refresh_mouse_capture();
    return previous;
}

LookDelta host_take_look() {
    constexpr double angle_units_per_pixel = mouse_degrees_per_pixel * angle_units_per_turn / degrees_per_turn;
    const double x = host.input.pending.look_x * angle_units_per_pixel + host.look_remainder_x;
    const double y = host.input.pending.look_y * angle_units_per_pixel + host.look_remainder_y;
    const s32 dx = static_cast<s32>(std::trunc(x));
    const s32 dy = static_cast<s32>(std::trunc(y));
    host.look_remainder_x = x - dx;
    host.look_remainder_y = y - dy;
    host.input.pending.look_x = host.input.pending.look_y = 0;
    return {-dx, dy};
}

Renderer *host_renderer() { return &host.renderer; }

void host_begin_frame() {
    host.frame_faces.clear();
}

void host_enqueue_face(const DrawFace &face) {
    if (host.frame_faces.size() == host.frame_faces.max_size())
        host_fail("Render command allocation is too large.");
    // Animated instances reuse the same projection/morph scratch storage. No
    // pointer into that storage survives this submission.
    try {
        host.frame_faces.push_back(face);
    } catch (const std::bad_alloc &) {
        host_fail("Cannot allocate render commands.");
    }
}

void host_present_frame(const FrameStyle &style) {
    const FaceList faces{host.frame_faces.data(), host.frame_faces.size(), style};
    host_present_faces(&faces);
}

void host_present_faces(const FaceList *faces) {
    int width = 0, height = 0;
    SDL_GetWindowSizeInPixels(host.window, &width, &height);
    // A minimized window can have no drawable pixels. Still update the logical
    // target, so a one-shot modal is there when the window is restored.
    if (!renderer_draw_faces(&host.renderer, faces, width, height))
        host_fail("Cannot submit game render commands.");
    if (width > 0 && height > 0)
        SDL_GL_SwapWindow(host.window);
}
}
