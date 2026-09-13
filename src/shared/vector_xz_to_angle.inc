ADDRESS_AT("GAME", 0x80014fb8, 0xb0)
ADDRESS_AT("OPEN", 0x80015ce4, 0xb0)
s32 vector_xz_to_angle(s32 x, s32 z)
{
    if (z > 0) {
        return catan((x << KF_FIXED12_BITS) / z) + KF_ANGLE_HALF_TURN;
    }
    if (z < 0) {
        return catan((x << KF_FIXED12_BITS) / z) & KF_ANGLE_WRAP_MASK;
    }
    if (x > 0) {
        return KF_ANGLE_THREE_QUARTER_TURN;
    }
    return KF_ANGLE_QUARTER_TURN;
}
