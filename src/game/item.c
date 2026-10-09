#include <kf/lib/null.h>
#include <kf/game/graphics.h>

#include <kf/game/input.h>
#include <kf/lib/map_data.h>
#include <kf/lib/item.h>
#include <kf/game/cd.h>
#include <psyq/libc.h>
#include <kf/game/menu.h>
#include <kf/game/player.h>
#include <kf/lib/memory.h>
#include <psyq/pad.h>
#include <kf/lib/graphics.h>

void shop_menu_buy(s32 shop_bank);
void shop_menu_sell(s32 shop_bank);

enum {
    MENU_SHOP_VISIBLE_ROWS = 9,
    MENU_PICKUP_CONFIRM_TEXT_X = 60,
    MENU_PICKUP_CONFIRM_ACCEPT_Y = 26,
    MENU_PICKUP_CONFIRM_DECLINE_Y = MENU_PICKUP_CONFIRM_ACCEPT_Y + MENU_CONFIRM_ROW_STEP
};

KfMenuAssets menu_assets;

MenuWindowLayout menu_window_layouts[KF_MENU_WINDOW_LAYOUT_COUNT];

MenuGlyphRow item_name_rows[KF_ITEM_COUNT];

MenuGlyphRow magic_name_rows[KF_MAGIC_PLAYER_COUNT];

u16 item_buy_prices[KF_ITEM_COUNT][KF_ITEM_SHOP_COUNT];

u16 item_sell_prices[KF_ITEM_COUNT][KF_ITEM_SHOP_COUNT];

#include "../lib/floor_item_load.inc"

void item_load_database(void)
{
    char name[40] = "\\KF\\ITEM0\\I000.TMD;1";
    u8 *stat_data;
    u8 *src;
    s32 i;

    if (cd_file_load_allocated(&stat_data, "COM\\STAT.DAT") != KF_RESOURCE_LOADED) {
        exit(1);
    }

    src = stat_data;
    memcpy(&menu_assets, src, sizeof menu_assets);
    src += sizeof menu_assets;
    memcpy(menu_window_layouts, src, sizeof menu_window_layouts);
    src += sizeof menu_window_layouts;
    memcpy(item_name_rows, src, sizeof(item_name_rows));
    src += sizeof(item_name_rows);
    memcpy(magic_name_rows, src, sizeof(magic_name_rows));
    src += sizeof(magic_name_rows);
    memcpy(item_buy_prices, src, sizeof(item_buy_prices));
    src += sizeof(item_buy_prices);
    memcpy(item_sell_prices, src, sizeof(item_sell_prices));

    memory_release_last();

    for (i = 0; i < KF_ITEM_COUNT; i++) {
        s32 n = i + 1;
        s32 rem;

        name[8] = i / 30 + '1';
        name[11] = n / 100 + '0';
        rem = n % 100;
        name[12] = rem / 10 + '0';

        n = rem % 10 + '0';
        name[13] = n;
        if (CdSearchFile((CdlFILE *)&cd_file_table[i], name) != NULL) {
            if ((cd_file_table[i].size & (KF_CD_SECTOR_BYTES - 1)) != 0) {
                cd_file_table[i].size =
                    ((cd_file_table[i].size >> KF_CD_SECTOR_SHIFT) + 1) << KF_CD_SECTOR_SHIFT;
            }
        }
    }
}

void shop_menu_root(s32 shop_bank)
{
    s32 cursor = ((s32)(KF_TRADE_BUY));
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    KfMenuResult result = KF_MENU_RESULT_PENDING;
    KfTradeMode trade_mode = KF_TRADE_NONE;

    menu_frame_begin();
    menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, ((s32)(KF_TRADE_BUY)), KF_MENU_CONFIRM_IDLE);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, ((s32)(KF_TRADE_BUY)), KF_MENU_CONFIRM_IDLE);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, ((s32)(KF_TRADE_BUY)), KF_MENU_CONFIRM_IDLE);
    menu_play_input_sound(MENU_SOUND_CURSOR);
    while (PadRead(1) != 0) {
    }

    for (;;) {
        menu_present_frame();
        if (trade_mode != KF_TRADE_NONE
                || ((s32)(result)) == ((s32)(trade_mode))) {
            menu_frame_begin();
            menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0) {
            }
        }
        switch (trade_mode) {
        case KF_TRADE_BUY:
            shop_menu_buy(shop_bank);
            break;
        case KF_TRADE_SELL:
            shop_menu_sell(shop_bank);
            break;
        }
        trade_mode = KF_TRADE_NONE;
        if (result != KF_MENU_RESULT_PENDING) {
            while (PadRead(1) != 0) {
            }
            return;
        }

        menu_frame_begin();
        confirm = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = PadRead(1);
        if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != ((s32)(KF_TRADE_BUY))) {
                cursor--;
            } else {
                cursor = KF_SHOP_ROW_RETURN;
            }
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_SHOP_ROW_RETURN) {
                cursor++;
            } else {
                cursor = ((s32)(KF_TRADE_BUY));
            }
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (cursor < KF_SHOP_ROW_RETURN) {
                trade_mode = ((KfTradeMode)(cursor));
            } else {
                result = KF_MENU_RESULT_CANCELLED;
            }
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_CANCELLED;
        }
        menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, cursor, confirm);
    }
}

void shop_menu_buy(s32 shop_bank)
{
    KfMenuList ctx;
    s16 entries[KF_ITEM_COUNT][MENU_GLYPHS_PER_ROW];
    u8 available[KF_ITEM_COUNT];
    KfObjectId item_ids[KF_ITEM_COUNT];
    u8 *stock;
    s32 slot;
    s32 found;
    s32 j;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = ((s32)(KF_MENU_RESULT_PENDING));

    while (PadRead(1) != 0) {
    }
    menu_list_init(&ctx, KF_MENU_WINDOW_SHOP, ((s32)(KF_TRADE_BUY)));

    stock = item_stock[((s32)(shop_bank))];
    found = 0;
    for (slot = ((s32)(KF_ITEM_VERDITE)); slot < KF_ITEM_COUNT; slot++) {
        if (stock[slot] != 0 && item_stock[((u8)(KF_ITEM_STOCK_PLAYER))][slot] < KF_ITEM_STACK_CAPACITY) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++) {
                entries[found][j] = item_name_rows[slot].codes[j];
            }
            available[found] = stock[slot];
            item_ids[found] = ((KfObjectId)(slot));
            found++;
        }
    }
    for (slot = 0; slot < ((s32)(KF_ITEM_VERDITE)); slot++) {
        if (stock[slot] != 0 && item_stock[((u8)(KF_ITEM_STOCK_PLAYER))][slot] < KF_ITEM_STACK_CAPACITY) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++) {
                entries[found][j] = item_name_rows[slot].codes[j];
            }
            available[found] = stock[slot];
            item_ids[found] = ((KfObjectId)(slot));
            found++;
        }
    }
    ctx.entry_count = found;
    ctx.visible_rows = MENU_SHOP_VISIBLE_ROWS;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &entries[0][0];
    ctx.quantities = NULL;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED) {
            return;
        }
        menu_draw_item_detail(item_ids[ctx.selected_index], shop_bank, KF_TRADE_BUY);
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            if (menu_list_confirm(&ctx, KF_MENU_CONFIRM_BUY,
                    KF_MENU_PREVIEW_ITEM_DETAIL, item_ids[ctx.selected_index], shop_bank, KF_TRADE_BUY)
                    == KF_MENU_RESULT_CANCELLED) {
                selection = ((s32)(KF_MENU_RESULT_PENDING));
            } else {
                selection = ((u8)(item_ids[ctx.selected_index]));
            }
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (selection != ((s32)(KF_MENU_RESULT_PENDING))) {
            while (PadRead(1) != 0) {
            }
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = ((s32)(KF_MENU_RESULT_CANCELLED));
            }
        } else if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_previous(&ctx);
            goto load_selected_model;
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_next(&ctx);
        load_selected_model:
            if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED) {
                return;
            }
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            if (player_state.gold
                    < item_buy_prices[((u8)(item_ids[ctx.selected_index]))][((s32)(shop_bank)) - ((s32)(KF_ITEM_STOCK_FIRST_SHOP))]) {
                menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            } else {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                confirm = KF_MENU_CONFIRM_REQUESTED;
            }
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = ((s32)(KF_MENU_RESULT_CANCELLED));
        }

        menu_frame_begin();
        if (ctx.entry_count != 0) {
            menu_draw_item_detail(item_ids[ctx.selected_index], shop_bank, KF_TRADE_BUY);
        }
        menu_list_render(&ctx);
    }

    menu_release_item_model();
    if (selection != ((s32)(KF_MENU_RESULT_CANCELLED))) {
        if (selection == ((s32)(KF_ITEM_GOLD_CROSS))) {
            stock[((u8)(KF_ITEM_GOLD_CROSS))]--;
        }
        player_state.gold -= item_buy_prices[selection][((s32)(shop_bank)) - ((s32)(KF_ITEM_STOCK_FIRST_SHOP))];
        item_stock[((u8)(KF_ITEM_STOCK_PLAYER))][selection]++;
    }
}

void shop_menu_sell(s32 shop_bank)
{
    KfMenuList ctx;
    s16 entries[KF_ITEM_COUNT][MENU_GLYPHS_PER_ROW];
    u8 available[KF_ITEM_COUNT];
    KfObjectId item_ids[KF_ITEM_COUNT];
    u8 *stock;
    s32 slot;
    s32 found;
    s32 j;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = ((s32)(KF_MENU_RESULT_PENDING));

    while (PadRead(1) != 0) {
    }
    menu_list_init(&ctx, KF_MENU_WINDOW_SHOP, ((s32)(KF_TRADE_SELL)));

    stock = item_stock[((u8)(KF_ITEM_STOCK_PLAYER))];
    found = 0;
    for (slot = 0; slot < ((s32)(KF_ITEM_GOLD_CROSS)); slot++) {
        if (stock[slot] != 0) {
            available[found] = stock[slot];
            if (PLAYER_ITEM_IS_EQUIPPED(slot)) {
                available[found]--;
            }
            if (available[found] != 0) {
                for (j = 0; j < MENU_GLYPHS_PER_ROW; j++) {
                    entries[found][j] = item_name_rows[slot].codes[j];
                }
                item_ids[found] = ((KfObjectId)(slot));
                found++;
            }
        }
    }
    ctx.entry_count = found;
    ctx.visible_rows = MENU_SHOP_VISIBLE_ROWS;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &entries[0][0];
    ctx.quantities = NULL;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED) {
            return;
        }
        menu_draw_item_detail(item_ids[ctx.selected_index], shop_bank, KF_TRADE_SELL);
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            selection = ((s32)(menu_list_confirm(&ctx, KF_MENU_CONFIRM_SELL,
                    KF_MENU_PREVIEW_ITEM_DETAIL, item_ids[ctx.selected_index], shop_bank, KF_TRADE_SELL)));
            if (selection == ((s32)(KF_MENU_RESULT_CANCELLED))) {
                selection = ((s32)(KF_MENU_RESULT_PENDING));
            } else {
                selection = ((u8)(item_ids[ctx.selected_index]));
            }
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (selection != ((s32)(KF_MENU_RESULT_PENDING))) {
            while (PadRead(1) != 0) {
            }
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = ((s32)(KF_MENU_RESULT_CANCELLED));
            }
        } else if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_previous(&ctx);
            if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED) {
                return;
            }
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_next(&ctx);
            if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED) {
                return;
            }
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = ((s32)(KF_MENU_RESULT_CANCELLED));
        }

        menu_frame_begin();
        if (ctx.entry_count != 0) {
            menu_draw_item_detail(item_ids[ctx.selected_index], shop_bank, KF_TRADE_SELL);
        }
        menu_list_render(&ctx);
    }

    menu_release_item_model();
    if (selection != ((s32)(KF_MENU_RESULT_CANCELLED))) {
        stock[selection]--;
        player_state.gold += item_sell_prices[selection][((s32)(shop_bank)) - ((s32)(KF_ITEM_STOCK_FIRST_SHOP))];
    }
}

KfMenuResult item_pickup_confirm(s32 item_id)
{
    MenuGlyphString accept_label;
    MenuGlyphString decline_label;
    KfMenuConfirmChoice choice = KF_MENU_CHOICE_ACCEPT;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    KfMenuResult result = KF_MENU_RESULT_PENDING;
    s32 stock_count;
    s32 prev;

    stock_count = item_stock[((u8)(KF_ITEM_STOCK_PLAYER))][((s32)(item_id))];
    if (menu_load_item_model(item_id) != KF_RESOURCE_LOADED) {
        return KF_MENU_RESULT_DECLINED;
    }

    accept_label.position.x = MENU_PICKUP_CONFIRM_TEXT_X;
    accept_label.position.y = MENU_PICKUP_CONFIRM_ACCEPT_Y;

    accept_label.glyphs.codes[0] = 0x53;
    accept_label.glyphs.codes[1] = 0x6a;
    accept_label.glyphs.codes[2] = MENU_TEXT_END;
    decline_label.position.x = MENU_PICKUP_CONFIRM_TEXT_X;
    decline_label.position.y = MENU_PICKUP_CONFIRM_DECLINE_Y;

    decline_label.glyphs.codes[0] = 0x63;
    decline_label.glyphs.codes[1] = 0x61;
    decline_label.glyphs.codes[2] = 0x6a;
    decline_label.glyphs.codes[3] = MENU_TEXT_END;

    menu_frame_begin();
    menu_draw_pickup_preview(item_id);
    menu_draw_two_option(
        &accept_label,
        &decline_label, KF_MENU_CHOICE_ACCEPT, KF_MENU_CONFIRM_IDLE);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_pickup_preview(item_id);
    menu_draw_two_option(
        &accept_label,
        &decline_label, KF_MENU_CHOICE_ACCEPT, KF_MENU_CONFIRM_IDLE);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_pickup_preview(item_id);
    menu_draw_two_option(
        &accept_label,
        &decline_label, KF_MENU_CHOICE_ACCEPT, KF_MENU_CONFIRM_IDLE);
    menu_play_input_sound(MENU_SOUND_CURSOR);
    while (PadRead(1) != 0) {
    }

    for (;;) {
        menu_present_frame();
        if (result != KF_MENU_RESULT_PENDING) {
            menu_frame_begin();
            menu_draw_pickup_preview(item_id);
            menu_draw_two_option(
                &accept_label,
                &decline_label, choice, confirm);
            menu_present_frame();
            while (PadRead(1) != 0) {
            }
            break;
        }

        menu_frame_begin();
        prev = input;
        input = PadRead(1);
        if (PAD_PRESSED(input, prev, PADLup) || PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (choice != KF_MENU_CHOICE_ACCEPT) {
                choice = KF_MENU_CHOICE_ACCEPT;
            } else {
                choice = KF_MENU_CHOICE_DECLINE;
            }
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (choice != KF_MENU_CHOICE_ACCEPT) {
                result = KF_MENU_RESULT_DECLINED;
            } else {
                result = KF_MENU_RESULT_STACK_FULL;
                if (stock_count != KF_ITEM_STACK_CAPACITY) {
                    item_stock[((u8)(KF_ITEM_STOCK_PLAYER))][((s32)(item_id))]++;
                    result = KF_MENU_RESULT_ACCEPTED;
                }
            }
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_DECLINED;
        }
        menu_draw_pickup_preview(item_id);
        menu_draw_two_option(
            &accept_label,
            &decline_label, choice, confirm);
    }

    menu_release_item_model();
    return result;
}
