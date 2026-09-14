#ifndef KF_GAME_MENU_H
#define KF_GAME_MENU_H

/*
 * Shared menu, item and talk UI types and prototypes.
 *
 * Generated during extern-crutch removal: types and declarations duplicated
 * across the GAME sources live here once as their evidence permits. DAT_/func_
 * spellings remain unresolved WIP identities.
 */

#include <kf/game/save.h>
#include <kf/lib/cd_file.h>
#include <kf/lib/debug.h>
#include <psyq/sdk.h>
#include <kf/game/magic.h>
#include <kf/lib/render_types.h>

KF_ENUM_BEGIN(KfMenuMode, s32)
    KF_MENU_MODE_ROOT = 0,
    KF_MENU_MODE_ITEM_PICKUP = 1,
    KF_MENU_MODE_SHOP = 2
KF_ENUM_END(KfMenuMode)

/* Shared signed controls; selectors additionally carry nonnegative IDs/rows. */
KF_ENUM_BEGIN(KfMenuResult, s32)
    KF_MENU_RESULT_PENDING = -99,
    KF_MENU_RESULT_GAME_LOADED = -3,
    KF_MENU_RESULT_RETURN_TO_INTRO = -2,
    KF_MENU_RESULT_CANCELLED = -1,
    KF_MENU_RESULT_ACCEPTED = 0,
    KF_MENU_RESULT_DECLINED = 1,
    KF_MENU_RESULT_STACK_FULL = 2
KF_ENUM_END(KfMenuResult)

KF_ENUM_BEGIN(KfMenuRootChoice, s32)
    KF_ROOT_CHOICE_NONE = -1,
    KF_ROOT_CHOICE_USE_ITEM = 0,
    KF_ROOT_CHOICE_USE_MAGIC = 1,
    KF_ROOT_CHOICE_EQUIPMENT = 2,
    KF_ROOT_CHOICE_STATUS = 3,
    KF_ROOT_CHOICE_DROP_ITEM = 4,
    KF_ROOT_CHOICE_SYSTEM = 5,
    KF_ROOT_CHOICE_CONFIG = 6
KF_ENUM_END(KfMenuRootChoice)

KF_ENUM_BEGIN(KfEquipmentMenuCategory, s32)
    KF_EQUIP_MENU_NONE = -1,
    KF_EQUIP_MENU_WEAPON = 0,
    KF_EQUIP_MENU_MAGIC = 1,
    KF_EQUIP_MENU_SHIELD = 2,
    KF_EQUIP_MENU_HEAD = 3,
    KF_EQUIP_MENU_BODY = 4,
    KF_EQUIP_MENU_ARM = 5,
    KF_EQUIP_MENU_LEG = 6,
    KF_EQUIP_MENU_ACCESSORY = 7
KF_ENUM_END(KfEquipmentMenuCategory)

enum {
    KF_MENU_ROOT_RETURN_ROW = KF_ENUM_ENCODE(s32, KF_ROOT_CHOICE_CONFIG) + 1,
    KF_MENU_ROOT_ROW_COUNT = KF_MENU_ROOT_RETURN_ROW + 1,
    KF_MENU_EQUIPMENT_RETURN_ROW = KF_ENUM_ENCODE(s32, KF_EQUIP_MENU_ACCESSORY) + 1,
    KF_MENU_EQUIPMENT_ROW_COUNT = KF_MENU_EQUIPMENT_RETURN_ROW + 1,
    KF_MENU_CONFIG_EFFECTS_ROW = 0,
    KF_MENU_CONFIG_MUSIC_ROW = 1,
    KF_MENU_CONFIG_GAUGES_ROW = 2,
    KF_MENU_CONFIG_COMPASS_ROW = 3,
    KF_MENU_CONFIG_SETTING_COUNT = 4,
    KF_MENU_CONFIG_RETURN_ROW = KF_MENU_CONFIG_SETTING_COUNT,
    KF_MENU_CONFIG_ROW_COUNT = KF_MENU_CONFIG_RETURN_ROW + 1
};

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

KF_ENUM_BEGIN(KfTradeMode, s32)
    KF_TRADE_NONE = -1,
    KF_TRADE_BUY = 0,
    KF_TRADE_SELL = 1
KF_ENUM_END(KfTradeMode)

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
    KF_SHOP_ROW_RETURN = 2,
    KF_SHOP_ROW_GOLD = 3,
    KF_SHOP_CHOICE_COUNT = KF_SHOP_ROW_RETURN + 1
};

/* Confirm-button state, independent of the selected option and its result. */
KF_ENUM_BEGIN(KfMenuConfirmState, s32)
    KF_MENU_CONFIRM_IDLE = 0,
    KF_MENU_CONFIRM_REQUESTED = 1
KF_ENUM_END(KfMenuConfirmState)

KF_ENUM_BEGIN(KfMenuConfirmChoice, s32)
    KF_MENU_CHOICE_ACCEPT = 0,
    KF_MENU_CHOICE_DECLINE = 1
KF_ENUM_END(KfMenuConfirmChoice)

static inline KfMenuResult menu_confirm_result_from_choice(KfMenuConfirmChoice choice)
{
    return KF_ENUM_DECODE(KfMenuResult, -KF_ENUM_ENCODE(s32, choice));
}

enum {
    MENU_CONFIRM_TEXT_X = 96,
    MENU_CONFIRM_ROW_STEP = 20
};

/* A selection transports either a shared control or an existing ID domain. */
#if KF_MODERN_TYPES
template <typename Value>
class KfMenuSelection {
public:
    constexpr KfMenuSelection(Value value) : value_(KF_ENUM_ENCODE(s32, value)) {}
    constexpr KfMenuSelection(KfMenuResult result) : value_(KF_ENUM_ENCODE(s32, result)) {}

    constexpr s32 encoded_value() const { return value_; }
    friend constexpr bool operator==(KfMenuSelection lhs, KfMenuSelection rhs)
    { return lhs.value_ == rhs.value_; }

private:
    s32 value_;
};

template <typename Integer, typename Value>
constexpr Integer kf_enum_encode(KfMenuSelection<Value> selection)
{
    return static_cast<Integer>(selection.encoded_value());
}

typedef KfMenuSelection<KfEffectKind> KfMagicPanelResult;
#else
typedef s32 KfMagicPanelResult;
#endif

/* Row positions in the loaded System, save and load windows. */
enum {
    KF_MENU_SYSTEM_RETURN_ROW = 2,
    KF_MENU_SYSTEM_ROW_COUNT = KF_MENU_SYSTEM_RETURN_ROW + 1,
    KF_MENU_SAVE_FORMAT_ROW = KF_SAVE_SLOT_COUNT,
    KF_MENU_SAVE_RETURN_ROW = KF_MENU_SAVE_FORMAT_ROW + 1,
    KF_MENU_SAVE_ROW_COUNT = KF_MENU_SAVE_RETURN_ROW + 1,
    KF_MENU_LOAD_RETURN_ROW = KF_SAVE_SLOT_COUNT,
    KF_MENU_LOAD_ROW_COUNT = KF_MENU_LOAD_RETURN_ROW + 1
};

KF_ENUM_BEGIN(KfMenuSystemAction, s32)
    KF_MENU_SYSTEM_ACTION_NONE = -1,
    KF_MENU_SYSTEM_ACTION_LOAD = 0,
    KF_MENU_SYSTEM_ACTION_QUIT = 1
KF_ENUM_END(KfMenuSystemAction)

/* Negative values suppress overlays; values at least ALL draw all three. */
KF_ENUM_BEGIN(KfSaveSlotOverlay, s32)
    KF_SAVE_OVERLAY_NONE = -1,
    KF_SAVE_OVERLAY_SKIP_FIRST = 0,
    KF_SAVE_OVERLAY_SKIP_SECOND = 1,
    KF_SAVE_OVERLAY_SKIP_THIRD = 2,
    KF_SAVE_OVERLAY_ALL = KF_SAVE_SLOT_COUNT
KF_ENUM_END(KfSaveSlotOverlay)

/* Zero-based TIM/Mddd. indices; menu_load_item_texture adds one to the ID. */
KF_ENUM_BEGIN(KfMenuTextureId, s32)
    KF_MENU_TEXTURE_NONE = 0xff,
    MENU_TEXTURE_LOADING_DATA = 0x67,
    MENU_TEXTURE_SAVING_DATA = 0x68,
    MENU_TEXTURE_FORMATTING_CARD = 0x69,
    MENU_TEXTURE_CONFIRM_CARD_FORMAT = 0x72,
    MENU_TEXTURE_POWER_OFF = 0x3e6
KF_ENUM_END(KfMenuTextureId)

/* Magic artwork uses the same zero-based index as its spell record. */
#if KF_MODERN_TYPES
constexpr KfMenuTextureId menu_texture_from_magic(KfEffectKind magic_id)
{
    return KF_ENUM_DECODE(KfMenuTextureId, KF_ENUM_ENCODE(u8, magic_id));
}
#else
#define menu_texture_from_magic(magic_id) (magic_id)
#endif

/* Feedback styles; the cursor cue is also reused for opening/config actions. */
KF_ENUM_BEGIN(KfMenuSoundCue, s32)
    MENU_SOUND_CURSOR = 0,
    MENU_SOUND_CONFIRM = 1,
    MENU_SOUND_CANCEL_OR_ERROR = 2
KF_ENUM_END(KfMenuSoundCue)

/* Signed glyph streams share a terminator; each atlas has its own blank. */
enum {
    MENU_TEXT_END = -1,
    MENU_TEXT_BLANK = 0xff,
    MENU_TEXT_GLYPH_MASK = 0x0fff,
    MENU_TEXT_DAKUTEN = 0x1000,
    MENU_TEXT_HANDAKUTEN = 0x2000,
    MENU_NUMBER_BLANK = 10,
    MENU_NUMBER_SLASH = 11,
    /* Screen pixels between consecutive digits in the numeric atlas. */
    MENU_NUMBER_ADVANCE = 7
};

/* Decimal field widths shared by the header and detailed statistics views. */
enum {
    MENU_STATS_VITAL_DIGITS = 4,
    MENU_STATS_VALUE_DIGITS = 6
};

/* Displayed class titles: physical tier rows, magic tier columns. */
enum {
    MENU_CLASS_MIDDLE_STAT_MIN = 40,
    MENU_CLASS_HIGH_STAT_MIN = 60,
    MENU_CLASS_MAGIC_TIER_COUNT = 3,
    MENU_CLASS_LABEL_GLYPHS = 4,
    MENU_CLASS_FIRST_GLYPH = 0x100
};

/* Shared fixed backdrop grid; packet extents still read the resource descriptor. */
enum {
    /* Fixed grid steps match the shipped STAT.DAT backdrop tile. */
    MENU_BACKDROP_COLUMN_STEP = 71,
    MENU_BACKDROP_ROW_STEP = 104,
    MENU_BACKDROP_TOP_Y = 16,
    MENU_BACKDROP_BOTTOM_Y = MENU_BACKDROP_TOP_Y + MENU_BACKDROP_ROW_STEP
};

/* Shared ordering-table buckets; preserve insertion order within each bucket. */
enum {
    MENU_MARKER_OT_DEPTH = 500,
    MENU_CONTENT_OT_DEPTH = 1000,
    MENU_WIDGET_OT_DEPTH = 2000,
    MENU_WINDOW_OT_DEPTH = 2900,
    MENU_BACKGROUND_OT_DEPTH = 3000,
    /* Zero-based: release held input after the third panel draw. */
    MENU_PANEL_INPUT_RELEASE_FRAME = 2
};

/* Screen-space anchor shared by menu labels and sprite blitters. */
typedef struct MenuPoint {
    s16 x;
    s16 y;
} MenuPoint;

enum {
    MENU_GLYPHS_PER_ROW = 10,
    MENU_WINDOW_ROW_CAPACITY = 10
};

/* One runtime-loaded menu name: ten glyph codes selected by item or spell ID. */
typedef struct MenuGlyphRow {
    s16 codes[MENU_GLYPHS_PER_ROW];
} MenuGlyphRow;

/*
 * Positioned menu text: screen origin followed by the usual ten-code label.
 * Capacity-specific workspaces use the same proven origin/code prefix.
 * The configuration draw ABI passes two complete halfword-aligned labels.
 */
typedef struct MenuGlyphString {
    MenuPoint position;
    MenuGlyphRow glyphs;
} MenuGlyphString;

/* One title and ten selectable labels in a runtime-loaded menu window. */
typedef struct MenuWindowLayout {
    MenuGlyphString title;
    MenuGlyphString rows[MENU_WINDOW_ROW_CAPACITY];
} MenuWindowLayout;

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
    s16 width;
    s16 height;
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

enum {
    MENU_LIST_TILE_BACKDROP,
    MENU_LIST_TILE_ROW,
    MENU_LIST_TILE_END,
    MENU_LIST_TILE_SELECTED,
    MENU_LIST_TILE_COUNT
};

enum {
    MENU_BACKGROUND_QUAD_COUNT = 4,
    MENU_DIALOG_QUAD_COUNT = 6
};

/* Complete first block loaded from COM\\STAT.DAT; GPU packets are mutable. */
typedef struct KfMenuAssets {
    POLY_FT4 background_quads[KF_DISPLAY_BUFFER_COUNT][MENU_BACKGROUND_QUAD_COUNT];
    POLY_FT4 mid_depth_quads[KF_DISPLAY_BUFFER_COUNT];
    POLY_FT4 foreground_quads[KF_DISPLAY_BUFFER_COUNT];
    POLY_F4 dialog_quads[KF_DISPLAY_BUFFER_COUNT][MENU_DIALOG_QUAD_COUNT];
    MenuSpriteDef number_atlas;
    MenuSpriteDef glyph_atlas;
    MenuTileSprite window_backdrop;
    MenuSpriteDef option_background;
    MenuSpriteDef option_highlight;
    MenuSpriteDef row_background;
    MenuSpriteDef row_confirmed_background;
    MenuTileSprite list_tiles[MENU_LIST_TILE_COUNT];
    MenuSpriteDef selection_cursor;
} KfMenuAssets;

/*
 * Scrollable menu-list state. The first 24 bytes are a positioned title,
 * followed by list geometry/cursor bytes and the row-glyph and optional
 * quantity sources consumed by the shared list renderer.
 */
typedef struct KfMenuList {
    MenuGlyphString title;
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

/* Navigation callers handle empty lists before stepping. */
static inline void menu_list_previous(KfMenuList *list)
{
    if (list->selected_index != 0) {
        list->selected_index--;
        if (list->cursor_row == 0)
            list->scroll_offset--;
        else
            list->cursor_row--;
    } else {
        list->selected_index = list->entry_count - 1;
        if (list->entry_count < list->visible_rows) {
            list->scroll_offset = 0;
            list->cursor_row = list->entry_count - 1;
        } else {
            list->scroll_offset = list->entry_count - list->visible_rows;
            list->cursor_row = list->visible_rows - 1;
        }
    }
}

static inline void menu_list_next(KfMenuList *list)
{
    if (list->selected_index < list->entry_count - 1) {
        list->selected_index++;
        if (list->cursor_row == list->visible_rows - 1)
            list->scroll_offset++;
        else
            list->cursor_row++;
    } else {
        list->selected_index = 0;
        list->scroll_offset = 0;
        list->cursor_row = 0;
    }
}

/* Angle units per preview draw; a full revolution is 4096 units. */
enum {
    MENU_ITEM_PREVIEW_YAW_STEP = 16,
    MENU_PICKUP_PREVIEW_YAW_STEP = 8
};

/* Shared inventory/shop model translation, in GTE transform units. */
enum {
    MENU_ITEM_PREVIEW_TRANSLATION_X = 560,
    MENU_ITEM_PREVIEW_TRANSLATION_Y = 140,
    MENU_ITEM_PREVIEW_TRANSLATION_Z = 1500
};

/* Inventory/shop text positions and row pitch are screen pixels. */
enum {
    MENU_ITEM_NAME_X = 174,
    MENU_ITEM_PREVIEW_NAME_Y = 36,
    MENU_ITEM_PREVIEW_LINE_HEIGHT = 18,
    MENU_ITEM_PREVIEW_QUANTITY_DIGITS = 2
};

extern SVECTOR menu_item_preview_rotation;
extern KfMenuAssets menu_assets;
extern MenuWindowLayout menu_window_layouts[KF_MENU_WINDOW_LAYOUT_COUNT];
extern MenuGlyphRow item_name_rows[KF_ITEM_COUNT];
extern MenuGlyphRow magic_name_rows[KF_MAGIC_PLAYER_COUNT];
extern u16 item_buy_prices[KF_ITEM_COUNT][KF_ITEM_SHOP_COUNT];
extern u16 item_sell_prices[KF_ITEM_COUNT][KF_ITEM_SHOP_COUNT];
KF_ENUM_BEGIN(KfMenuModelAllocation, s32)
    KF_MENU_MODEL_RELEASED = 0,
    KF_MENU_MODEL_ALLOCATED = 1
KF_ENUM_END(KfMenuModelAllocation)
extern KfMenuModelAllocation menu_item_model_allocation_pending;

extern void item_load_database(void);
extern void item_menu_root(KF_ENUM_PARAM(KfItemStockBank, s32) shop_bank);
extern KfMenuResult item_pickup_confirm(KF_ENUM_PARAM(KfObjectId, s32) item_id);
extern void menu_add_frame_quad(void);
extern void menu_add_marker_quad(void);
extern void menu_blit_sprite(
    const MenuSpriteDef *sprite, const MenuPoint *position);
extern void menu_blit_sprite_translucent(
    const MenuSpriteDef *sprite, const MenuPoint *position);
extern void menu_config_panel(void);
extern void menu_draw_dialog_frame(
    const KfSaveSlotSummary *summaries, KfSaveSlotOverlay slot_overlay);
extern void menu_draw_item_detail(
    KF_ENUM_PARAM(KfObjectId, s32) item_id, KF_ENUM_PARAM(KfItemStockBank, s32) shop_bank, KfTradeMode price_mode);
extern void menu_draw_item_name_frame(KF_ENUM_PARAM(KfObjectId, s32) item_id);
extern void menu_draw_number(
    const MenuSpriteDef *font, const MenuGlyphString *string);
extern void menu_draw_string(
    const MenuSpriteDef *font, const MenuGlyphString *string);
extern void menu_draw_name_list(void);
extern void menu_draw_stats_header(void);
extern void menu_draw_status_details(void);
extern void menu_draw_two_option(
    const MenuGlyphString *accept_label, const MenuGlyphString *decline_label,
    KfMenuConfirmChoice selected_choice, KfMenuConfirmState confirmation);
extern void menu_draw_window(KfMenuWindowKind window_kind, s32 count, s32 highlight, KfMenuConfirmState confirmation);
extern void menu_draw_window_backdrop(void);
extern void menu_format_number(
    s32 value, s32 count, KF_ENUM_PARAM(KfFormatPaddingMode, s32) padding_mode, s16 *out);
extern void menu_drop_item(void);
#if KF_MODERN_TYPES && !defined(KF_MENU_MODE_IMPLEMENTATION)
extern u32 menu_enter_mode(KfMenuMode menu_mode);
extern u32 menu_enter_mode(KfMenuMode menu_mode, KfObjectId item_id);
extern u32 menu_enter_mode(KfMenuMode menu_mode, KfItemStockBank shop_bank);
#else
extern u32 menu_enter_mode(KfMenuMode menu_mode, ...);
#endif
extern void menu_equip_select(KfEquipmentMenuCategory equipment_category);
extern void menu_frame_begin(void);
extern void menu_item_model_preview(KF_ENUM_PARAM(KfObjectId, s32) item_id);
extern void menu_list_init(KfMenuList *list, KfMenuWindowKind window_kind, s32 row);
#if KF_MODERN_TYPES && !defined(KF_MENU_LIST_IMPLEMENTATION)
extern KfMenuResult menu_list_interact(
    const KfMenuList *list, KfMenuConfirmKind confirm_kind, KfMenuPreviewMode preview_mode,
    KfObjectId item_id, KF_ENUM_PARAM(KfItemStockBank, u32) shop_bank, KfTradeMode price_mode);
extern KfMenuResult menu_list_interact(
    const KfMenuList *list, KfMenuConfirmKind confirm_kind, KfMenuPreviewMode preview_mode,
    KfEffectKind magic_id, KF_ENUM_PARAM(KfItemStockBank, u32) shop_bank, KfTradeMode price_mode);
#else
extern KfMenuResult menu_list_interact(
    const KfMenuList *list, KfMenuConfirmKind confirm_kind, KfMenuPreviewMode preview_mode,
    s32 preview_id, KF_ENUM_PARAM(KfItemStockBank, u32) shop_bank, KfTradeMode price_mode);
#endif
extern void menu_list_render(const KfMenuList *list);
extern KF_ENUM_PARAM(KfResourceLoadResult, u32) menu_load_item_model(KF_ENUM_PARAM(KfObjectId, s32) item_id);
extern KF_ENUM_PARAM(KfResourceLoadResult, u32) menu_load_item_texture(KfMenuTextureId texture_id);
extern void menu_release_item_model(void);
extern KfMenuResult menu_load_panel(void);
extern KfMagicPanelResult menu_magic_panel(void);
extern void menu_map_viewer(KF_ENUM_PARAM(KfObjectId, s32) item_id);
extern void menu_option_root(void);
extern void menu_play_input_sound(KfMenuSoundCue cue);
extern void menu_present_frame(void);
extern s32 menu_root(void);
extern void menu_save_confirm(void);
extern KfMenuResult menu_save_load_hub(void);
extern KfMenuResult menu_save_panel(void);
extern void menu_spell_select(void);
extern void menu_status_panel(void);
extern KfMenuResult menu_two_option_prompt(
    KfMenuWindowKind window_kind, s32 count, s32 highlight,
    const KfSaveSlotSummary *summaries);
extern void talk_show_dialogue_page(KF_ENUM_PARAM(KfFloorId, u8) floor, u8 stage, KF_ENUM_PARAM(KfCharacterId, s32) character_id, u8 page);

/* Requires the GAME graphics state; preserve reverse primitive insertion order. */
#define MENU_ENQUEUE_BACKGROUND() ( \
    AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH), \
        (void *)&menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][3]), \
    AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH), \
        (void *)&menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][2]), \
    AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH), \
        (void *)&menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][1]), \
    AddPrim((void *)(game_graphics_runtime.display_state.ordering_table + MENU_BACKGROUND_OT_DEPTH), \
        (void *)&menu_assets.background_quads[KF_ENUM_ENCODE(u8, game_graphics_runtime.display_state.buffer_index)][0]))

#endif
