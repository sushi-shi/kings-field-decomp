#ifndef KF_GAME_MENU_H
#define KF_GAME_MENU_H

/*
 * Shared menu, item and talk UI types and prototypes.
 *
 * Generated during extern-crutch removal: types and declarations duplicated
 * across src/game/*.c live here once as their evidence permits. DAT_/func_
 * spellings remain unresolved WIP identities.
 */

#include <kf/semantic_types.h>

/* Screen-space anchor shared by menu labels and sprite blitters. */
typedef struct MenuPoint {
    u16 x;
    u16 y;
} MenuPoint;

/*
 * Positioned menu text: screen origin followed by the usual ten-code label.
 * Capacity-specific workspaces use the same proven origin/code prefix.
 */
typedef struct MenuGlyphString {
    u16 x;
    u16 y;
    s16 codes[10];
} MenuGlyphString;

/*
 * Texture-page, CLUT, texel-origin, and extent descriptor shared by menu
 * sprites and font atlases. Retail reads every field as a halfword for
 * screen-space arithmetic; renderers narrow texel coordinates only when
 * storing them into GPU packet bytes.
 */
typedef struct MenuSpriteDef {
    u16 tpage;
    u16 clut;
    u16 u;
    u16 v;
    u16 width;
    u16 height;
} MenuSpriteDef;

/* Eight-byte cell in the runtime-loaded menu label bank. */
typedef struct MenuLabelCell {
    u16 halfwords[4];
} MenuLabelCell;

/*
 * Scrollable menu-list state. The first 24 bytes are a positioned title,
 * followed by list geometry/cursor bytes and the row-glyph and optional
 * quantity sources consumed by the shared list renderer.
 */
typedef struct KfMenuList {
    s16 title_x;
    u16 title_y;
    s16 title_glyphs[10];
    u8 list_x;
    u8 list_y;
    u8 entry_count;
    u8 visible_rows;
    u8 scroll_offset;
    u8 selected_index;
    u8 cursor_row;
    u8 glyphs_per_entry;
    s16 *glyph_rows;
    u8 *quantities;
} KfMenuList;

extern POLY_FT4 DAT_800580e8[2][4];
extern MenuSpriteDef DAT_800583e8;
extern MenuSpriteDef DAT_800583f4;
extern MenuSpriteDef DAT_8005840c;
extern MenuSpriteDef DAT_80058418;
extern MenuSpriteDef DAT_80058424;
extern MenuSpriteDef DAT_80058430;
extern MenuSpriteDef DAT_8005846c;
extern MenuLabelCell DAT_80058494[];

extern void item_load_floor_placements(KfFloorItemPlacement *placements);
extern void item_load_database(void);
extern void item_menu_root(s32 arg);
extern s32 item_use_confirm(s32 arg);
extern void menu_add_frame_quad(void);
extern void menu_add_marker_quad(void);
extern void menu_blit_sprite(
    const MenuSpriteDef *sprite, const MenuPoint *position);
extern void menu_blit_sprite_translucent(
    const MenuSpriteDef *sprite, const MenuPoint *position);
extern void menu_config_panel(void);
extern void menu_draw_dialog_frame(
    const KfSaveSlotSummary *summaries, s32 kind);
extern void menu_draw_item_detail(s32 object, s32 col, s32 mode);
extern void menu_draw_item_name_frame(s32 item_id);
extern void menu_draw_number(
    const MenuSpriteDef *font, const MenuGlyphString *string);
extern void menu_draw_string(
    const MenuSpriteDef *font, const MenuGlyphString *string);
extern void menu_draw_stats_header(void);
extern void menu_draw_status_details(void);
extern void menu_draw_two_option(
    const MenuGlyphString *option0, const MenuGlyphString *option1,
    s32 selected, s32 highlight);
extern void menu_draw_window(s32 kind, s32 count, s32 highlight, s32 flag);
extern void menu_draw_window_backdrop(void);
extern void menu_format_number(
    s32 value, s32 count, s32 pad_zero, s16 *out);
extern void menu_drop_item(void);
extern void menu_equip_select(s32 object);
extern void menu_frame_begin(void);
extern void menu_item_model_preview(s32 item_id);
extern void menu_list_init(KfMenuList *list, s32 row, s32 column);
extern s32 menu_list_interact(const KfMenuList *list, s32 kind, s32 mode,
                              s32 item_id, u32 arg4, u32 arg5);
extern void menu_list_render(const KfMenuList *list);
extern u32 menu_load_item_model(s32 id);
extern u32 menu_load_item_texture(s32 id);
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
extern s32 menu_two_option_prompt(
    s32 kind, s32 count, s32 highlight,
    const KfSaveSlotSummary *summaries);
extern void talk_show_indexed_image(u8 prefix_digit, u8 index_digit, s32 group_id, u8 frame_digit);

#endif
