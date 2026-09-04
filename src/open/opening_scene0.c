#include <kf/address.h>
#include <kf/audio.h>
#include <kf/open_camera_path.h>
#include <kf/open_opening_helpers.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>
#include <kf/open_scene0.h>

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

DATA(0x80035874, 0x4)
SoundRef opening_scene0_sound = {9, 0, 0x43};
