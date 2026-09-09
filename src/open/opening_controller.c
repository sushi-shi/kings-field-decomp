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
#include <kf/psyq.h>
#include <kf/psyq_libc.h>
#include <kf/psyq_pad.h>
#include <kf/resources.h>

/* Retail retains the allocation subobject base across scene calls. */
#define OPENING_ARENA_FROM_ALLOCATION(state) \
    ((KfMemoryArena *)((u8 *)(state) - \
                       (u32)&((KfMemoryArena *)0)->allocation))

DATA(0x800372d4, 0x6)
char opening_initial_tim_path[KF_OPENING_INITIAL_TIM_PATH_BYTES] = {'B', '0', '\\', 'L', '0', '.'};

RODATA(0x80012020, 0x15)

ADDRESS(0x800156bc, 0x214)
void opening_run(KfOpenMode display_mode)
{
    void *tim_data;
    KfMemoryAllocationState *allocation_state;
    KF_ENUM_STORAGE(KfOpeningInputAction, s32) scene3_action;
    KF_ENUM_STORAGE(KfOpeningInputAction, s32) skip_action;

    PadInit(0);
    /* Retail clears the display state and the contiguous opening runtime BSS. */
    memset(&open_graphics_runtime, 0, sizeof open_graphics_runtime);
    memset(&opening_entity_state, 0, sizeof opening_entity_state);
    memory_set_allocation_mode(KF_MEMORY_CREATE_ARENA);
    audio_initialize();
    display_initialize(display_mode);
    opening_entity_pool_reset();
    memory_set_allocation_mode(KF_MEMORY_REBASE_ARENA);
    memory_capture_system_heap_start();
    memory_reset_system_heap();

    switch (display_mode) {
    case KF_OPEN_MODE_INTRO:
        SetDispMask(1);
        if (cd_file_load_into(
                open_graphics_runtime.display_state.asset_load_buffer,
                opening_initial_tim_path) != KF_RESOURCE_LOADED) {
            return;
        }
        scene3_action = KF_OPENING_INPUT_ADVANCE;
        tim_upload_images((u_long *)open_graphics_runtime.display_state.asset_load_buffer);
        skip_action = KF_OPENING_INPUT_SKIP;
        opening_fade_in();
        cd_file_load_allocated(&tim_data, "B0\\MIX0.");
        allocation_state = &memory_arena.allocation;
        tim_upload_images((u_long *)tim_data);
        memory_release_last();
        opening_input_action = KF_OPENING_INPUT_NONE;

opening_scene0:
        opening_scene0_run();
        if (opening_input_action == scene3_action) {
            goto opening_scene1;
        }
        if (opening_input_action != skip_action) {
            goto opening_scene1;
        }

opening_reload:
        allocation_state->stack[KF_MEMORY_STACK_DEPTH_INDEX] = 0;
        allocation_state->cursor =
            OPENING_ARENA_FROM_ALLOCATION(allocation_state)->start;
        cd_file_load_allocated(&tim_data, "B0\\MIX3.");
        tim_upload_images((u_long *)tim_data);
        memory_release_last();
        goto opening_complete;

opening_scene1:
        opening_input_action = KF_OPENING_INPUT_NONE;
        opening_scene1_run();
        if (opening_input_action == scene3_action) {
            goto opening_scene3;
        }
        if (opening_input_action == skip_action) {
            goto opening_reload;
        }
        goto opening_scene0;

opening_scene3:
        audio_stop_sequence(KF_AUDIO_STOP_FADE);
        opening_input_action = KF_OPENING_INPUT_NONE;
        opening_scene3_run();

opening_complete:
        audio_stop_sequence(KF_AUDIO_STOP_FADE);
        break;

    case KF_OPEN_MODE_ENDING:
        opening_ending_scene_run();
        opening_ending_scroll_run();
        break;
    }

    opening_fade_in();
    audio_shutdown();
    ResetGraph(KF_GPU_RESET_KEEP_DISPLAY);
    PadStop();
}
