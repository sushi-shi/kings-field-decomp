#include <kf/address.h>
#define KF_MENU_LIST_IMPLEMENTATION
#include <kf/game_menu.h>
#include <kf/game.h>

enum {
    MENU_LIST_CONFIRM_ACCEPT_Y = 185,
    MENU_LIST_CONFIRM_DECLINE_Y = MENU_LIST_CONFIRM_ACCEPT_Y + MENU_CONFIRM_ROW_STEP
};

/* Confirm the current list entry using a two-option footer. Up/down toggles
 * the footer; confirm accepts its selected option, and cancel declines.
 * The final highlighted frame is presented before waiting for button release.
 */
ADDRESS(0x80028380, 0x354)
KfMenuConfirmResult menu_list_interact(
    const KfMenuList *list, KfMenuConfirmKind kind, KfMenuPreviewMode preview_mode,
    s32 item_id, KF_ENUM_PARAM(KfShopId, u32) shop_id, KfItemPriceMode price_mode)
{
    MenuGlyphString opt0;
    MenuGlyphString opt1;
    KfMenuConfirmChoice selected;
    KF_ENUM_STORAGE(KfMenuConfirmState, u32) highlight;
    u32 pad;
    u32 prev_pad;
    KfMenuConfirmResult result;

    selected = KF_MENU_CHOICE_ACCEPT;
    highlight = KF_MENU_CONFIRM_IDLE;
    pad = 0;
    result = KF_MENU_CONFIRM_PENDING;
    while (PadRead(1) != 0) {
    }

    opt0.position.x = MENU_CONFIRM_TEXT_X;
    opt0.position.y = MENU_LIST_CONFIRM_ACCEPT_Y;
    opt1.position.x = MENU_CONFIRM_TEXT_X;
    opt1.position.y = MENU_LIST_CONFIRM_DECLINE_Y;
    if (kind == KF_MENU_CONFIRM_USE) {
        opt0.glyphs.codes[0] = 0x72;
        opt0.glyphs.codes[1] = 0x42;
        opt0.glyphs.codes[2] = MENU_TEXT_END;
    } else if (kind == KF_MENU_CONFIRM_DROP) {
        opt0.glyphs.codes[0] = 0x75;
        opt0.glyphs.codes[1] = 0x52;
        opt0.glyphs.codes[2] = 0x6a;
        opt0.glyphs.codes[3] = MENU_TEXT_END;
    } else if (kind == KF_MENU_CONFIRM_YES_NO) {
        opt0.glyphs.codes[0] = 0x59;
        opt0.glyphs.codes[1] = 0x41;
        opt0.glyphs.codes[2] = MENU_TEXT_END;
    } else if (kind == KF_MENU_CONFIRM_BUY) {
        opt0.glyphs.codes[0] = 0x74;
        opt0.glyphs.codes[1] = 0x42;
        opt0.glyphs.codes[2] = MENU_TEXT_END;
    } else if (kind == KF_MENU_CONFIRM_SELL) {
        opt0.glyphs.codes[0] = 0x73;
        opt0.glyphs.codes[1] = 0x6a;
        opt0.glyphs.codes[2] = MENU_TEXT_END;
    } else {
        opt0.glyphs.codes[0] = 0x70;
        opt0.glyphs.codes[1] = 0x71;
        opt0.glyphs.codes[2] = MENU_TEXT_END;
    }
    if (kind == KF_MENU_CONFIRM_YES_NO) {
        opt1.glyphs.codes[0] = 0x41;
        opt1.glyphs.codes[1] = 0x41;
        opt1.glyphs.codes[2] = 0x43;
    } else {
        opt1.glyphs.codes[0] = 99;
        opt1.glyphs.codes[1] = 0x61;
        opt1.glyphs.codes[2] = 0x6a;
    }
    opt1.glyphs.codes[3] = MENU_TEXT_END;

    menu_frame_begin();
    if (preview_mode == KF_MENU_PREVIEW_ITEM_MODEL) {
        menu_item_model_preview(KF_ENUM_DECODE(KF_ENUM_PARAM(KfItemId, s32), item_id));
    } else if (preview_mode == KF_MENU_PREVIEW_ITEM_DETAIL) {
        menu_draw_item_detail(KF_ENUM_DECODE(KF_ENUM_PARAM(KfItemId, s32), item_id), shop_id, price_mode);
    } else if (preview_mode == KF_MENU_PREVIEW_MAGIC_ICON
            && item_id != KF_ENUM_ENCODE(s32, KF_MAGIC_NONE)) {
        menu_add_marker_quad();
    }
    menu_list_render(list);
    menu_draw_two_option(&opt0, &opt1, selected, highlight);
    menu_present_frame();
    do {
        if (result != KF_MENU_CONFIRM_PENDING) {
            menu_frame_begin();
            if (preview_mode == KF_MENU_PREVIEW_ITEM_MODEL) {
                menu_item_model_preview(KF_ENUM_DECODE(KF_ENUM_PARAM(KfItemId, s32), item_id));
            } else if (preview_mode == KF_MENU_PREVIEW_ITEM_DETAIL) {
                menu_draw_item_detail(KF_ENUM_DECODE(KF_ENUM_PARAM(KfItemId, s32), item_id), shop_id, price_mode);
            } else if (preview_mode == KF_MENU_PREVIEW_MAGIC_ICON
                    && item_id != KF_ENUM_ENCODE(s32, KF_MAGIC_NONE)) {
                menu_add_marker_quad();
            }
            menu_list_render(list);
            menu_draw_two_option(&opt0, &opt1, selected, highlight);
            menu_present_frame();
            while (PadRead(1) != 0) {
            }
            return result;
        }

        highlight = KF_MENU_CONFIRM_IDLE;
        menu_frame_begin();
        prev_pad = pad;
        pad = PadRead(1);
        if (((pad & PADLup) != 0 && (prev_pad & PADLup) == 0) ||
            ((pad & PADLdown) != 0 && (prev_pad & PADLdown) == 0)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (selected == KF_MENU_CHOICE_ACCEPT) {
                selected = KF_MENU_CHOICE_DECLINE;
            } else {
                selected = KF_MENU_CHOICE_ACCEPT;
            }
        } else if ((pad & PADRright) != 0 && (prev_pad & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            highlight = KF_MENU_CONFIRM_REQUESTED;
            result = menu_confirm_result_from_choice(selected);
        } else if ((pad & PADRdown) != 0 && (prev_pad & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_CONFIRM_CANCELLED;
        }
        if (preview_mode == KF_MENU_PREVIEW_ITEM_MODEL) {
            menu_item_model_preview(KF_ENUM_DECODE(KF_ENUM_PARAM(KfItemId, s32), item_id));
        } else if (preview_mode == KF_MENU_PREVIEW_ITEM_DETAIL) {
            menu_draw_item_detail(KF_ENUM_DECODE(KF_ENUM_PARAM(KfItemId, s32), item_id), shop_id, price_mode);
        } else if (preview_mode == KF_MENU_PREVIEW_MAGIC_ICON
                && item_id != KF_ENUM_ENCODE(s32, KF_MAGIC_NONE)) {
            menu_add_marker_quad();
        }
        menu_list_render(list);
        menu_draw_two_option(&opt0, &opt1, selected, highlight);
        menu_present_frame();
    } while (1);
}
