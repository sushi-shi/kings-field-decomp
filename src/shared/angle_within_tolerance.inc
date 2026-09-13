ADDRESS_AT("GAME", 0x80014f6c, 0x3c)
ADDRESS_AT("OPEN", 0x80015c98, 0x3c)
KfBool angle_within_tolerance(int lhs, int rhs, s16 range)
{
    int delta = (lhs - rhs) & KF_ANGLE_WRAP_MASK;

    return delta <= range || KF_ANGLE_FULL_TURN - range <= delta;
}
