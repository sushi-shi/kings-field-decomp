#include <kf/lib/graphics.h>
#ifndef KF_OPEN
#include <kf/game/state.h>
#endif

void display_begin_frame(void)
{
    graphics_runtime().display_state.buffer_index = display_next_buffer(graphics_runtime().display_state.buffer_index);
    kf::host_begin_frame();
#ifdef KF_OPEN
    graphics_runtime().DAT_8006e044 = 0;
    graphics_runtime().DAT_8006e040 = 0;
#else
    DAT_800a0768 = 0;
    graphics_runtime().DAT_8009569c = 0;
    graphics_runtime().DAT_80095698 = 0;
#endif
}

void display_present_frame(void)
{
    kf::host_wait_frame();
    kf::host_present_frame(graphics_runtime().display_state.frame_style);
}

void lighting_set_active_color_matrix(KfActiveColorPreset preset)
{
    memcpy(graphics_runtime().render_state.lighting.color_matrix.m, (color_matrix_table[kf_enum_encode<s32>(preset)]).m,
        sizeof graphics_runtime().render_state.lighting.color_matrix.m);
}
