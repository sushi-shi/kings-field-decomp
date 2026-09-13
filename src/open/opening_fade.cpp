#include <kf/open_render.h>

KfScreenRect opening_fade_rect = {
    KF_TRANSITION_RECT_X, 0, KF_TRANSITION_RECT_WIDTH, KF_TRANSITION_RECT_HEIGHT
};

u8 opening_fade_uv[KF_QUAD_TEX_DESCRIPTOR_BYTES] = {
    0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0
};

CVECTOR opening_fade_color = {0, 0, 0, 0};

void opening_fade_in(void)
{
    CVECTOR color = opening_fade_color;
    int tpage;
    int clut;
    s32 frame;

    tpage = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        KF_TRANSITION_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
    clut = GetClut(0, KF_TRANSITION_CLUT_Y);
    frame = 0;
    do {
        display_begin_frame();
        TRANSITION_COLOR_STEP(&color);
        sprite_add_ft4(
            &opening_fade_rect, opening_fade_uv, tpage, clut, &color, KF_TRANSITION_OT_DEPTH);
        display_present_frame();
        frame++;
    } while (frame < KF_TRANSITION_FADE_FRAMES);
    DrawSync(0);
}
