#include <kf/address.h>
#include <kf/open_audio.h>
#include <kf/open_camera_path.h>
#include <kf/open_ending_scene.h>
#include <kf/open_entity_transition.h>
#include <kf/open_opening_render.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>

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
                    &display_draw_environments[0],
                    brightness,
                    brightness,
                    brightness);
                setRGB0(
                    &display_draw_environments[1],
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
            &display_draw_environments[0], brightness, brightness, brightness);
        setRGB0(
            &display_draw_environments[1], brightness, brightness, brightness);
        lighting_set_color_matrix(
            &color_matrix_table[0], &color_matrix_table[1], blend);
        opening_render_frame(
            &opening_camera_path_state.position,
            &opening_camera_path_state.rotation);
        blend += 0x100;
        brightness -= 0x10;
    } while (blend < 0x1001);
}

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
