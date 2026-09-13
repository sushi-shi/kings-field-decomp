#include <kf/null.h>
#include <kf/address.h>
#include <kf/overlay.h>
#include <kf/audio.h>
#include <kf/cd_file.h>
#include <kf/memory.h>
#include <kf/open_controller.h>
#include <kf/open_opening_helpers.h>
#include <kf/open_opening_scenes.h>
#include <kf/open_render.h>
#include <kf/open_resources.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <psyq/pad.h>
#include <kf/resources.h>

DATA(0x800372d4, 0x6)
char opening_initial_tim_path[KF_OPENING_INITIAL_TIM_PATH_BYTES] = {'B', '0', '\\', 'L', '0', '.'};

RODATA(0x80012020, 0x15)

ADDRESS(0x800156bc, 0x214)
void opening_run(KfOverlayMode overlay_mode)
{
    u8 *tim_data;
    KF_ENUM_STORAGE(KfOpeningInputAction, s32) scene3_action;
    KF_ENUM_STORAGE(KfOpeningInputAction, s32) skip_action;

    PadInit(0);
    /* Retail clears the display state and the contiguous opening runtime BSS. */
    memset(&open_graphics_runtime, 0, sizeof open_graphics_runtime);
    memset(&opening_entity_state, 0, sizeof opening_entity_state);
    memory_set_allocation_mode(KF_MEMORY_CREATE_ARENA);
    audio_initialize();
    display_initialize(overlay_mode);
    opening_entity_pool_reset();
    memory_set_allocation_mode(KF_MEMORY_REBASE_ARENA);
    memory_capture_system_heap_start();
    memory_reset_system_heap();

    switch (overlay_mode) {
    case KF_OVERLAY_MODE_INTRO:
        SetDispMask(1);
        if (cd_file_load_into(
                open_graphics_runtime.display_state.asset_load_buffer,
                opening_initial_tim_path) != KF_RESOURCE_LOADED) {
            return;
        }
        scene3_action = KF_OPENING_INPUT_ADVANCE;
        tim_upload_images(open_graphics_runtime.display_state.asset_load_buffer);
        skip_action = KF_OPENING_INPUT_SKIP;
        opening_fade_in();
        cd_file_load_allocated(&tim_data, "B0\\MIX0.");
        tim_upload_images(tim_data);
        memory_release_last();
        opening_input_action = KF_OPENING_INPUT_NONE;

        for (;;) {
            opening_scene0_run();
            if (opening_input_action != scene3_action &&
                opening_input_action == skip_action) {
opening_reload:
                /* Retail addresses this reload relative to the allocation
                 * cursor's address: the compiler registerises the store's
                 * destination first and derives the start and stack slots
                 * from it, then hoists that address out of the loop. */
                memory_arena.allocation.cursor = memory_arena.start;
                memory_arena.allocation.stack[KF_MEMORY_STACK_DEPTH_INDEX] = 0;
                cd_file_load_allocated(&tim_data, "B0\\MIX3.");
                tim_upload_images(tim_data);
                memory_release_last();
                audio_stop_sequence(KF_AUDIO_STOP_FADE);
                break;
            }

            opening_input_action = KF_OPENING_INPUT_NONE;
            opening_scene1_run();
            if (opening_input_action != scene3_action) {
                if (opening_input_action == skip_action) {
                    goto opening_reload;
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
    audio_shutdown();
    ResetGraph(KF_GPU_RESET_KEEP_DISPLAY);
    PadStop();
}
