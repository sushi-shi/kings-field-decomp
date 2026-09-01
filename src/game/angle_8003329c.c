#include <kf/address.h>
#include <kf/game_types.h>

ADDRESS(0x8003329c, 0x48)
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
