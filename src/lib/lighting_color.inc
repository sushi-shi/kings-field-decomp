void lighting_set_active_color_matrix(KfActiveColorPreset preset)
{
    SetColorMatrix(&color_matrix_table[kf_enum_encode<s32>(preset)]);
}
