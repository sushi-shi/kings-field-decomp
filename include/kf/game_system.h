#ifndef KF_GAME_SYSTEM_H
#define KF_GAME_SYSTEM_H

/*
 * Core runtime prototypes: game loop, assets, input, and misc helpers.
 *
 * Generated during extern-crutch removal: declarations that were duplicated
 * as `extern` across src/game/*.c now live here once. DAT_/func_ spellings
 * remain unresolved WIP identities. Byte-neutral: a declaration never changes
 * codegen.
 */

#include <kf/game_actor.h>

extern void func_80014674(u32 mode);
extern void func_8002718c(void);
extern void func_800292f8(s32 object);
extern void func_800365f8(void);
extern void func_80036d3c(KfActor *actor);
extern void asset_registry_load_tmd_archive(u16 first_asset_id, u8 *archive);
extern void asset_registry_select(u16 index);
extern void asset_registry_set(u16 index, void *asset);
extern void common_resources_load(void);
extern void debug_print();
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
extern u32 pad_init_bad_identifier();
extern u32 pad_initialize(s32 identifier);
extern u32 pad_read();
extern void pad_read_bad_identifier(void);
extern void pad_stop(void);
extern void pad_stop_bad_identifier(void);
#endif
