#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/overlay.h>
#include <kf/game_player.h>
#include <kf/game_save.h>
#include <kf/psyq_kernel.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

enum {
    INITIAL_ACTOR_CLEAR_BYTES = 0x2b48,
    INITIAL_MAP_OBJECT_CLEAR_BYTES = 0x25b8,
    INITIAL_MAP_EVENT_CLEAR_BYTES = 0x2360,
    FRAME_PACER_INTERVAL_TICKS = 3,
    ENDING_MASTER_FADE_STEP_Q8 = 0x80
};

/* Keep the initialized counters' external symbols in the curated relocation model. */
DATA(0x80057b0c, 0x4)
u32 frame_pacer_vsync_count = 0;

DATA(0x80057b10, 0x4)
u32 frame_pacer_last_vsync = 0;

DATA(0x800958f8, 0x4)
KfGameExitCode game_exit_code;

/* Clear the reviewed BSS spans without changing their starting referents. */
ADDRESS(0x800146b8, 0x2e4)
void game_main_loop(void)
{
    s32 vsync_event;

    memset(&game_graphics_runtime, 0, sizeof game_graphics_runtime);
    memset(&actor_state, 0, INITIAL_ACTOR_CLEAR_BYTES);
    memset(&map_object_state, 0, INITIAL_MAP_OBJECT_CLEAR_BYTES);
    memset(&effect_state, 0, sizeof(KfEffectState));
    memset(map_event_pool, 0, INITIAL_MAP_EVENT_CLEAR_BYTES);
    memset(&player_state, 0, sizeof(KfPlayerState));
    memory_card_initialize();
    memory_set_allocation_mode(KF_MEMORY_CREATE_ARENA);
    audio_initialize();
    display_initialize();
    item_load_database();
    actor_pool_clear();
    map_object_pool_clear();
    effect_pool_reset();
    map_event_timers_reset();
    common_resources_load();
    game_initialize_session();
    memory_set_allocation_mode(KF_MEMORY_REBASE_ARENA);
    memory_capture_system_heap_start();
    memory_reset_system_heap();
    func_800365f8();
    SetDispMask(1);
    vsync_event = OpenEvent(RCntCNT3, EvSpINT, EvMdINTR, frame_pacer_vsync_callback);
    EnableEvent(vsync_event);
    player_warp_shimmer_at_player(KF_WARP_SHIMMER_SHRINK_REMOVE);
    if (save_file_cleanup_temporary() == KF_SAVE_CLEANUP_CARD_TIMEOUT) {
        display_show_error_screen(KF_SYSTEM_SCREEN_NO_MEMORY_CARD);
    }
    game_exit_code = KF_GAME_EXIT_NONE;
    for (;;) {
        player_update();
        if (game_exit_code != KF_GAME_EXIT_NONE) {
            break;
        }
        player_update_transform_snapshot(&player_position_snapshot, &player_rotation_snapshot);
        audio_set_listener_transform(&player_position_snapshot, &player_rotation_snapshot);
        actor_set_player_transform(&player_position_snapshot, &player_rotation_snapshot);
        actor_pool_update();
        map_object_pool_update();
        effect_pool_sweep();
        map_event_pool_update();
        render_frame(&player_position_snapshot, &player_rotation_snapshot);
        player_state.allow_near_actor_spawn = KF_ACTOR_NEAR_SPAWN_FORBIDDEN;
        frame_pacer_wait();
        if (map_cell_attribute_grid.cells[player_state.motion_state.fields.map_cell.coords.z][player_state.motion_state.fields.map_cell.coords.x]
            == KF_MAP_ATTRIBUTE_WARP) {
            if (player_state.previous_map_cell.word
                != player_state.motion_state.fields.map_cell.word) {
                if (player_warp_trigger_update() != 0) {
                    game_exit_code = KF_GAME_EXIT_ENDING;
                    player_warp_shimmer_at_player(KF_WARP_SHIMMER_GROW_KEEP);
                    display_play_transition();
                    audio_stop_sequence_master_fade(ENDING_MASTER_FADE_STEP_Q8);
                    break;
                }
                player_state.previous_map_cell.coords.x = player_state.motion_state.fields.map_cell.coords.x;
                player_state.previous_map_cell.coords.z = player_state.motion_state.fields.map_cell.coords.z;
            }
        } else {
            player_state.previous_map_cell.coords.z = KF_MAP_CELL_COORD_INVALID;
            player_state.previous_map_cell.coords.x = KF_MAP_CELL_COORD_INVALID;
        }
    }
    CloseEvent(vsync_event);
    game_shutdown();
}

ADDRESS(0x8001499c, 0x38)
void game_shutdown(void)
{
    memory_card_shutdown_events();
    audio_shutdown();
    PadStop();
    ResetGraph(KF_GPU_RESET_KEEP_DISPLAY);
}

ADDRESS(0x800149d4, 0x20)
void frame_pacer_vsync_callback(void)
{
    frame_pacer_vsync_count++;
}

ADDRESS(0x800149f4, 0x70)
void frame_pacer_wait(void)
{
    for (;;) {
        EnterCriticalSection();
        if (frame_pacer_last_vsync + (FRAME_PACER_INTERVAL_TICKS - 1)
                < frame_pacer_vsync_count
            || frame_pacer_vsync_count < frame_pacer_last_vsync) {
            frame_pacer_last_vsync = frame_pacer_vsync_count;
            ExitCriticalSection();
            return;
        }
        ExitCriticalSection();
    }
}
