s16 angle_shortest_delta(s32 first, s32 second)
{
    s16 difference;

    first &= KF_ANGLE_WRAP_MASK;
    second &= KF_ANGLE_WRAP_MASK;
    difference = second - first;
    if (difference >= KF_ANGLE_HALF_TURN) {
        return difference - KF_ANGLE_FULL_TURN;
    }
    if (difference <= -KF_ANGLE_HALF_TURN) {
        return difference + KF_ANGLE_FULL_TURN;
    }
    return difference;
}
