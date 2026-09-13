#include <kf/null.h>
#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/input.h>
#include <kf/map_data.h>
#include <kf/item.h>
#include <kf/game_cd.h>
#include <psyq/libc.h>
#include <kf/game.h>

/* Shared menu primitives (frame begin/flush, item draw, input sound, poll). */

/* Item sub-panels dispatched by the item menu (defined below). */
void item_menu_buy(KF_ENUM_PARAM(KfItemStockBank, s32) shop_bank);
void item_menu_sell(KF_ENUM_PARAM(KfItemStockBank, s32) shop_bank);

enum {
    MENU_SHOP_VISIBLE_ROWS = 9,
    MENU_PICKUP_CONFIRM_TEXT_X = 60,
    MENU_PICKUP_CONFIRM_ACCEPT_Y = 26,
    MENU_PICKUP_CONFIRM_DECLINE_Y = MENU_PICKUP_CONFIRM_ACCEPT_Y + MENU_CONFIRM_ROW_STEP
};

DATA(0x800580e8, 0x390)
KfMenuAssets menu_assets;

DATA(0x80058478, 0x948)
MenuWindowLayout menu_window_layouts[KF_MENU_WINDOW_LAYOUT_COUNT];

DATA(0x80058dc0, 0x640)
MenuGlyphRow item_name_rows[KF_ITEM_COUNT];

DATA(0x80059400, 0xb4)
MenuGlyphRow magic_name_rows[KF_MAGIC_PLAYER_COUNT];

DATA(0x800594b8, 0x140)
u16 item_buy_prices[KF_ITEM_COUNT][KF_ITEM_SHOP_COUNT];

DATA(0x800595f8, 0x140)
u16 item_sell_prices[KF_ITEM_COUNT][KF_ITEM_SHOP_COUNT];

/*
 * Expands the map resource stream's floor-item placement chunk into the runtime
 * floor-item table.  The first pass counts the placements up to the 0xffff
 * terminator; the second pass converts each tile/offset pair into a world
 * position sunk onto the floor and seeds a random starting animation frame.
 */
ADDRESS(0x80020b4c, 0x1b0)
void item_load_floor_placements(KfFloorItemPlacement *placements)
{
    KfFloorItemPlacement *first_placement;
    KfFloorItem *item;

    game_graphics_runtime.floor_item_count = 0;
    first_placement = placements;
    while (placements++->base_sprite_index != KF_FLOOR_ITEM_END) {
        game_graphics_runtime.floor_item_count++;
    }

    item = game_graphics_runtime.floor_items;
    placements = first_placement;
    if (placements->base_sprite_index != KF_FLOOR_ITEM_END) {
        do {
            s32 height;

            item->base_sprite_index = placements->base_sprite_index;
            item->facing_and_frame_count = placements->facing_and_frame_count;
            item->unknown_03 = placements->unknown_03;
            item->position_x = map_placement_axis_position(placements->tile_x, placements->local_x);
            item->position_z = map_placement_axis_position(placements->tile_z, placements->local_z);
            height = map_floor_height_grid.cells[placements->tile_z][placements->tile_x] * KF_MAP_HEIGHT_STEP;
            item->position_y = placements->local_y - height;
            item->animation_frame =
                (rand() * KF_ENUM_ENCODE(u8, item->facing_and_frame_count)) >> KF_FLOOR_ITEM_INITIAL_FRAME_RANDOM_BITS;
            item++;
            placements++;
        } while (placements->base_sprite_index != KF_FLOOR_ITEM_END);
    }
}

RODATA(0x800122a0, 0x25)

/*
 * Loads the item database: COM\STAT.DAT is read into the six stat
 * banks, then all 80 item TMD models (\KF\ITEMx\Innn.TMD) are streamed into the
 * shared CD file table, whose read sizes are rounded up to whole sectors.
 */
ADDRESS(0x80020cfc, 0x5dc)
void item_load_database(void)
{
    char name[40] = "\\KF\\ITEM0\\I000.TMD;1";
    u8 *stat_data;
    u8 *src;
    s32 i;

    if (cd_file_load_allocated(&stat_data, "COM\\STAT.DAT") != KF_RESOURCE_LOADED)
        exit(1);

    src = stat_data;
    memcpy((void *)&menu_assets, (const void *)src, sizeof menu_assets);
    src += sizeof menu_assets;
    memcpy((void *)menu_window_layouts, (const void *)src, sizeof menu_window_layouts);
    src += sizeof menu_window_layouts;
    memcpy((void *)item_name_rows, (const void *)src, sizeof(item_name_rows));
    src += sizeof(item_name_rows);
    memcpy((void *)magic_name_rows, (const void *)src, sizeof(magic_name_rows));
    src += sizeof(magic_name_rows);
    memcpy((void *)item_buy_prices, (const void *)src, sizeof(item_buy_prices));
    src += sizeof(item_buy_prices);
    memcpy((void *)item_sell_prices, (const void *)src, sizeof(item_sell_prices));

    memory_release_last();

    for (i = 0; i < KF_ITEM_COUNT; i++) {
        s32 n = i + 1;
        s32 rem;

        name[8] = i / 30 + '1';
        name[11] = n / 100 + '0';
        rem = n % 100;
        name[12] = rem / 10 + '0';
        name[13] = rem % 10 + '0';
        if (CdSearchFile((CdlFILE *)&cd_file_table[i], name) != NULL) {
            if ((cd_file_table[i].size & (KF_CD_SECTOR_BYTES - 1)) != 0)
                cd_file_table[i].size =
                    ((cd_file_table[i].size >> KF_CD_SECTOR_SHIFT) + 1) << KF_CD_SECTOR_SHIFT;
        }
    }
}

/*
 * Item menu: a three-row selector (two item sub-panels plus an exit row).  The
 * cursor wraps with up/down, confirm opens the selected sub-panel, and cancel
 * or confirming the exit row leaves the menu.
 */
ADDRESS(0x800212d8, 0x260)
void item_menu_root(KF_ENUM_PARAM(KfItemStockBank, s32) shop_bank)
{
    s32 cursor = KF_ENUM_ENCODE(s32, KF_TRADE_BUY);
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    KfMenuResult phase = KF_MENU_RESULT_PENDING;
    KfTradeMode action = KF_TRADE_NONE;

    menu_frame_begin();
    menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, KF_ENUM_ENCODE(s32, KF_TRADE_BUY), KF_MENU_CONFIRM_IDLE);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, KF_ENUM_ENCODE(s32, KF_TRADE_BUY), KF_MENU_CONFIRM_IDLE);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, KF_ENUM_ENCODE(s32, KF_TRADE_BUY), KF_MENU_CONFIRM_IDLE);
    menu_play_input_sound(MENU_SOUND_CURSOR);
    while (PadRead(1) != 0)
        ;

    for (;;) {
        menu_present_frame();
        if (action != KF_TRADE_NONE
                || KF_ENUM_ENCODE(s32, phase) == KF_ENUM_ENCODE(s32, action)) {
            menu_frame_begin();
            menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }
        switch (action) {
        case KF_TRADE_BUY:
            item_menu_buy(shop_bank);
            break;
        case KF_TRADE_SELL:
            item_menu_sell(shop_bank);
            break;
        }
        action = KF_TRADE_NONE;
        if (phase != KF_MENU_RESULT_PENDING) {
            while (PadRead(1) != 0)
                ;
            return;
        }

        menu_frame_begin();
        confirm = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = PadRead(1);
        if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_ENUM_ENCODE(s32, KF_TRADE_BUY))
                cursor--;
            else
                cursor = KF_SHOP_ROW_RETURN;
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_SHOP_ROW_RETURN)
                cursor++;
            else
                cursor = KF_ENUM_ENCODE(s32, KF_TRADE_BUY);
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (cursor < KF_SHOP_ROW_RETURN)
                action = KF_ENUM_DECODE(KfTradeMode, cursor);
            else
                phase = KF_MENU_RESULT_CANCELLED;
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            phase = KF_MENU_RESULT_CANCELLED;
        }
        menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, cursor, confirm);
    }
}

/*
 * Buy panel: lists available items from shop bank `shop_bank` whose player stack
 * is below 99, then runs a windowed grid cursor. Confirm on an entry the player
 * can afford deducts its price from gold and adds the item to inventory.
 */
ADDRESS(0x80021538, 0x5c4)
void item_menu_buy(KF_ENUM_PARAM(KfItemStockBank, s32) shop_bank)
{
    KfMenuList ctx;
    s16 entries[KF_ITEM_COUNT][MENU_GLYPHS_PER_ROW];
    u8 available[KF_ITEM_COUNT];
    KfObjectId index[KF_ITEM_COUNT];
    u8 *inv;
    s32 slot;
    s32 found;
    s32 j;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_PENDING);

    while (PadRead(1) != 0)
        ;
    menu_list_init(&ctx, KF_MENU_WINDOW_SHOP, KF_ENUM_ENCODE(s32, KF_TRADE_BUY));

    inv = item_stock[KF_ENUM_ENCODE(s32, shop_bank)];
    found = 0;
    for (slot = KF_ENUM_ENCODE(s32, KF_ITEM_VERDITE); slot < KF_ITEM_COUNT; slot++) {
        if (inv[slot] != 0 && item_stock[KF_ENUM_ENCODE(u8, KF_ITEM_STOCK_PLAYER)][slot] < KF_ITEM_STACK_CAPACITY) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                entries[found][j] = item_name_rows[slot].codes[j];
            available[found] = inv[slot];
            index[found] = KF_ENUM_DECODE(KfObjectId, slot);
            found++;
        }
    }
    for (slot = 0; slot < KF_ENUM_ENCODE(s32, KF_ITEM_VERDITE); slot++) {
        if (inv[slot] != 0 && item_stock[KF_ENUM_ENCODE(u8, KF_ITEM_STOCK_PLAYER)][slot] < KF_ITEM_STACK_CAPACITY) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                entries[found][j] = item_name_rows[slot].codes[j];
            available[found] = inv[slot];
            index[found] = KF_ENUM_DECODE(KfObjectId, slot);
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
        if (menu_load_item_model(index[ctx.selected_index]) != KF_RESOURCE_LOADED)
            return;
        menu_draw_item_detail(index[ctx.selected_index], shop_bank, KF_TRADE_BUY);
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            if (menu_list_interact(&ctx, KF_MENU_CONFIRM_BUY,
                    KF_MENU_PREVIEW_ITEM_DETAIL, index[ctx.selected_index], shop_bank, KF_TRADE_BUY)
                    == KF_MENU_RESULT_CANCELLED)
                selection = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_PENDING);
            else
                selection = KF_ENUM_ENCODE(u8, index[ctx.selected_index]);
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (selection != KF_ENUM_ENCODE(s32, KF_MENU_RESULT_PENDING)) {
            while (PadRead(1) != 0)
                ;
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_CANCELLED);
            }
        } else if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_previous(&ctx);
            goto load_selected_model;
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_next(&ctx);
        load_selected_model:
            if (menu_load_item_model(index[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return;
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            if (player_state.gold
                    < item_buy_prices[KF_ENUM_ENCODE(u8, index[ctx.selected_index])][KF_ENUM_ENCODE(s32, shop_bank) - KF_ENUM_ENCODE(s32, KF_ITEM_STOCK_FIRST_SHOP)]) {
                menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            } else {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                confirm = KF_MENU_CONFIRM_REQUESTED;
            }
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_CANCELLED);
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_draw_item_detail(index[ctx.selected_index], shop_bank, KF_TRADE_BUY);
        menu_list_render(&ctx);
    }

    menu_release_item_model();
    if (selection != KF_ENUM_ENCODE(s32, KF_MENU_RESULT_CANCELLED)) {
        if (selection == KF_ENUM_ENCODE(s32, KF_ITEM_GOLD_CROSS))
            inv[KF_ENUM_ENCODE(u8, KF_ITEM_GOLD_CROSS)]--;
        player_state.gold -= item_buy_prices[selection][KF_ENUM_ENCODE(s32, shop_bank) - KF_ENUM_ENCODE(s32, KF_ITEM_STOCK_FIRST_SHOP)];
        item_stock[KF_ENUM_ENCODE(u8, KF_ITEM_STOCK_PLAYER)][selection]++;
    }
}

/*
 * Sell panel: lists player quantities, subtracting any copy that
 * is currently equipped so it cannot be sold.  Confirm on an entry removes one
 * copy from inventory and credits its sell price to gold.
 */
ADDRESS(0x80021afc, 0x500)
void item_menu_sell(KF_ENUM_PARAM(KfItemStockBank, s32) shop_bank)
{
    KfMenuList ctx;
    s16 entries[KF_ITEM_COUNT][MENU_GLYPHS_PER_ROW];
    u8 available[KF_ITEM_COUNT];
    KfObjectId index[KF_ITEM_COUNT];
    u8 *inv;
    s32 slot;
    s32 found;
    s32 j;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_PENDING);

    while (PadRead(1) != 0)
        ;
    menu_list_init(&ctx, KF_MENU_WINDOW_SHOP, KF_ENUM_ENCODE(s32, KF_TRADE_SELL));

    inv = item_stock[KF_ENUM_ENCODE(u8, KF_ITEM_STOCK_PLAYER)];
    found = 0;
    for (slot = 0; slot < KF_ENUM_ENCODE(s32, KF_ITEM_GOLD_CROSS); slot++) {
        if (inv[slot] != 0) {
            available[found] = inv[slot];
            if (PLAYER_ITEM_IS_EQUIPPED(slot))
                available[found]--;
            if (available[found] != 0) {
                for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                    entries[found][j] = item_name_rows[slot].codes[j];
                index[found] = KF_ENUM_DECODE(KfObjectId, slot);
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
        if (menu_load_item_model(index[ctx.selected_index]) != KF_RESOURCE_LOADED)
            return;
        menu_draw_item_detail(index[ctx.selected_index], shop_bank, KF_TRADE_SELL);
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            selection = KF_ENUM_ENCODE(s32, menu_list_interact(&ctx, KF_MENU_CONFIRM_SELL,
                    KF_MENU_PREVIEW_ITEM_DETAIL, index[ctx.selected_index], shop_bank, KF_TRADE_SELL));
            if (selection == KF_ENUM_ENCODE(s32, KF_MENU_RESULT_CANCELLED))
                selection = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_PENDING);
            else
                selection = KF_ENUM_ENCODE(u8, index[ctx.selected_index]);
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (selection != KF_ENUM_ENCODE(s32, KF_MENU_RESULT_PENDING)) {
            while (PadRead(1) != 0)
                ;
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_CANCELLED);
            }
        } else if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_previous(&ctx);
            if (menu_load_item_model(index[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return;
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_next(&ctx);
            if (menu_load_item_model(index[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return;
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = KF_ENUM_ENCODE(s32, KF_MENU_RESULT_CANCELLED);
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_draw_item_detail(index[ctx.selected_index], shop_bank, KF_TRADE_SELL);
        menu_list_render(&ctx);
    }

    menu_release_item_model();
    if (selection != KF_ENUM_ENCODE(s32, KF_MENU_RESULT_CANCELLED)) {
        inv[selection]--;
        player_state.gold += item_sell_prices[selection][KF_ENUM_ENCODE(s32, shop_bank) - KF_ENUM_ENCODE(s32, KF_ITEM_STOCK_FIRST_SHOP)];
    }
}

/*
 * Item-pickup confirmation for `item_id`: yes adds one copy (returns 0),
 * or leaves a full stack of 99 untouched (returns 2). Cancel/no returns 1.
 */
ADDRESS(0x80021ffc, 0x2b8)
KfMenuResult item_pickup_confirm(KF_ENUM_PARAM(KfObjectId, s32) item_id)
{
    MenuGlyphString accept_label;
    MenuGlyphString decline_label;
    KfMenuConfirmChoice choice = KF_MENU_CHOICE_ACCEPT;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    KfMenuResult result = KF_MENU_RESULT_PENDING;
    s32 stock_count;
    s32 prev;

    stock_count = item_stock[KF_ENUM_ENCODE(u8, KF_ITEM_STOCK_PLAYER)][KF_ENUM_ENCODE(s32, item_id)];
    if (menu_load_item_model(item_id) != KF_RESOURCE_LOADED)
        return KF_MENU_RESULT_DECLINED;

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
    menu_draw_item_name_frame(item_id);
    menu_draw_two_option(
        &accept_label,
        &decline_label, KF_MENU_CHOICE_ACCEPT, KF_MENU_CONFIRM_IDLE);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_item_name_frame(item_id);
    menu_draw_two_option(
        &accept_label,
        &decline_label, KF_MENU_CHOICE_ACCEPT, KF_MENU_CONFIRM_IDLE);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_item_name_frame(item_id);
    menu_draw_two_option(
        &accept_label,
        &decline_label, KF_MENU_CHOICE_ACCEPT, KF_MENU_CONFIRM_IDLE);
    menu_play_input_sound(MENU_SOUND_CURSOR);
    while (PadRead(1) != 0)
        ;

    for (;;) {
        menu_present_frame();
        if (result != KF_MENU_RESULT_PENDING) {
            menu_frame_begin();
            menu_draw_item_name_frame(item_id);
            menu_draw_two_option(
                &accept_label,
                &decline_label, choice, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
            break;
        }

        menu_frame_begin();
        prev = input;
        input = PadRead(1);
        if ((PAD_PRESSED(input, prev, PADLup))
                || (PAD_PRESSED(input, prev, PADLdown))) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (choice != KF_MENU_CHOICE_ACCEPT)
                choice = KF_MENU_CHOICE_ACCEPT;
            else
                choice = KF_MENU_CHOICE_DECLINE;
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (choice != KF_MENU_CHOICE_ACCEPT) {
                result = KF_MENU_RESULT_DECLINED;
            } else {
                result = KF_MENU_RESULT_STACK_FULL;
                if (stock_count != KF_ITEM_STACK_CAPACITY) {
                    item_stock[KF_ENUM_ENCODE(u8, KF_ITEM_STOCK_PLAYER)][KF_ENUM_ENCODE(s32, item_id)]++;
                    result = KF_MENU_RESULT_ACCEPTED;
                }
            }
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_MENU_RESULT_DECLINED;
        }
        menu_draw_item_name_frame(item_id);
        menu_draw_two_option(
            &accept_label,
            &decline_label, choice, confirm);
    }

    menu_release_item_model();
    return result;
}
