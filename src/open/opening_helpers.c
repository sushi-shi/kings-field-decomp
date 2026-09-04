#include <kf/address.h>
#include <kf/open_opening_helpers.h>
#include <kf/psyq_pad.h>

DATA(0x80043178, 0x4)
u32 opening_input_action;

ADDRESS(0x80013c70, 0x3c)
void opening_poll_input(void)
{
    u32 input = PadRead(1);

    if (input != 0) {
        if ((input & 0x100) != 0) {
            opening_input_action = 2;
        } else {
            opening_input_action = 1;
        }
    }
}

ADDRESS(0x80013cac, 0x48)
s16 angle_shortest_delta(s32 first, s32 second)
{
    s32 difference;
    s16 signed_difference;

    first &= 0xfff;
    second &= 0xfff;
    difference = second - first;
    signed_difference = difference;
    if (signed_difference >= 2048) {
        return difference - 4096;
    }
    if (signed_difference < -2047) {
        return difference + 4096;
    }
    return signed_difference;
}
