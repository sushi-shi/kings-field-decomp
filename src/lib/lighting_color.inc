void lighting_set_active_color_matrix(KfActiveColorPreset preset)
{
    memcpy(KF_GRAPHICS_RUNTIME.render_state.lighting.color_matrix.m, (color_matrix_table[kf_enum_encode<s32>(preset)]).m,
        sizeof KF_GRAPHICS_RUNTIME.render_state.lighting.color_matrix.m);
}
