#include <kf/lib/graphics.h>
#ifndef KF_OPEN
#include <kf/game/state.h>
#endif

void display_begin_frame(void)
{
    KF_GRAPHICS_RUNTIME.display_state.buffer_index = display_next_buffer(KF_GRAPHICS_RUNTIME.display_state.buffer_index);
    kf::host_begin_frame();
#ifdef KF_OPEN
    KF_GRAPHICS_RUNTIME.DAT_8006e044 = 0;
    KF_GRAPHICS_RUNTIME.DAT_8006e040 = 0;
#else
    DAT_800a0768 = 0;
    KF_GRAPHICS_RUNTIME.DAT_8009569c = 0;
    KF_GRAPHICS_RUNTIME.DAT_80095698 = 0;
#endif
}

void display_present_frame(void)
{
    kf::host_wait_frame();
    kf::host_present_frame(KF_GRAPHICS_RUNTIME.display_state.frame_style);
}

void lighting_set_active_color_matrix(KfActiveColorPreset preset)
{
    memcpy(KF_GRAPHICS_RUNTIME.render_state.lighting.color_matrix.m, (color_matrix_table[kf_enum_encode<s32>(preset)]).m,
        sizeof KF_GRAPHICS_RUNTIME.render_state.lighting.color_matrix.m);
}
