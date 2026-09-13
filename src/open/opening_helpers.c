#include <kf/address.h>
#include <kf/game_math.h>
#include <kf/open_opening_helpers.h>
#include <psyq/pad.h>

DATA(0x80043178, 0x4)
KfOpeningInputAction opening_input_action;

ADDRESS(0x80013c70, 0x3c)
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

#include "../shared/angle_shortest_delta.inc"
