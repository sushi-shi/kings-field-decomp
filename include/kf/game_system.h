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
extern void asset_aux_block_load(const u32 *source);
extern void asset_registry_load_tmd_archive(u16 first_asset_id, u8 *archive);
extern void asset_registry_select(u16 index);
extern void asset_registry_set(u16 index, void *asset);
extern void common_resources_load(void);
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
extern s32 memory_card_check_or_format(s16 allow_format);
extern void memory_card_initialize(void);
extern void memory_card_shutdown_events(void);
extern u32 pad_init_bad_identifier();
extern u32 pad_initialize(s32 identifier);
extern u32 pad_read();
extern void pad_read_bad_identifier(void);
extern void pad_stop(void);
extern void pad_stop_bad_identifier(void);
extern s32 save_file_cleanup_temporary(void);
extern s32 save_system_read_catalog(KfSaveSlotSummary *summaries);
extern s32 save_system_read_slot(s16 slot_id);
extern s32 save_system_write_slot(s16 slot_id);

#endif
