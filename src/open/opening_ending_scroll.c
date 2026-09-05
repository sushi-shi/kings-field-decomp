#include <kf/address.h>
#include <kf/open_audio.h>
#include <kf/open_camera_path.h>
#include <kf/open_controller.h>
#include <kf/open_opening_render.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>
#include <kf/psyq_audio.h>

DATA(0x80035820, 0x54)
static KfCameraPathPoint opening_ending_scroll_camera_path[3] = {
    {{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},
    {{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},
    {{-1, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},
};

DATA(0x80035888, 0x48)
static u16 opening_ending_scroll_panels[9][4] = {
    {32, 256, 255, 254},
    {32, 512, 255, 254},
    {32, 768, 255, 254},
    {32, 1024, 255, 254},
    {32, 1261, 255, 254},
    {32, 1490, 255, 254},
    {32, 1746, 255, 254},
    {32, 2002, 255, 254},
    {32, 2258, 255, 254},
};

DATA(0x80037290, 0x10)
static u16 opening_ending_scroll_backgrounds[2][4] = {
    {0, 0, 320, 160}, {0, 160, 320, 160},
};
DATA(0x800372a0, 0x4)
static CVECTOR opening_ending_scroll_top_start = {32, 0, 32, 0};
DATA(0x800372a4, 0x4)
static CVECTOR opening_ending_scroll_bottom_start = {255, 0, 0, 0};
DATA(0x800372a8, 0x4)
static CVECTOR opening_ending_scroll_top_end = {0, 0, 32, 0};
DATA(0x800372ac, 0x4)
static CVECTOR opening_ending_scroll_bottom_end = {0, 0, 32, 0};
DATA(0x800372b0, 0x4)
static CVECTOR opening_ending_scroll_panel_color = {200, 200, 200, 0};
DATA(0x800372b4, 0x4)
static CVECTOR opening_ending_scroll_background_color = {0, 0, 0, 0};
DATA(0x800372b8, 0x8)
static u8 opening_ending_scroll_uv[8] = {0, 0, 1, 0, 255, 0, 254, 0};

RODATA(0x80012000, 0x20)

ADDRESS(0x80014e28, 0x798)
void opening_ending_scroll_run(void)
{
    VECTOR transition_position;
    MATRIX light_matrix = {
        {{0, 0, -4095}, {4095, 0, -2048}, {-4095, 0, -2048}}, {0, 0, 0}
    };
    u32 texture_pages[9];
    u32 cluts[9];
    CVECTOR top_color;
    CVECTOR bottom_color;
    KfOpeningEntity *entity_26;
    KfOpeningEntity *entity_27;
    s16 lighting_phase;
    s16 scrolling;
    s16 sequence_phase;
    s32 sequence_delay;
    s32 sequence_volume;
    s16 lighting_blend;
    s16 background_blend;
    s16 scroll_phase;
    s16 panel_index;
    u16 *panel;

    render_state.light_matrix = light_matrix;
    lighting_blend = 0;
    opening_resources_load_ending_entities();
    texture_pages[0] = GetTPage(0, 0, 0x1c0, 0x100);
    cluts[0] = GetClut(0, 0x1ed);
    texture_pages[1] = GetTPage(0, 0, 0x200, 0x100);
    cluts[1] = GetClut(0, 0x1ee);
    texture_pages[2] = GetTPage(0, 0, 0x240, 0x100);
    cluts[2] = GetClut(0, 0x1ef);
    texture_pages[3] = GetTPage(0, 0, 0x280, 0x100);
    cluts[3] = GetClut(0, 0x1f0);
    texture_pages[4] = GetTPage(0, 0, 0x2c0, 0x100);
    cluts[4] = GetClut(0, 0x1f1);
    texture_pages[5] = GetTPage(0, 0, 0x300, 0x100);
    cluts[5] = GetClut(0, 0x1f2);
    texture_pages[6] = GetTPage(0, 0, 0x340, 0x100);
    cluts[6] = GetClut(0, 0x1f3);
    texture_pages[7] = GetTPage(0, 0, 0x380, 0x100);
    cluts[7] = GetClut(0, 0x1f4);
    texture_pages[8] = GetTPage(0, 0, 0x3c0, 0x100);
    cluts[8] = GetClut(0, 0x1f5);

    entity_26 = opening_entity_find_by_object_id(opening_entity_state.entities, 26);
    entity_27 = opening_entity_find_by_object_id(opening_entity_state.entities, 27);
    lighting_phase = 0;
    background_blend = 0;
    sequence_phase = 0;
    scrolling = 0;
    scroll_phase = 0;
    entity_26->position.vy -= 1500;
    entity_27->object_id = 0xff;
    opening_camera_path_begin(opening_ending_scroll_camera_path);
    SetFogNear(11000, 200);
    SetBackColor(0, 0, 0);
    SetFarColor(0, 0, 0);
    tmd_projection_shift = 2;
    /* Retail retains this otherwise unconsumed stack-owned position snapshot. */
    transition_position.vy = -10000;
    transition_position.vx = opening_camera_path_state.position.vx;
    transition_position.vz = opening_camera_path_state.position.vz;
    floor_item_state.material.color.r = 0;
    floor_item_state.material.color.g = 0;
    floor_item_state.material.color.b = 0;
    top_color = opening_ending_scroll_top_start;
    bottom_color = opening_ending_scroll_bottom_start;

    for (;;) {
        switch (lighting_phase) {
        case 0:
            if (lighting_blend <= 0x1000) {
                lighting_set_color_matrix(
                    &color_matrix_table[1], &color_matrix_table[3], lighting_blend);
                lighting_blend += 0x40;
            } else {
                lighting_phase = 1;
                lighting_blend = 0;
            }
            break;
        case 1:
            if (lighting_blend <= 0x1000) {
                lighting_set_color_matrix(
                    &color_matrix_table[3], &color_matrix_table[4], lighting_blend);
                lighting_blend += 3;
            } else {
                lighting_phase = 2;
                lighting_blend = 0;
            }
            break;
        }
        switch (sequence_phase) {
        case 1:
            if (--sequence_delay == -1) {
                sequence_phase = 2;
                sequence_volume = 381;
            }
            break;
        case 2:
            --sequence_volume;
            SsSetMVol(sequence_volume / 3, sequence_volume / 3);
            if (sequence_volume == 0) {
                sequence_phase = 3;
                opening_resources_load_ending_sequence();
            }
            break;
        }
        if (opening_camera_path_state.frames_remaining != -1) {
            opening_camera_path_step(0);
        }
        if (entity_26->position.vy < -8000) {
            entity_26->position.vy += 3;
        } else if (scrolling == 0) {
            entity_26->object_id = 0xff;
            scrolling = 1;
            entity_27->object_id = 27;
        }
        render_set_view_transform(
            &opening_camera_path_state.position, &opening_camera_path_state.rotation);
        display_begin_frame();
        SetGeomScreen(200);
        opening_render_entities();

        background_blend += 3;
        if (background_blend > 0x1000) {
            background_blend = 0x1000;
        }
        color_lerp_cvector(&opening_ending_scroll_top_start,
                          &opening_ending_scroll_top_end, &top_color, background_blend);
        color_lerp_cvector(&opening_ending_scroll_bottom_start,
                          &opening_ending_scroll_bottom_end, &bottom_color, background_blend);
        sprite_add_g4(opening_ending_scroll_backgrounds[0],
                      &top_color.r, &top_color.r, &bottom_color.r, &bottom_color.r);
        sprite_add_f4(opening_ending_scroll_backgrounds[1],
                      &opening_ending_scroll_background_color.r, 0x2f65);

        if (scroll_phase == 0) {
            entity_27->rotation.z = (entity_27->rotation.z - 1) & 0xfff;
            if (entity_27->object_id != 0xff && floor_item_state.material.color.r < 255) {
                ++floor_item_state.material.color.r;
                floor_item_state.material.color.b = floor_item_state.material.color.r;
                floor_item_state.material.color.g = floor_item_state.material.color.r;
            }
        }
        if (scrolling > 0) {
            panel_index = 0;
            panel = opening_ending_scroll_panels[0];
            do {
                if (scroll_phase == 0 || scroll_phase == 2) {
                    if ((s16)opening_ending_scroll_panels[8][1] > 50) {
                        --panel[1];
                    } else if (sequence_phase == 0) {
                        sequence_phase = 1;
                        sequence_delay = 20;
                    }
                }
                if ((u16)(panel[1] + 255) < 495) {
                    sprite_add_ft4(panel, opening_ending_scroll_uv,
                                   texture_pages[panel_index], cluts[panel_index],
                                   &opening_ending_scroll_panel_color.r, 4);
                }
                ++panel_index;
                panel += 4;
            } while (panel_index < 9);
            if (--scroll_phase == -1) {
                scroll_phase = 3;
            }
        }
        display_present_frame();
    }
}
