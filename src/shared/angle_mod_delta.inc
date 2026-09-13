ADDRESS_AT("GAME", 0x80014fa8, 0x10)
ADDRESS_AT("OPEN", 0x80015cd4, 0x10)
KfBool angle_mod_delta_le_half_turn(int lhs, int rhs)
{
    return ((lhs - rhs) & KF_ANGLE_WRAP_MASK) < (KF_ANGLE_HALF_TURN + 1);
}
