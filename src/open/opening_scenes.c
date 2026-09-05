#include <kf/address.h>
#include <kf/audio.h>
#include <kf/open_audio.h>
#include <kf/open_camera_path.h>
#include <kf/open_opening_helpers.h>
#include <kf/open_opening_render.h>
#include <kf/open_opening_scenes.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>
#include <kf/open_scene0.h>
#include <kf/psyq_audio.h>

typedef char KfOpeningEntitySizeCheck[
    sizeof(KfOpeningEntity) == 0x28 ? 1 : -1];
typedef char KfOpeningEntityPositionOffsetCheck[
    (u32)&((KfOpeningEntity *)0)->position == 0x08 ? 1 : -1];

DATA(0x800354f4, 0x1dc)
KfCameraPathPoint opening_scene0_camera_path[17] = {
    {{101000, -13000, 101000, 0}, {256, 0, 200, 0}, 0, 0},
    {{101000, -12000, 117000, 0}, {0, 128, -200, 0}, 400, 0},
    {{101000, -11700, 127000, 0}, {0, 1024, 100, 0}, 400, 0},
    {{95000, -11500, 127000, 0}, {0, 0, 300, 0}, 400, 0},
    {{95000, -11200, 151000, 0}, {-256, 0, 0, 0}, 400, 0},
    {{95000, -11800, 165000, 0}, {0, -800, 0, 0}, 400, 0},
    {{97000, -12200, 167000, 0}, {0, -1024, -200, 0}, 400, 0},
    {{113000, -11900, 167000, 0}, {0, -1024, -100, 0}, 400, 0},
    {{127000, -11800, 167000, 0}, {0, -1024, 0, 0}, 400, 0},
    {{131000, -12200, 167000, 0}, {100, -800, -100, 0}, 400, 0},
    {{139000, -12000, 171000, 0}, {100, -512, -100, 0}, 400, 0},
    {{149000, -11900, 181000, 0}, {100, -750, -100, 0}, 400, 0},
    {{161000, -11900, 189000, 0}, {100, -1024, -100, 0}, 400, 0},
    {{163000, -11900, 187000, 0}, {100, -1536, -100, 0}, 300, 0},
    {{163000, -11900, 163000, 0}, {100, -512, -100, 0}, 200, 0},
    {{163000, -12100, 161000, 0}, {100, -256, -100, 0}, 100, 0},
    {{-1, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},
};

DATA(0x800356d0, 0x54)
KfCameraPathPoint opening_scene3_camera_path[3] = {
    {{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},
    {{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},
    {{-1, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},
};

DATA(0x80035724, 0xfc)
KfCameraPathPoint opening_ending_camera_path[9] = {
    {{101000, -11500, 101000, 0}, {0, 0, 0, 0}, 0, 0},
    {{101000, -11500, 108600, 0}, {0, 0, 0, 0}, 35, 0},
    {{101000, -11500, 108800, 0}, {0, 0, 0, 0}, 20, 0},
    {{101000, -11500, 109000, 0}, {0, 0, 0, 0}, 10, 0},
    {{101000, -11500, 109050, 0}, {0, 0, 0, 0}, 1, 0},
    {{101000, -11500, 109400, 0}, {0, 0, 0, 0}, 30, 0},
    {{101000, -11500, 109800, 0}, {0, 0, 0, 0}, 60, 0},
    {{101000, -11500, 117000, 0}, {0, 0, 0, 0}, 100, 0},
    {{-1, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},
};

DATA(0x80035820, 0x54)
static KfCameraPathPoint opening_ending_scroll_camera_path[3] = {
    {{101000, -8500, 89000, 0}, {0, 0, 0, 0}, 0, 0},
    {{101000, -12000, 89000, 0}, {0, 0, 0, 0}, 2, 0},
    {{-1, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},
};

DATA(0x80035874, 0x4)
SoundRef opening_scene0_sound = {9, 0, 0x43};

DATA(0x80035878, 0x10)
u16 opening_scene3_overlay_rects[2][4] = {
    {32, 256, 255, 254},
    {32, 512, 255, 254},
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

DATA(0x80037284, 0x8)
u8 opening_scene3_overlay_uv[8] = {0, 0, 0, 0, 255, 0, 254, 0};

DATA(0x8003728c, 0x4)
u8 opening_scene3_overlay_color[4] = {200, 200, 200, 0};

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

ADDRESS(0x80014268, 0x174)
void opening_scene0_run(void)
{
    KfOpeningEntity *entity_11;
    KfOpeningEntity *entity_12;
    s16 blend;
    s16 next_blend;

    blend = 0;
    opening_resources_load_scene0();
    entity_11 = opening_entity_find_by_object_id(
        opening_entity_state.entities, 11);
    entity_12 = opening_entity_find_by_object_id(
        opening_entity_state.entities, 12);
    entity_12->rotation.y = 0xc00;
    entity_11->rotation.y = 0xc00;
    opening_camera_path_begin(opening_scene0_camera_path);

    for (;;) {
        opening_poll_input();
        opening_camera_path_step(0);
        if (opening_camera_path_state.point_index >= 8) {
            if (entity_11->rotation.y == 0xc00) {
                sound_ref_play(&opening_scene0_sound, 100);
            }
            entity_11->rotation.y -= 40;
            entity_12->rotation.y += 40;
        }

        if (opening_camera_path_state.point_index >= 15) {
            goto fade_out;
        }
        if (opening_input_action == 0) {
            goto fade_in;
        }

fade_out:
        if (blend < 0) {
            goto scene_complete;
        }
        next_blend = blend - 0x100;
        blend = next_blend;
        goto update_color;

fade_in:
        if (blend >= 0x1000) {
            goto render_frame;
        }
        next_blend = blend + 0x100;
        blend = next_blend;

update_color:
        lighting_set_color_matrix(
            &color_matrix_table[1], &color_matrix_table[0], next_blend);

render_frame:
        audio_set_listener_transform(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        opening_scene0_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
    }

scene_complete:
    audio_stop_sequence(1);
}

ADDRESS(0x800143dc, 0x180)
void opening_scene1_draw_fade(u8 shade)
{
    POLY_FT4 *left;
    POLY_FT4 *right;
    u32 **ordering_table_slot;

    display_begin_frame();
    left = (POLY_FT4 *)open_graphics_runtime.display_state.primitive_buffer->cursor;
    open_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT4);
    right = (POLY_FT4 *)open_graphics_runtime.display_state.primitive_buffer->cursor;
    open_graphics_runtime.display_state.primitive_buffer->cursor += sizeof(POLY_FT4);

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

    ordering_table_slot = &open_graphics_runtime.ordering_table;
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

ADDRESS(0x80014608, 0x1fc)
void opening_entity_transition(s16 mode, const VECTOR *position)
{
    KfOpeningEntity *entity;
    VECTOR position_snapshot;
    s16 entity_index;
    s16 frame;
    s16 initial_scale_y;
    s16 scale_step;

    switch (mode) {
    case 0:
        initial_scale_y = 0;
        scale_step = 0x100;
        break;
    case 1:
        goto deactivate;
    case 2:
    case 3:
        initial_scale_y = 0x2000;
        scale_step = -0x100;
        break;
    }

    entity = &opening_entity_state.entities[24];
    entity_index = 3;
    /* Retail retains these stack coordinates without a subsequent consumer. */
    position_snapshot.vx = position->vx;
    position_snapshot.vz = position->vz;
    position_snapshot.vy = position->vy;
    do {
        entity->object_id = 0x13;
        entity->position = *position;
        entity->rotation.z = 0;
        entity->rotation.y = 0;
        entity->rotation.x = 0;
        entity->scale.vz = 0x1000;
        entity->scale.vx = 0x1000;
        entity->scale.vy = initial_scale_y;
        entity++;
        entity_index--;
    } while (entity_index != -1);

    if (mode == 3) {
        return;
    }

    frame = 0;
    do {
        entity = &opening_entity_state.entities[24];
        entity_index = 0;
        do {
            if ((entity_index << 3) < frame) {
                u16 scale_y = entity->scale.vy;

                if (scale_y < 0x2001) {
                    entity->scale.vy = scale_step + scale_y;
                }
            }
            entity_index++;
            entity->rotation.y = (entity->rotation.y + 0x200) & 0xfff;
            entity++;
        } while (entity_index < 4);
        opening_render_frame(0, 0);
        VSync(0);
        frame++;
    } while (frame < 48);

    if (mode == 0) {
        return;
    }

deactivate:
    entity = &opening_entity_state.entities[24];
    entity_index = 3;
    do {
        entity->object_id = 0xff;
        entity++;
        entity_index--;
    } while (entity_index != -1);
}

ADDRESS(0x80014804, 0x330)
void opening_scene3_run(void)
{
    KfOpeningEntity *entity_13;
    KfOpeningEntity *entity_14;
    VECTOR transition_position;
    u32 texture_pages[2];
    u32 cluts[2];
    u16 *overlay_rect;
    s16 *overlay_y;
    s16 blend;
    s16 overlay_index;
    s32 wave_angle;

    wave_angle = 0;
    opening_resources_load_scene3();
    texture_pages[0] = (u16)GetTPage(0, 0, 0x1c0, 0x100);
    cluts[0] = (u16)GetClut(0, 0x1ed);
    texture_pages[1] = (u16)GetTPage(0, 0, 0x200, 0x100);
    cluts[1] = (u16)GetClut(0, 0x1ee);

    entity_13 = opening_entity_find_by_object_id(opening_entity_state.entities, 13);
    entity_14 = opening_entity_find_by_object_id(opening_entity_state.entities, 14);
    entity_14->rotation.y = 0;
    entity_13->rotation.y = 0;
    opening_camera_path_begin(opening_scene3_camera_path);

    blend = 0;
    for (;;) {
        lighting_set_color_matrix(
            &color_matrix_table[1], &color_matrix_table[0], blend);
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        opening_poll_input();
        blend += 0x100;
        if (blend >= 0x1001) {
            break;
        }
    }

    while (entity_13->rotation.y < 0x400) {
        entity_13->rotation.y += 0x10;
        entity_14->rotation.y -= 0x10;
        audio_set_listener_transform(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        opening_poll_input();
        if (opening_input_action != 0) {
            break;
        }
    }

    do {
        opening_camera_path_step(rsin(wave_angle) >> 7);
        if (opening_camera_path_state.frames_remaining == -1) {
            break;
        }
        wave_angle = (wave_angle + 100) & 0xfff;
        render_set_view_transform(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        display_begin_frame();
        SetGeomScreen(200);
        opening_render_entities();
        overlay_index = 0;
        overlay_rect = opening_scene3_overlay_rects[0];
        overlay_y = (s16 *)&overlay_rect[1];
        do {
            /* Retain quads while their signed Y span can still cross the screen. */
            if ((u16)(--*overlay_y + 0xff) < 0x1ef) {
                sprite_add_ft4(
                    overlay_rect,
                    opening_scene3_overlay_uv,
                    texture_pages[overlay_index],
                    cluts[overlay_index],
                    opening_scene3_overlay_color,
                    4);
            }
            overlay_index++;
            overlay_y += 4;
            overlay_rect += 4;
        } while (overlay_index < 2);
        display_present_frame();
        opening_poll_input();
    } while (opening_input_action == 0);

    transition_position.vx = opening_camera_path_state.position.vx;
    transition_position.vy = -10000;
    transition_position.vz = opening_camera_path_state.position.vz;
    if (opening_input_action == 0) {
        opening_entity_transition(0, &transition_position);
    }

    blend = 0;
    do {
        lighting_set_color_matrix(
            &color_matrix_table[0], &color_matrix_table[1], blend);
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        blend += 0x100;
    } while (blend < 0x1001);

    if (opening_input_action == 0) {
        opening_entity_transition(1, &transition_position);
    }
}

ADDRESS(0x80014b34, 0x2f4)
void opening_ending_scene_run(void)
{
    KfOpeningEntity *entity_13;
    KfOpeningEntity *entity_14;
    VECTOR transition_position;
    s16 brightness;
    s16 blend;
    s32 wave_angle;

    wave_angle = 0;
    opening_resources_load_ending();
    entity_13 = opening_entity_find_by_object_id(opening_entity_state.entities, 13);
    entity_14 = opening_entity_find_by_object_id(opening_entity_state.entities, 14);
    entity_14->rotation.y = 0;
    entity_13->rotation.y = 0;
    opening_camera_path_begin(opening_ending_camera_path);

    transition_position.vy = -10000;
    transition_position.vx = opening_camera_path_state.position.vx;
    transition_position.vz = opening_camera_path_state.position.vz;
    SetDispMask(1);
    blend = 0;
    opening_entity_transition(3, &transition_position);

    do {
        lighting_set_color_matrix(
            &color_matrix_table[1], &color_matrix_table[0], blend);
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        blend += 0x100;
    } while (blend < 0x1001);

    opening_entity_transition(2, &transition_position);
    brightness = 0;
    blend = 0;
    for (;;) {
        opening_camera_path_step(rsin(wave_angle) >> 7);
        if (opening_camera_path_state.frames_remaining == -1) {
            break;
        }
        if (opening_camera_path_state.point_index >= 4) {
            if (entity_13->rotation.y < 0x400) {
                entity_13->rotation.y += 0x10;
                entity_14->rotation.y -= 0x10;
                audio_set_listener_transform(
                    &opening_camera_path_state.position,
                    &opening_camera_path_state.rotation);
                opening_render_frame(
                    &opening_camera_path_state.position,
                    &opening_camera_path_state.rotation);
                SetBackColor(brightness, brightness, brightness);
                SetFarColor(brightness, brightness, brightness);
                setRGB0(
                    &open_graphics_runtime.display_draw_environments[0],
                    brightness,
                    brightness,
                    brightness);
                setRGB0(
                    &open_graphics_runtime.display_draw_environments[1],
                    brightness,
                    brightness,
                    brightness);
                if (brightness >= 0xff) {
                    brightness = 0xff;
                } else {
                    brightness += 4;
                }
                if (blend < 0xfff) {
                    blend += 0x40;
                } else {
                    blend = 0xfff;
                }
            }
        } else {
            wave_angle = (wave_angle + 100) & 0xfff;
        }
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
    }

    brightness = 0xff;
    blend = 0;
    do {
        if (brightness < 0) {
            brightness = 0;
        }
        SetBackColor(brightness, brightness, brightness);
        SetFarColor(brightness, brightness, brightness);
        setRGB0(
            &open_graphics_runtime.display_draw_environments[0], brightness, brightness, brightness);
        setRGB0(
            &open_graphics_runtime.display_draw_environments[1], brightness, brightness, brightness);
        lighting_set_color_matrix(
            &color_matrix_table[0], &color_matrix_table[1], blend);
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        blend += 0x100;
        brightness -= 0x10;
    } while (blend < 0x1001);
}

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

    open_graphics_runtime.render_state.light_matrix = light_matrix;
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
    open_graphics_runtime.tmd_projection_shift = 2;
    /* Retail retains this otherwise unconsumed stack-owned position snapshot. */
    transition_position.vy = -10000;
    transition_position.vx = opening_camera_path_state.position.vx;
    transition_position.vz = opening_camera_path_state.position.vz;
    open_graphics_runtime.floor_item_state.material.color.r = 0;
    open_graphics_runtime.floor_item_state.material.color.g = 0;
    open_graphics_runtime.floor_item_state.material.color.b = 0;
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
            if (entity_27->object_id != 0xff && open_graphics_runtime.floor_item_state.material.color.r < 255) {
                ++open_graphics_runtime.floor_item_state.material.color.r;
                open_graphics_runtime.floor_item_state.material.color.b = open_graphics_runtime.floor_item_state.material.color.r;
                open_graphics_runtime.floor_item_state.material.color.g = open_graphics_runtime.floor_item_state.material.color.r;
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
