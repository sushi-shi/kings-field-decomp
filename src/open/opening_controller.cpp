#include <kf/lib/null.h>

#include <kf/lib/overlay.h>
#include <kf/open/audio.h>
#include <kf/lib/resource_file.h>
#include <kf/lib/memory.h>
#include <kf/open/controller.h>
#include <kf/open/opening_helpers.h>
#include <kf/open/opening_scenes.h>
#include <kf/open/render.h>
#include <kf/open/resources.h>
#include <kf/lib/geometry_types.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <kf/platform/input.hpp>
#include <kf/lib/resources.h>

char opening_initial_tim_path[KF_OPENING_INITIAL_TIM_PATH_BYTES] = "B0/L0.";

static void opening_load_skip_assets(void)
{
    u8 *tim_data;
    std::size_t tim_size;
    memory_allocation_reset(memory_arena);
    resource_file_load_allocated(memory_arena, &tim_data, "B0/MIX3.", &tim_size);
    tim_upload_images(tim_data, tim_size);
    memory_release_last(memory_arena);
    audio_stop_sequence(KF_AUDIO_STOP_FADE);
}

void opening_run(KfOverlayMode overlay_mode)
{
    u8 *tim_data;
    std::size_t tim_size;
    KfEnumStorage<KfOpeningInputAction, s32> scene3_action;
    KfEnumStorage<KfOpeningInputAction, s32> skip_action;

    memset((void *)&open_graphics_runtime, 0, sizeof open_graphics_runtime);
    memset((void *)&opening_entity_state, 0, sizeof opening_entity_state);
    memory_set_allocation_mode(memory_arena, KF_MEMORY_CREATE_ARENA);
    audio_initialize();
    display_initialize(overlay_mode);
    opening_entity_pool_reset();
    memory_set_allocation_mode(memory_arena, KF_MEMORY_REBASE_ARENA);

    switch (overlay_mode) {
    case KF_OVERLAY_MODE_INTRO:
        if (resource_file_load_into(
                (void *)open_graphics_runtime.display_state.asset_load_buffer,
                open_graphics_runtime.display_state.asset_load_capacity,
                opening_initial_tim_path, &tim_size) != KF_RESOURCE_LOADED) {
            exit(1);
        }
        scene3_action = KF_OPENING_INPUT_ADVANCE;
        tim_upload_images(open_graphics_runtime.display_state.asset_load_buffer, tim_size);
        skip_action = KF_OPENING_INPUT_SKIP;
        opening_fade_in();
        resource_file_load_allocated(memory_arena, &tim_data, "B0/MIX0.", &tim_size);
        tim_upload_images(tim_data, tim_size);
        memory_release_last(memory_arena);
        opening_input_action = KF_OPENING_INPUT_NONE;

        for (;;) {
            opening_scene0_run();
            if (opening_input_action != scene3_action &&
                opening_input_action == skip_action) {
                opening_load_skip_assets();
                break;
            }

            opening_input_action = KF_OPENING_INPUT_NONE;
            opening_scene1_run();
            if (opening_input_action != scene3_action) {
                if (opening_input_action == skip_action) {
                    opening_load_skip_assets();
                    break;
                }
                continue;
            }

            audio_stop_sequence(KF_AUDIO_STOP_FADE);
            opening_input_action = KF_OPENING_INPUT_NONE;
            opening_scene3_run();

            audio_stop_sequence(KF_AUDIO_STOP_FADE);
            break;
        }
        break;

    case KF_OVERLAY_MODE_ENDING:
        opening_ending_scene_run();
        opening_ending_scroll_run();
        break;
    }

    opening_fade_in();
    audio_close_vab(audio_state);
}

void opening_controller_reset_module_state(void)
{
    kf::restore_initial_value<opening_initial_tim_path>();
}
