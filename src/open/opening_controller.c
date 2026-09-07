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


/*
 * Cursor-relative retail accesses prove this allocator prefix layout. Its
 * canonical data ownership stays split until the allocator TU is resolved.
 */
typedef struct {
    u8 *cursor;
    u32 stack[KF_MEMORY_STACK_WORDS];
} OpeningAllocationState;

typedef struct {
    u8 *start;
    u8 *end;
    OpeningAllocationState allocation;
} OpeningArenaState;

#define OPENING_ARENA_FROM_ALLOCATION(state) \
    ((OpeningArenaState *)((u8 *)(state) - \
                           (u32)&((OpeningArenaState *)0)->allocation))

DATA(0x800372d4, 0x6)
char opening_initial_tim_path[6] = "B0\\L0.";

RODATA(0x80012020, 0x18)

ADDRESS(0x800156bc, 0x214)
void opening_run(s32 display_mode)
{
    void *tim_data;
    OpeningAllocationState *allocation_state;
    s32 scene3_action;
    s32 skip_action;

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
                opening_initial_tim_path) != 0) {
            return;
        }
        scene3_action = KF_OPENING_INPUT_ADVANCE;
        tim_upload_images(open_graphics_runtime.display_state.asset_load_buffer);
        skip_action = KF_OPENING_INPUT_SKIP;
        opening_fade_in();
        cd_file_load_allocated(&tim_data, "B0\\MIX0.");
        allocation_state = (OpeningAllocationState *)&memory_arena_cursor;
        tim_upload_images(tim_data);
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
        tim_upload_images(tim_data);
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
