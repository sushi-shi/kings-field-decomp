#include <kf/address.h>
#include <kf/game_render.h>
#include <kf/game.h>

/* The texture descriptor keeps byte alignment for retail's unaligned copies. */
typedef struct {
    u8 v[KF_QUAD_TEX_DESCRIPTOR_BYTES];
} FadeUv;

/* Position rect {x, y, w, h}, texture-coordinate rect (even bytes u, v, w, h),
 * and the fade modulation color that ramps up from black. */
DATA(0x80057b14, 0x8)
KfScreenRect fade_screen_rect = {
    KF_TRANSITION_RECT_X, 0, KF_TRANSITION_RECT_WIDTH, KF_TRANSITION_RECT_HEIGHT
};
DATA(0x80057b1c, 0x8)
FadeUv fade_screen_uv = {{
    0, 0, 0, 0, KF_TRANSITION_RECT_WIDTH, 0, KF_TRANSITION_RECT_HEIGHT, 0
}};
DATA(0x80057b24, 0x4)
CVECTOR fade_screen_color = {0, 0, 0, 0};
DATA(0x80057b28, 0x7)
char fade_screen_path[7] = "B0\\L0.";

/* Loads a full-screen image from the disc and fades it in over 18 frames by
 * ramping the modulation color; called from game_main_loop on a transition. */
ADDRESS(0x800144d4, 0x1a0)
void display_play_transition(void)
{
    KfScreenRect rect = fade_screen_rect;
    FadeUv uv = fade_screen_uv;
    CVECTOR color = fade_screen_color;
    int tpage;
    int clut;
    s32 i;

    DrawSync(0);
    PutDrawEnv(&display_draw_environments[0]);
    PutDrawEnv(&display_draw_environments[1]);
    DrawSync(0);

    if (cd_file_load_into(display_state.asset_load_buffer, fade_screen_path)) {
        return;
    }
    tim_upload_images(display_state.asset_load_buffer);

    tpage = GetTPage(
        KF_GPU_TEXTURE_4BIT, KF_GPU_BLEND_AVERAGE,
        KF_TRANSITION_TPAGE_X, KF_TEXTURE_LOWER_PAGE_Y);
    clut = GetClut(0, KF_TRANSITION_CLUT_Y);

    for (i = 0; i < KF_TRANSITION_FADE_FRAMES; i++) {
        display_begin_frame();
        if (color.r < KF_TRANSITION_FADE_LIMIT) {
            color.r += KF_TRANSITION_FADE_STEP;
        } else {
            color.r = KF_TRANSITION_FADE_LIMIT;
        }
        color.b = color.r;
        color.g = color.r;
        sprite_add_ft4(&rect, uv.v, tpage, clut, &color, KF_TRANSITION_OT_DEPTH);
        display_present_frame();
    }
    DrawSync(0);
}
