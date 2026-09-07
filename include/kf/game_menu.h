#ifndef KF_GAME_MENU_H
#define KF_GAME_MENU_H

/*
 * Shared menu, item and talk UI types and prototypes.
 *
 * Generated during extern-crutch removal: types and declarations duplicated
 * across src/game/*.c live here once as their evidence permits. DAT_/func_
 * spellings remain unresolved WIP identities.
 */

#include <kf/game_save.h>
#include <kf/psyq.h>
#include <kf/magic.h>

KF_ENUM_BEGIN(KfMenuMode, s32)
    KF_MENU_MODE_ROOT = 0,
    KF_MENU_MODE_ITEM_PICKUP = 1,
    KF_MENU_MODE_SHOP = 2
KF_ENUM_END(KfMenuMode)

/* Negative controls in the root result channel; nonnegative values are item IDs. */
enum {
    KF_MENU_ROOT_PENDING = -99,
    KF_MENU_ROOT_GAME_LOADED = -3,
    KF_MENU_ROOT_RETURN_TO_INTRO = -2,
    KF_MENU_ROOT_NO_ITEM = -1
};

KF_ENUM_BEGIN(KfEquipmentMenuCategory, s32)
    KF_EQUIP_MENU_WEAPON = 0,
    KF_EQUIP_MENU_MAGIC = 1,
    KF_EQUIP_MENU_SHIELD = 2,
    KF_EQUIP_MENU_HEAD = 3,
    KF_EQUIP_MENU_BODY = 4,
    KF_EQUIP_MENU_ARM = 5,
    KF_EQUIP_MENU_LEG = 6,
    KF_EQUIP_MENU_ACCESSORY = 7
KF_ENUM_END(KfEquipmentMenuCategory)

KF_ENUM_BEGIN(KfMenuConfirmKind, s32)
    KF_MENU_CONFIRM_USE = 0,
    KF_MENU_CONFIRM_DROP = 1,
    KF_MENU_CONFIRM_YES_NO = 2,
    KF_MENU_CONFIRM_BUY = 3,
    KF_MENU_CONFIRM_SELL = 4,
    KF_MENU_CONFIRM_EQUIP = 5
KF_ENUM_END(KfMenuConfirmKind)

KF_ENUM_BEGIN(KfMenuPreviewMode, s32)
    KF_MENU_PREVIEW_ITEM_MODEL = 0,
    KF_MENU_PREVIEW_ITEM_DETAIL = 1,
    KF_MENU_PREVIEW_MAGIC_ICON = 2
KF_ENUM_END(KfMenuPreviewMode)

KF_ENUM_BEGIN(KfItemPriceMode, s32)
    KF_ITEM_PRICE_BUY = 0,
    KF_ITEM_PRICE_SELL = 1
KF_ENUM_END(KfItemPriceMode)

/* Indices in the window bank loaded from COM/STAT.DAT. */
KF_ENUM_BEGIN(KfMenuWindowKind, s32)
    KF_MENU_WINDOW_ROOT = 0,
    KF_MENU_WINDOW_EQUIPMENT = 1,
    KF_MENU_WINDOW_SYSTEM = 2,
    /* Present in the resource; no confirmed caller selects this layout. */
    KF_MENU_WINDOW_SAVE_LOAD = 3,
    KF_MENU_WINDOW_SAVE = 4,
    KF_MENU_WINDOW_LOAD = 5,
    KF_MENU_WINDOW_CONFIG = 6,
    KF_MENU_WINDOW_SHOP = 7
KF_ENUM_END(KfMenuWindowKind)

enum {
    /* Includes the final zero-filled record. */
    KF_MENU_WINDOW_LAYOUT_COUNT = 9
};

/* Rows in the loaded shop window; the gold row is not a menu choice. */
enum {
    KF_SHOP_ROW_BUY = 0,
    KF_SHOP_ROW_SELL = 1,
    KF_SHOP_ROW_RETURN = 2,
    KF_SHOP_ROW_GOLD = 3,
    KF_SHOP_CHOICE_COUNT = KF_SHOP_ROW_RETURN + 1
};

KF_ENUM_BEGIN(KfMenuConfirmResult, s32)
    KF_MENU_CONFIRM_PENDING = -99,
    KF_MENU_CONFIRM_CANCELLED = -1,
    KF_MENU_CONFIRM_ACCEPTED = 0
KF_ENUM_END(KfMenuConfirmResult)

/* Row positions in the loaded System, save and load windows. */
enum {
    KF_MENU_SYSTEM_LOAD_ROW = 0,
    KF_MENU_SYSTEM_QUIT_ROW = 1,
    KF_MENU_SYSTEM_RETURN_ROW = 2,
    KF_MENU_SYSTEM_ROW_COUNT = KF_MENU_SYSTEM_RETURN_ROW + 1,
    KF_MENU_SAVE_FORMAT_ROW = KF_SAVE_SLOT_COUNT,
    KF_MENU_SAVE_RETURN_ROW = KF_MENU_SAVE_FORMAT_ROW + 1,
    KF_MENU_SAVE_ROW_COUNT = KF_MENU_SAVE_RETURN_ROW + 1,
    KF_MENU_LOAD_RETURN_ROW = KF_SAVE_SLOT_COUNT,
    KF_MENU_LOAD_ROW_COUNT = KF_MENU_LOAD_RETURN_ROW + 1
};

enum {
    KF_MENU_TEXTURE_NONE = 0xff
};

KF_ENUM_BEGIN(KfItemPickupResult, s32)
    KF_ITEM_PICKUP_PENDING = -99,
    KF_ITEM_PICKUP_ACQUIRED = 0,
    KF_ITEM_PICKUP_NOT_ACQUIRED = 1,
    KF_ITEM_PICKUP_STACK_FULL = 2
KF_ENUM_END(KfItemPickupResult)

/* Feedback styles; the cursor cue is also reused for opening/config actions. */
enum {
    MENU_SOUND_CURSOR = 0,
    MENU_SOUND_CONFIRM = 1,
    MENU_SOUND_CANCEL_OR_ERROR = 2
};

/* Signed glyph streams share a terminator; each atlas has its own blank. */
enum {
    MENU_TEXT_END = -1,
    MENU_TEXT_BLANK = 0xff,
    MENU_TEXT_GLYPH_MASK = 0x0fff,
    MENU_TEXT_DAKUTEN = 0x1000,
    MENU_TEXT_HANDAKUTEN = 0x2000,
    MENU_NUMBER_BLANK = 10,
    MENU_NUMBER_SLASH = 11
};

/* Displayed class titles: physical tier rows, magic tier columns. */
enum {
    MENU_CLASS_MIDDLE_STAT_MIN = 40,
    MENU_CLASS_HIGH_STAT_MIN = 60,
    MENU_CLASS_MAGIC_TIER_COUNT = 3,
    MENU_CLASS_LABEL_GLYPHS = 4,
    MENU_CLASS_FIRST_GLYPH = 0x100
};

/* Shared ordering-table buckets; preserve insertion order within each bucket. */
enum {
    MENU_WIDGET_OT_DEPTH = 2000,
    MENU_WINDOW_OT_DEPTH = 2900,
    MENU_BACKGROUND_OT_DEPTH = 3000,
    /* Zero-based: release held input after the third panel draw. */
    MENU_PANEL_INPUT_RELEASE_FRAME = 2
};

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

/* The configuration draw ABI passes two complete halfword-aligned labels. */
typedef char check_menu_glyph_alignment[__alignof__(MenuGlyphString) == 2 ? 1 : -1];
typedef char check_menu_glyph_prefix[
    (unsigned long)&((MenuGlyphString *)0)->x == 0 &&
    (unsigned long)&((MenuGlyphString *)0)->y == 2 &&
    (unsigned long)&((MenuGlyphString *)0)->codes == 4 ? 1 : -1];

/* One title and ten selectable labels in a runtime-loaded menu window. */
typedef struct MenuWindowLayout {
    MenuGlyphString title;
    MenuGlyphString rows[10];
} MenuWindowLayout;

/* One runtime-loaded menu name: ten glyph codes selected by item or spell ID. */
typedef struct MenuGlyphRow {
    s16 codes[10];
} MenuGlyphRow;

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

/* Menu-list tile descriptor with byte-sized texture coordinates. */
typedef struct MenuTileSprite {
    u16 tpage;
    u16 clut;
    u8 u;
    u8 unknown_05;
    u8 v;
    u8 unknown_07;
    u16 width;
    u16 height;
} MenuTileSprite;

/* Complete first block loaded from COM\\STAT.DAT; GPU packets are mutable. */
typedef struct KfMenuAssets {
    POLY_FT4 background_quads[2][4];
    POLY_FT4 mid_depth_quads[2];
    POLY_FT4 foreground_quads[2];
    POLY_F4 dialog_quads[2][6];
    MenuSpriteDef number_atlas;
    MenuSpriteDef glyph_atlas;
    MenuTileSprite window_backdrop;
    MenuSpriteDef option_background;
    MenuSpriteDef option_highlight;
    MenuSpriteDef row_background;
    MenuSpriteDef row_confirmed_background;
    MenuTileSprite list_tiles[4];
    MenuSpriteDef selection_cursor;
} KfMenuAssets;

typedef enum MenuListTileIndex {
    MENU_LIST_TILE_BACKDROP,
    MENU_LIST_TILE_ROW,
    MENU_LIST_TILE_END,
    MENU_LIST_TILE_SELECTED,
} MenuListTileIndex;

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

/* Angle units per preview draw; a full revolution is 4096 units. */
enum {
    MENU_ITEM_PREVIEW_YAW_STEP = 16,
    MENU_PICKUP_PREVIEW_YAW_STEP = 8
};

extern SVECTOR menu_item_preview_rotation;
extern KfMenuAssets menu_assets;
extern MenuWindowLayout menu_window_layouts[KF_MENU_WINDOW_LAYOUT_COUNT];
extern MenuGlyphRow item_name_rows[80];
extern MenuGlyphRow magic_name_rows[KF_MAGIC_PLAYER_COUNT];
extern u16 item_buy_prices[80][2];
extern u16 item_sell_prices[80][2];
extern s32 menu_item_model_allocation_pending;

extern void item_load_database(void);
extern void item_menu_root(s32 shop_id);
extern KfItemPickupResult item_pickup_confirm(s32 item_id);
extern void menu_add_frame_quad(void);
extern void menu_add_marker_quad(void);
extern void menu_blit_sprite(
    const MenuSpriteDef *sprite, const MenuPoint *position);
extern void menu_blit_sprite_translucent(
    const MenuSpriteDef *sprite, const MenuPoint *position);
extern void menu_config_panel(void);
extern void menu_draw_dialog_frame(
    const KfSaveSlotSummary *summaries, s32 kind);
extern void menu_draw_item_detail(
    s32 item_id, s32 shop_id, KfItemPriceMode price_mode);
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
extern void menu_draw_window(KfMenuWindowKind kind, s32 count, s32 highlight, s32 flag);
extern void menu_draw_window_backdrop(void);
extern void menu_format_number(
    s32 value, s32 count, s32 pad_zero, s16 *out);
extern void menu_drop_item(void);
extern u32 menu_enter_mode(KfMenuMode mode, ...);
extern void menu_equip_select(KfEquipmentMenuCategory category);
extern void menu_frame_begin(void);
extern void menu_item_model_preview(s32 item_id);
extern void menu_list_init(KfMenuList *list, KfMenuWindowKind kind, s32 row);
extern KfMenuConfirmResult menu_list_interact(
    const KfMenuList *list, KfMenuConfirmKind kind, KfMenuPreviewMode preview_mode,
    s32 item_id, u32 shop_id, KfItemPriceMode price_mode);
extern void menu_list_render(const KfMenuList *list);
extern u32 menu_load_item_model(s32 id);
extern u32 menu_load_item_texture(s32 id);
extern void menu_release_item_model(void);
extern KfMenuConfirmResult menu_load_panel(void);
extern s32 menu_magic_panel(void);
extern void menu_map_viewer(s32 item_code);
extern void menu_option_root(void);
extern void menu_play_input_sound(s32 cue);
extern void menu_present_frame(void);
extern s32 menu_root(void);
extern void menu_save_confirm(void);
extern s32 menu_save_load_hub(void);
extern KfMenuConfirmResult menu_save_panel(void);
extern void menu_spell_select(void);
extern void menu_status_panel(void);
extern KfMenuConfirmResult menu_two_option_prompt(
    KfMenuWindowKind kind, s32 count, s32 highlight,
    const KfSaveSlotSummary *summaries);
extern void talk_show_dialogue_page(u8 floor, u8 stage, s32 character_id, u8 page);

#endif
