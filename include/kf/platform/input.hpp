#pragma once
#include <kf/lib/types.h>

namespace kf {
enum class Button : u32 {
    Up = 1u << 12, Down = 1u << 14, Left = 1u << 15, Right = 1u << 13,
    Attack = 1u << 4, Back = 1u << 6, Magic = 1u << 7, Confirm = 1u << 5,
    Start = 1u << 8, Select = 1u << 11,
    StrafeRight = 1u << 3, StrafeLeft = 1u << 2,
    LookDown = 1u << 1, LookUp = 1u << 0
};
constexpr u32 button_mask(Button button) { return static_cast<u32>(button); }
constexpr u32 operator&(u32 buttons, Button button) { return buttons & button_mask(button); }
constexpr u32 operator|(Button a, Button b) { return button_mask(a) | button_mask(b); }

constexpr bool button_pressed(u32 current, u32 previous, Button button)
{
    return (current & button) != 0 && (previous & button) == 0;
}

enum class InputContext : u8 { Opening, Gameplay, Menu, Scripted };
struct LookDelta { s32 yaw, pitch; };

u32 host_read_buttons();
void host_wait_buttons_released(u32 mask = ~u32(0));
void host_wait_button_press();
InputContext host_set_input_context(InputContext context);
LookDelta host_take_look();
}
