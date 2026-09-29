#include <kf/lib/math.h>
#include <kf/open/opening_helpers.h>
#include <psyq/pad.h>

KfOpeningInputAction opening_input_action;

void opening_poll_input(void)
{
    u32 input = PadRead(1);

    if (input != 0) {
        if ((input & PADk) != 0) {
            opening_input_action = KF_OPENING_INPUT_SKIP;
        } else {
            opening_input_action = KF_OPENING_INPUT_ADVANCE;
        }
    }
}

#include "../lib/angle_shortest_delta.inc"
