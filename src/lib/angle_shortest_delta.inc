ADDRESS_AT("GAME", 0x8003329c, 0x48)
ADDRESS_AT("OPEN", 0x80013cac, 0x48)
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
