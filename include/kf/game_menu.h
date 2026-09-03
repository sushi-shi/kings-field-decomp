#ifndef KF_GAME_MENU_H
#define KF_GAME_MENU_H

/*
 * Menu, item and talk UI prototypes.
 *
 * Generated during extern-crutch removal: declarations that were duplicated
 * as `extern` across src/game/*.c now live here once. DAT_/func_ spellings
 * remain unresolved WIP identities. Byte-neutral: a declaration never changes
 * codegen.
 */

#include <kf/semantic_types.h>

extern void item_load_floor_placements(KfFloorItemPlacement *placements);
extern void item_load_database(void);
extern s32 item_use_confirm(s32 arg);
extern void menu_add_frame_quad(void);
extern void menu_add_marker_quad(void);
extern void menu_config_panel(void);
extern void menu_draw_item_detail(s32 object, s32 col, s32 mode);
extern void menu_draw_number(u16 *atlas, s16 *str);
extern void menu_draw_stats_header(void);
extern void menu_drop_item(void);
extern void menu_equip_select(s32 object);
extern void menu_frame_begin(void);
extern void menu_item_model_preview(s32 item_id);
extern s32 menu_list_interact(u32 ctx, s32 arg1, s32 arg2, s32 item_id, u32 arg4, u32 arg5);
extern void menu_list_render(s16 *ctx);
extern u32 menu_load_item_model(s32 id);
extern s32 menu_load_panel(void);
extern s32 menu_magic_panel(void);
extern void menu_map_viewer(s32 item_code);
extern void menu_option_root(void);
extern void menu_play_input_sound(s32 cue);
extern void menu_present_frame(void);
extern s32 menu_root(void);
extern void menu_save_confirm(void);
extern s32 menu_save_load_hub(void);
extern s32 menu_save_panel(void);
extern void menu_spell_select(void);
extern void menu_status_panel(void);
extern void talk_show_indexed_image(u8 prefix_digit, u8 index_digit, s32 group_id, u8 frame_digit);

#endif
