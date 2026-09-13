#include <kf/game_math.h>
#include <kf/open_opening_helpers.h>
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

s16 angle_shortest_delta(s32 first, s32 second)
{
    s32 difference;
    s16 signed_difference;

    first &= KF_ANGLE_WRAP_MASK;
    second &= KF_ANGLE_WRAP_MASK;
    difference = second - first;
    signed_difference = difference;
    if (signed_difference >= KF_ANGLE_HALF_TURN) {
        return difference - KF_ANGLE_FULL_TURN;
    }
    if (signed_difference < -KF_ANGLE_HALF_TURN + 1) {
        return difference + KF_ANGLE_FULL_TURN;
    }
    return signed_difference;
}
