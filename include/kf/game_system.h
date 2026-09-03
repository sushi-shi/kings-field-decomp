#ifndef KF_GAME_SYSTEM_H
#define KF_GAME_SYSTEM_H

/*
 * Core runtime prototypes: memory, pools, game loop, save, cd,
 * assets, input and misc helpers.
 *
 * Generated during extern-crutch removal: declarations that were duplicated
 * as `extern` across src/game/*.c now live here once. DAT_/func_ spellings
 * remain unresolved WIP identities. Byte-neutral: a declaration never changes
 * codegen.
 */

#include <kf/semantic_types.h>

extern void func_80014674(u32 mode);
extern void effect5_texture_cache_prepare(s32 mode);
extern void func_8001ed38(void);
extern void func_8002718c(void);
extern void func_800292f8(s32 object);
extern void map_ambient_script_floor1(void);
extern void map_ambient_script_floor2(void);
extern void map_ambient_script_floor3(void);
extern void map_ambient_script_floor4(void);
extern void map_ambient_script_floor5(void);
extern void map_action_script_floor1(void);
extern void map_action_script_floor2(void);
extern void map_action_script_floor3(void);
extern void map_action_script_floor4(void);
extern void map_event_timers_reset(void);
extern void map_event_pool_update(void);
extern void map_world_state_persist(void);
extern void map_unload_floor(void);
extern void func_800365f8(void);
extern void func_80036d3c(KfActor *actor);
extern s16 func_800468d8(u8 *sequence, s16 vab_id);
extern void func_8004a344(s16 sequence);
extern void func_8004a3c8(s16 sequence, s16 track);
extern void memcpy_8004a52c();
extern void func_8004b6e0(s16 sequence);
extern s16 angle_approach(s16 current, s16 target, s32 step);
extern int angle_mod_delta_le_half_turn(int lhs, int rhs);
extern s16 angle_shortest_delta(s32 first, s32 second);
extern void angle_to_forward_xz(s16 angle, struct KfVecXZs *direction);
extern void asset_aux_block_load(const u32 *source);
extern void asset_registry_load_tmd_archive(u16 first_asset_id, u8 *archive);
extern void asset_registry_select(u16 index);
extern void asset_registry_set(u16 index, void *asset);
extern s32 cd_file_load_allocated(
    void **destination, const char *relative_path);
extern s32 cd_file_load_into(void *destination, const char *relative_path);
extern s32 cd_file_load_table_entry(void **destination, s32 index);
extern void common_resources_load(void);
extern int fixed6_ratio_step(int value, int divisor);
extern s32 fixed_vector2_length(s32 x, s32 y);
extern s32 format(const char *device);
extern char *format_int_dec(s32 value);
extern char *format_int_hex(u32 value);
extern char *format_pad_left(char *string, char pad, u8 width);
extern void frame_pacer_vsync_callback(void);
extern void frame_pacer_wait(void);
extern void game_initialize_session(void);
extern void game_main_loop(void);
extern void game_shutdown(void);
extern void game_state_acknowledge_pending(void);
extern void game_state_initialize(void);
extern void *memory_allocate(s32 size);
extern void memory_allocation_reset(void);
extern void memory_capture_system_heap_start(void);
extern void memory_set_allocation_mode(s32 mode);
extern s32 memory_card_check_or_format(s16 allow_format);
extern void memory_card_initialize(void);
extern void memory_card_shutdown_events(void);
extern void *memory_malloc_checked(s32 size);
extern void memory_release_last(void);
extern void memory_reset_system_heap(void);
extern u32 pad_init_bad_identifier();
extern u32 pad_initialize(s32 identifier);
extern void pad_read_bad_identifier(void);
extern void pad_stop(void);
extern void pad_stop_bad_identifier(void);
extern s32 save_file_cleanup_temporary(void);
extern s32 save_system_read_catalog(KfSaveSlotSummary *summaries);
extern s32 save_system_read_slot(s16 slot_id);
extern s32 save_system_write_slot(s16 slot_id);
extern void tim_upload_images(u_long *tim_data);
extern void vector2s_scale_shift11(s16 scale, s16 *vector);
extern void vector3s_scale_shift12(s16 scale, s16 *vector);

#endif
