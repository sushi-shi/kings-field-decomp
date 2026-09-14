ADDRESS_AT("GAME", 0x8001bab8, 0x2c)
ADDRESS_AT("OPEN", 0x800168dc, 0x2c)
void lighting_set_active_color_matrix(KfActiveColorPreset preset)
{
    SetColorMatrix(&color_matrix_table[KF_ENUM_ENCODE(s32, preset)]);
}
