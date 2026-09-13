#include <kf/game_graphics.h>

#include <kf/game_render.h>
#include <kf/game.h>

typedef struct {
    u8 v[KF_QUAD_TEX_DESCRIPTOR_BYTES];
} FadeUv;

KfScreenRect fade_screen_rect = {
    KF_TRANSITION_RECT_X, 0, KF_TRANSITION_RECT_WIDTH, KF_TRANSITION_RECT_HEIGHT
};

FadeUv fade_screen_uv = {{
    0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0
}};

CVECTOR fade_screen_color = {0, 0, 0, 0};

char fade_screen_path[7] = "B0\\L0.";

void display_play_transition(void)
{
    KfScreenRect rect = fade_screen_rect;
    FadeUv uv = fade_screen_uv;
    CVECTOR color = fade_screen_color;
    int tpage;
    int clut;
    s32 i;

    DrawSync(0);
    PutDrawEnv(&game_graphics_runtime.display_draw_environments[0]);
    PutDrawEnv(&game_graphics_runtime.display_draw_environments[1]);
    DrawSync(0);

    if (cd_file_load_into((void *)game_graphics_runtime.display_state.asset_load_buffer, fade_screen_path) != KF_RESOURCE_LOADED) {
        return;
    }
    tim_upload_images(game_graphics_runtime.display_state.asset_load_buffer);

    tpage = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        KF_TRANSITION_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
    clut = GetClut(0, KF_TRANSITION_CLUT_Y);

    for (i = 0; i < KF_TRANSITION_FADE_FRAMES; i++) {
        display_begin_frame();
        TRANSITION_COLOR_STEP(&color);
        sprite_add_ft4(&rect, uv.v, tpage, clut, &color, KF_TRANSITION_OT_DEPTH);
        display_present_frame();
    }
    DrawSync(0);
}
