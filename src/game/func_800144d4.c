#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfDisplayState display_state;
extern DRAWENV display_draw_environments[2];

extern void sprite_add_ft4(u16 *position, u8 *texcoords, u16 tpage, u16 clut,
                          u8 *color, u16 ot_index);
extern s32 cd_file_load_into(void *destination, char *relative_path);
extern void tim_upload_images(u_long *tim_data);
extern void display_begin_frame(void);
extern void display_present_frame(void);

/* Screen-quad descriptor blocks for the faded full-screen image. They keep
 * sub-word alignment so the retail copies them with unaligned word moves. */
typedef struct {
    u16 v[4];
} FadeRect;
typedef struct {
    u8 v[8];
} FadeUv;
typedef struct {
    u8 v[4];
} FadeColor;

/* Position rect {x, y, w, h}, texture-coordinate rect (even bytes u, v, w, h),
 * and the fade modulation color that ramps up from black. */
DATA(0x80057b14, 0x8)
FadeRect fade_screen_rect = {{32, 0, 255, 240}};
DATA(0x80057b1c, 0x8)
FadeUv fade_screen_uv = {{0, 0, 0, 0, 255, 0, 240, 0}};
DATA(0x80057b24, 0x4)
FadeColor fade_screen_color = {{0, 0, 0, 0}};
DATA(0x80057b28, 0x7)
char fade_screen_path[7] = "B0\\L0.";

/* Loads a full-screen image from the disc and fades it in over 18 frames by
 * ramping the modulation color; called from game_main_loop on a transition. */
ADDRESS(0x800144d4, 0x1a0)
void display_play_transition(void)
{
    FadeRect rect = fade_screen_rect;
    FadeUv uv = fade_screen_uv;
    FadeColor color = fade_screen_color;
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

    tpage = GetTPage(0, 0, 0x240, 0x100);
    clut = GetClut(0, 0x1ef);

    for (i = 0; i < 18; i++) {
        display_begin_frame();
        if (color.v[0] < 0xff) {
            color.v[0] += 6;
        } else {
            color.v[0] = 0xff;
        }
        color.v[2] = color.v[0];
        color.v[1] = color.v[0];
        sprite_add_ft4(rect.v, uv.v, tpage, clut, color.v, 4);
        display_present_frame();
    }
    DrawSync(0);
}
