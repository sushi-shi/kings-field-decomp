void lighting_set_active_color_matrix(KfActiveColorPreset preset)
{
    SetColorMatrix(&color_matrix_table[((s32)(preset))]);
}
