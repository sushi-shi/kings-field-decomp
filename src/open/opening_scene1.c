#include <kf/address.h>
#include <kf/open_audio.h>
#include <kf/open_opening_helpers.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>
#include <kf/open_scene1.h>

ADDRESS(0x800143dc, 0x180)
void opening_scene1_draw_fade(u8 shade)
{
    POLY_FT4 *left;
    POLY_FT4 *right;
    u32 **ordering_table_slot;

    display_begin_frame();
    left = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
    right = (POLY_FT4 *)display_state.primitive_buffer->cursor;
    display_state.primitive_buffer->cursor += sizeof(POLY_FT4);

    SetPolyFT4(left);
    SetPolyFT4(right);
    left->tpage = GetTPage(2, 0, 0x140, 0x100);
    right->tpage = GetTPage(2, 0, 0x1c0, 0x100);

    left->x0 = 0;
    left->y0 = 0;
    left->x1 = 192;
    left->y1 = 0;
    left->x2 = 0;
    left->y2 = 240;
    left->x3 = 192;
    left->y3 = 240;

    right->x0 = 128;
    right->y0 = 0;
    right->x1 = 320;
    right->y1 = 0;
    right->x2 = 128;
    right->y2 = 240;
    right->x3 = 320;
    right->y3 = 240;

    left->u0 = 0;
    left->v0 = 0;
    left->u1 = 192;
    left->v1 = 0;
    left->u2 = 0;
    left->v2 = 240;
    left->u3 = 192;
    left->v3 = 240;

    right->u0 = 0;
    right->v0 = 0;
    right->u1 = 192;
    right->v1 = 0;
    right->u2 = 0;
    right->v2 = 240;
    right->u3 = 192;
    right->v3 = 240;

    left->r0 = shade;
    left->g0 = shade;
    left->b0 = shade;
    right->r0 = shade;
    right->g0 = shade;
    right->b0 = shade;

    ordering_table_slot = &ordering_table;
    AddPrim(*ordering_table_slot, left);
    AddPrim(*ordering_table_slot, right);
    display_present_frame();
}

ADDRESS(0x8001455c, 0xac)
void opening_scene1_run(void)
{
    s32 frame;
    s32 shade;

    opening_resources_load_scene1();
    shade = 0;
    do {
        opening_scene1_draw_fade((u8)shade);
        opening_poll_input();
        shade += 4;
    } while (shade < 129);

    frame = 0;
    do {
        if (frame == 600) {
            audio_stop_sequence(0);
        }
        VSync(0);
        opening_poll_input();
        if (opening_input_action != 0) {
            break;
        }
        frame++;
    } while (frame < 1000);

    audio_stop_sequence(1);
    shade = 128;
    do {
        opening_scene1_draw_fade((u8)shade);
        shade -= 4;
    } while (shade >= 0);
}
