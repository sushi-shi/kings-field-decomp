#include <kf/address.h>
#include <kf/open_render.h>

typedef struct {
    u8 v[4];
} OpeningFadeColor;

DATA(0x800372c0, 0x8)
u16 opening_fade_rect[4] = {32, 0, 255, 240};
DATA(0x800372c8, 0x8)
u8 opening_fade_uv[8] = {0, 0, 0, 0, 255, 0, 240, 0};
DATA(0x800372d0, 0x4)
OpeningFadeColor opening_fade_color = {{0, 0, 0, 0}};

ADDRESS(0x800155c0, 0xfc)
void opening_fade_in(void)
{
    OpeningFadeColor color = opening_fade_color;
    int tpage;
    int clut;
    s32 frame;

    tpage = GetTPage(0, 0, 0x240, 0x100);
    clut = GetClut(0, 0x1ef);
    frame = 0;
    do {
        display_begin_frame();
        if (color.v[0] < 0xff) {
            color.v[0] += 6;
        } else {
            color.v[0] = 0xff;
        }
        color.v[2] = color.v[0];
        color.v[1] = color.v[0];
        sprite_add_ft4(
            opening_fade_rect, opening_fade_uv, tpage, clut, color.v, 4);
        display_present_frame();
        frame++;
    } while (frame < 18);
    DrawSync(0);
}
