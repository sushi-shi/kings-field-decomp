#include <kf/address.h>
#include <kf/open_audio.h>
#include <kf/open_camera_path.h>
#include <kf/open_entity_transition.h>
#include <kf/open_opening_helpers.h>
#include <kf/open_opening_render.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>
#include <kf/open_scene3.h>

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

DATA(0x800356d0, 0x54)
KfCameraPathPoint opening_scene3_camera_path[3] = {
    {{101000, -11500, 115200, 0}, {0, 0x800, 0, 0}, 0, 0},
    {{101000, -11500, 101000, 0}, {0, 0x800, 0, 0}, 0x10, 0},
    {{-1, -1, -1, 0}, {-1, -1, -1, 0}, -1, 0},
};

DATA(0x80035878, 0x10)
u16 opening_scene3_overlay_rects[2][4] = {
    {32, 256, 255, 254},
    {32, 512, 255, 254},
};

DATA(0x80037284, 0x8)
u8 opening_scene3_overlay_uv[8] = {0, 0, 0, 0, 255, 0, 254, 0};

DATA(0x8003728c, 0x4)
u8 opening_scene3_overlay_color[4] = {200, 200, 200, 0};
