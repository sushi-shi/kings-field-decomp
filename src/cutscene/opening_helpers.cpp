#include <kf/platform/prelude.h>
#include <kf/cutscene/opening_helpers.h>
#include <kf/lib/math.h>
#include <kf/platform/input.h>

KfOpeningInputAction opening_input_action;

void opening_poll_input(void)
{
    u32 input = kf::host_read_buttons();

    if (input != 0) {
        if ((input & kf::Button::Start) != 0) {
            opening_input_action = KF_OPENING_INPUT_SKIP;
        } else {
            opening_input_action = KF_OPENING_INPUT_ADVANCE;
        }
    }
}

void opening_helpers_reset_module_state(void)
{
    kf::restore_initial_value<opening_input_action>();
}
