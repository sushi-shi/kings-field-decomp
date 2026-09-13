ADDRESS_AT("GAME", 0x80014e48, 0x5c)
ADDRESS_AT("OPEN", 0x80015be0, 0x5c)
void vector3s_scale_shift12(s16 scale, SVECTOR *vector)
{
    s32 x = vector->vx * scale;
    s32 y = vector->vy * scale;
    s32 z = vector->vz * scale;

    setVector(vector, x >> KF_FIXED12_BITS, y >> KF_FIXED12_BITS, z >> KF_FIXED12_BITS);
}
