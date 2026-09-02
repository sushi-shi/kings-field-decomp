#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfMapObjectState map_object_state;

extern KfDisplayState display_state;

extern KfActorState actor_state;

extern KfPlayerState player_state;

/* Psy-Q Release 2.5 MEMORY.H declares memset without a prototype. */
extern void *memset();
/* Psy-Q KERNEL: OpenEvent(desc, spec, mode, func), EnableEvent, CloseEvent; LIBGPU SetDispMask. */
extern s32 OpenEvent(u32 descriptor, s32 spec, s32 mode, void (*handler)(void));
extern s32 EnableEvent(s32 event);
extern s32 CloseEvent(s32 event);

extern void memory_card_initialize(void);
extern void memory_set_allocation_mode(s32 arg0);
extern void audio_initialize(void);
extern void func_8001bb94(void);
extern void func_80020cfc(void);
extern void actor_pool_clear(void);
extern void map_object_pool_clear(void);
extern void effect_pool_reset(void);
extern void func_800356e8(void);
extern void common_resources_load(void);
extern void game_initialize_session(void);
extern void memory_capture_system_heap_start(void);
extern void memory_reset_system_heap(void);
extern void func_800365f8(void);
extern void func_80014674(u32 arg0);
extern s32 save_file_cleanup_temporary(void);
extern void func_8001b7b0(char object);
extern void player_update(void);
extern void player_update_transform_snapshot(
    VECTOR *position_out, SVECTOR *rotation_out);
extern void audio_set_listener_transform(
    const VECTOR *position_or_null, const SVECTOR *rotation_or_null);
extern void actor_set_player_transform(
    const VECTOR *position, const SVECTOR *rotation);
extern void actor_pool_update(void);
extern void map_object_pool_update(void);
extern void effect_pool_sweep(void);
extern void func_8003596c(void);
extern void func_8001fde4(
    const VECTOR *position_or_null, const SVECTOR *rotation_or_null);
extern u32 player_warp_trigger_update(void);
extern void func_800144d4(void);
extern void audio_stop_sequence_master_fade(s32 fade_step);
extern void memory_card_shutdown_events(void);
extern void audio_shutdown(void);
extern void pad_stop(void);
extern u32 func_80050544(u32 object);
extern void frame_pacer_vsync_callback(void);
extern void frame_pacer_wait(void);
extern void game_shutdown(void);

extern KfMagicRecord magic_records[24];
extern KfMapEvent map_event_pool[8];
extern u32 game_exit_code;
extern VECTOR player_position_snapshot;
extern SVECTOR player_rotation_snapshot;
extern u8 map_cell_attribute_grid[100][100];
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

/*
 * Both counters live in the retail load image four bytes apart, so they were
 * explicitly initialized data, not commons; static linkage is the curated
 * TU-ownership candidate for this frame pacer.
 */
DATA(0x80057b0c, 0x4)
static u32 frame_pacer_vsync_count = 0;

DATA(0x80057b10, 0x4)
static u32 frame_pacer_last_vsync = 0;
/*
 * The player state block is one aggregate in the original source (see
 * KfPlayerState); the inventory still names its members separately, so it is
 * viewed through its first member until the aggregate identity exists.
 */

/*
 * The six memset spans clear whole BSS runs that start at the named objects.
 */
ADDRESS(0x800146b8, 0x2e4)
void game_main_loop(void)
{
    s32 vsync_event;

    memset(&display_state.buffer_index, 0, 0x249cc);
    memset(actor_state.definitions, 0, 0x2b48);
    memset(map_object_state.definitions, 0, 0x25b8);
    memset(magic_records, 0, 0xd28);
    memset(map_event_pool, 0, 0x2360);
    memset(&player_state, 0, sizeof(KfPlayerState));
    memory_card_initialize();
    memory_set_allocation_mode(0);
    audio_initialize();
    func_8001bb94();
    func_80020cfc();
    actor_pool_clear();
    map_object_pool_clear();
    effect_pool_reset();
    func_800356e8();
    common_resources_load();
    game_initialize_session();
    memory_set_allocation_mode(1);
    memory_capture_system_heap_start();
    memory_reset_system_heap();
    func_800365f8();
    SetDispMask(1);
    vsync_event = OpenEvent(0xf2000003, 2, 0x1000, frame_pacer_vsync_callback);
    EnableEvent(vsync_event);
    func_80014674(1);
    if (save_file_cleanup_temporary() == 2) {
        func_8001b7b0(2);
    }
    game_exit_code = 0;
    for (;;) {
        player_update();
        if (game_exit_code != 0) {
            break;
        }
        player_update_transform_snapshot(&player_position_snapshot, &player_rotation_snapshot);
        audio_set_listener_transform(&player_position_snapshot, &player_rotation_snapshot);
        actor_set_player_transform(&player_position_snapshot, &player_rotation_snapshot);
        actor_pool_update();
        map_object_pool_update();
        effect_pool_sweep();
        func_8003596c();
        func_8001fde4(&player_position_snapshot, &player_rotation_snapshot);
        player_state.unknown_0d = 0;
        frame_pacer_wait();
        if (map_cell_attribute_grid[player_state.map_cell.z][player_state.map_cell.x]
            == 0x40) {
            if (*(u16 *)&player_state.previous_map_cell
                != *(u16 *)&player_state.map_cell) {
                if (player_warp_trigger_update() != 0) {
                    game_exit_code = 0xfe;
                    func_80014674(2);
                    func_800144d4();
                    audio_stop_sequence_master_fade(0x80);
                    break;
                }
                player_state.previous_map_cell.x = player_state.map_cell.x;
                player_state.previous_map_cell.z = player_state.map_cell.z;
            }
        } else {
            player_state.previous_map_cell.z = 0xff;
            player_state.previous_map_cell.x = 0xff;
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
    pad_stop();
    func_80050544(3);
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
        if (frame_pacer_last_vsync + 2 < frame_pacer_vsync_count
            || frame_pacer_vsync_count < frame_pacer_last_vsync) {
            frame_pacer_last_vsync = frame_pacer_vsync_count;
            ExitCriticalSection();
            return;
        }
        ExitCriticalSection();
    }
}
