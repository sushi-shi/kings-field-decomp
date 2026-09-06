#include <kf/address.h>
#include <kf/map_data.h>
#include <kf/item.h>
#include <kf/game_cd.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

/* Shared menu primitives (frame begin/flush, item draw, input sound, poll). */

/* Item sub-panels dispatched by the item menu (defined below). */
void item_menu_buy(s32 shop_id);
void item_menu_sell(s32 shop_id);

DATA(0x800580e8, 0x390)
KfMenuAssets menu_assets;

DATA(0x80058478, 0x948)
MenuWindowLayout menu_window_layouts[9];

DATA(0x80058dc0, 0x640)
MenuGlyphRow item_name_rows[80];

DATA(0x80059400, 0xb4)
MenuGlyphRow magic_name_rows[9];

DATA(0x800594b8, 0x140)
u16 item_buy_prices[80][2];

DATA(0x800595f8, 0x140)
u16 item_sell_prices[80][2];

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
    u16 *count = &floor_item_count;

    *count = 0;
    first_placement = placements;
    while (placements++->item_id != KF_FLOOR_ITEM_END) {
        (*count)++;
    }

    item = floor_items;
    placements = first_placement;
    if (placements->item_id != KF_FLOOR_ITEM_END) {
        do {
            s32 height;

            item->item_id = placements->item_id;
            item->facing_and_frame_count = placements->facing_and_frame_count;
            item->unknown_03 = placements->unknown_03;
            item->position_x = placements->tile_x * KF_MAP_TILE_SIZE + placements->local_x;
            item->position_z = placements->tile_z * KF_MAP_TILE_SIZE + placements->local_z;
            height = map_floor_height_grid[placements->tile_z][placements->tile_x] * KF_MAP_HEIGHT_STEP;
            item->position_y = placements->local_y - height;
            item->animation_frame =
                (rand() * item->facing_and_frame_count) >> 15;
            item++;
            placements++;
        } while (placements->item_id != KF_FLOOR_ITEM_END);
    }
}

RODATA(0x800122a0, 0x28)

/*
 * Loads the item database: COM\STAT.DAT is read into the six stat
 * banks, then all 80 item TMD models (\KF\ITEMx\Innn.TMD) are streamed into the
 * shared CD file table, whose read sizes are rounded up to whole sectors.
 */
ADDRESS(0x80020cfc, 0x5dc)
void item_load_database(void)
{
    char name[40] = "\\KF\\ITEM0\\I000.TMD;1";
    void *stat_data;
    u8 *src;
    s32 i;

    if (cd_file_load_allocated(&stat_data, "COM\\STAT.DAT") != 0)
        exit(1);

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

    for (i = 0; i < 80; i++) {
        s32 n = i + 1;
        s32 rem;

        name[8] = i / 30 + '1';
        name[11] = n / 100 + '0';
        rem = n % 100;
        name[12] = rem / 10 + '0';
        name[13] = rem % 10 + '0';
        if (CdSearchFile((CdlFILE *)&cd_file_table[i], name) != 0) {
            if ((cd_file_table[i].size & 0x7ff) != 0)
                cd_file_table[i].size =
                    ((cd_file_table[i].size >> 11) + 1) << 11;
        }
    }
}

/*
 * Item menu: a three-row selector (two item sub-panels plus an exit row).  The
 * cursor wraps with up/down, confirm opens the selected sub-panel, and cancel
 * or confirming the exit row leaves the menu.
 */
ADDRESS(0x800212d8, 0x260)
void item_menu_root(s32 shop_id)
{
    s32 cursor = KF_SHOP_ROW_BUY;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 done = -99;
    s32 selection = -1;

    menu_frame_begin();
    menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, KF_SHOP_ROW_BUY, 0);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, KF_SHOP_ROW_BUY, 0);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, KF_SHOP_ROW_BUY, 0);
    menu_play_input_sound(MENU_SOUND_CURSOR);
    while (PadRead(1) != 0)
        ;

    for (;;) {
        menu_present_frame();
        if (selection != -1 || done == selection) {
            menu_frame_begin();
            menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }
        switch (selection) {
        case KF_SHOP_ROW_BUY:
            item_menu_buy(shop_id);
            break;
        case KF_SHOP_ROW_SELL:
            item_menu_sell(shop_id);
            break;
        }
        selection = -1;
        if (done != -99) {
            while (PadRead(1) != 0)
                ;
            return;
        }

        menu_frame_begin();
        confirm = 0;
        prev = input;
        input = PadRead(1);
        if ((input & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_SHOP_ROW_BUY)
                cursor--;
            else
                cursor = KF_SHOP_ROW_RETURN;
        } else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_SHOP_ROW_RETURN)
                cursor++;
            else
                cursor = KF_SHOP_ROW_BUY;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = 1;
            if (cursor < KF_SHOP_ROW_RETURN)
                selection = cursor;
            else
                done = -1;
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            done = -1;
        }
        menu_draw_window(KF_MENU_WINDOW_SHOP, KF_SHOP_CHOICE_COUNT, cursor, confirm);
    }
}

/*
 * Buy panel: lists available items from shop bank `shop_id` whose player stack
 * is below 99, then runs a windowed grid cursor. Confirm on an entry the player
 * can afford deducts its price from gold and adds the item to inventory.
 */
ADDRESS(0x80021538, 0x5c4)
void item_menu_buy(s32 shop_id)
{
    KfMenuList ctx;
    s16 entries[80][10];
    u8 available[80];
    u8 index[80];
    u8 *inv;
    s32 slot;
    s32 found;
    s32 j;
    s32 input = 0;
    s32 prev;
    s32 confirm = 0;
    s32 selection = -99;

    while (PadRead(1) != 0)
        ;
    menu_list_init(&ctx, KF_MENU_WINDOW_SHOP, KF_SHOP_ROW_BUY);

    inv = item_stock[shop_id];
    found = 0;
    for (slot = 42; slot < 80; slot++) {
        if (inv[slot] != 0 && item_stock[0][slot] < KF_ITEM_STACK_CAPACITY) {
            for (j = 0; j < 10; j++)
                entries[found][j] = item_name_rows[slot].codes[j];
            available[found] = inv[slot];
            index[found] = slot;
            found++;
        }
    }
    for (slot = 0; slot < 42; slot++) {
        if (inv[slot] != 0 && item_stock[0][slot] < KF_ITEM_STACK_CAPACITY) {
            for (j = 0; j < 10; j++)
                entries[found][j] = item_name_rows[slot].codes[j];
            available[found] = inv[slot];
            index[found] = slot;
            found++;
        }
    }
    ctx.entry_count = found;
    ctx.visible_rows = 9;
    ctx.glyphs_per_entry = 10;
    ctx.glyph_rows = &entries[0][0];
    ctx.quantities = 0;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_model(index[ctx.selected_index]) != 0)
            return;
        menu_draw_item_detail(index[ctx.selected_index], shop_id, KF_ITEM_PRICE_BUY);
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == 1) {
            selection = -99;
            if (menu_list_interact(&ctx, KF_MENU_CONFIRM_BUY,
                    KF_MENU_PREVIEW_ITEM_DETAIL, index[ctx.selected_index], shop_id, KF_ITEM_PRICE_BUY)
                    != KF_MENU_CONFIRM_CANCELLED)
                selection = index[ctx.selected_index];
        }
        if (selection != -99) {
            confirm = 0;
            while (PadRead(1) != 0)
                ;
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = -1;
            }
        } else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (ctx.selected_index != 0) {
                ctx.selected_index--;
                if (ctx.cursor_row != 0)
                    ctx.cursor_row--;
                else
                    ctx.scroll_offset--;
            } else if (ctx.entry_count < ctx.visible_rows) {
                ctx.selected_index = ctx.entry_count - 1;
                ctx.scroll_offset = 0;
                ctx.cursor_row = ctx.entry_count - 1;
            } else {
                ctx.scroll_offset = ctx.entry_count - ctx.visible_rows;
                ctx.cursor_row = ctx.visible_rows - 1;
            }
            if (menu_load_item_model(index[ctx.selected_index]) != 0)
                return;
        } else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (ctx.selected_index < ctx.entry_count - 1) {
                ctx.selected_index++;
                if (ctx.cursor_row != ctx.visible_rows - 1)
                    ctx.cursor_row++;
                else
                    ctx.scroll_offset++;
            } else {
                ctx.selected_index = 0;
                ctx.scroll_offset = 0;
                ctx.cursor_row = 0;
            }
            if (menu_load_item_model(index[ctx.selected_index]) != 0)
                return;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            if (player_state.gold
                    < item_buy_prices[index[ctx.selected_index]][shop_id - 1]) {
                menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            } else {
                menu_play_input_sound(MENU_SOUND_CONFIRM);
                confirm = 1;
            }
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = -1;
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_draw_item_detail(index[ctx.selected_index], shop_id, KF_ITEM_PRICE_BUY);
        menu_list_render(&ctx);
    }

    menu_release_item_model();
    if (selection != -1) {
        if (selection == KF_ITEM_GOLD_CROSS)
            inv[KF_ITEM_GOLD_CROSS]--;
        player_state.gold -= item_buy_prices[selection][shop_id - 1];
        item_stock[0][selection]++;
    }
}

/*
 * Sell panel: lists player quantities, subtracting any copy that
 * is currently equipped so it cannot be sold.  Confirm on an entry removes one
 * copy from inventory and credits its sell price to gold.
 */
ADDRESS(0x80021afc, 0x500)
void item_menu_sell(s32 shop_id)
{
    KfMenuList ctx;
    s16 entries[80][10];
    u8 available[80];
    u8 index[80];
    u8 *inv;
    s32 slot;
    s32 found;
    s32 j;
    s32 input = 0;
    s32 prev;
    s32 confirm = 0;
    s32 selection = -99;

    while (PadRead(1) != 0)
        ;
    menu_list_init(&ctx, KF_MENU_WINDOW_SHOP, KF_SHOP_ROW_SELL);

    inv = item_stock[0];
    found = 0;
    for (slot = 0; slot < KF_ITEM_GOLD_CROSS; slot++) {
        if (inv[slot] != 0) {
            available[found] = inv[slot];
            if (slot == player_state.equipped_weapon_id
                    || slot == player_state.equipped_shield_id
                    || slot == player_state.equipped_head_armor_id
                    || slot == player_state.equipped_body_armor_id
                    || slot == player_state.equipped_arm_armor_id
                    || slot == player_state.equipped_leg_armor_id
                    || slot == player_state.equipped_accessory_id)
                available[found]--;
            if (available[found] != 0) {
                for (j = 0; j < 10; j++)
                    entries[found][j] = item_name_rows[slot].codes[j];
                index[found] = slot;
                found++;
            }
        }
    }
    ctx.entry_count = found;
    ctx.visible_rows = 9;
    ctx.glyphs_per_entry = 10;
    ctx.glyph_rows = &entries[0][0];
    ctx.quantities = 0;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_model(index[ctx.selected_index]) != 0)
            return;
        menu_draw_item_detail(index[ctx.selected_index], shop_id, KF_ITEM_PRICE_SELL);
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == 1) {
            selection = -99;
            if (menu_list_interact(&ctx, KF_MENU_CONFIRM_SELL,
                    KF_MENU_PREVIEW_ITEM_DETAIL, index[ctx.selected_index], shop_id, KF_ITEM_PRICE_SELL)
                    != KF_MENU_CONFIRM_CANCELLED)
                selection = index[ctx.selected_index];
        }
        if (selection != -99) {
            confirm = 0;
            while (PadRead(1) != 0)
                ;
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = -1;
            }
        } else if ((input & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (ctx.selected_index != 0) {
                ctx.selected_index--;
                if (ctx.cursor_row != 0)
                    ctx.cursor_row--;
                else
                    ctx.scroll_offset--;
            } else if (ctx.entry_count < ctx.visible_rows) {
                ctx.selected_index = ctx.entry_count - 1;
                ctx.scroll_offset = 0;
                ctx.cursor_row = ctx.entry_count - 1;
            } else {
                ctx.scroll_offset = ctx.entry_count - ctx.visible_rows;
                ctx.cursor_row = ctx.visible_rows - 1;
            }
            if (menu_load_item_model(index[ctx.selected_index]) != 0)
                return;
        } else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (ctx.selected_index < ctx.entry_count - 1) {
                ctx.selected_index++;
                if (ctx.cursor_row != ctx.visible_rows - 1)
                    ctx.cursor_row++;
                else
                    ctx.scroll_offset++;
            } else {
                ctx.selected_index = 0;
                ctx.scroll_offset = 0;
                ctx.cursor_row = 0;
            }
            if (menu_load_item_model(index[ctx.selected_index]) != 0)
                return;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = 1;
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = -1;
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_draw_item_detail(index[ctx.selected_index], shop_id, KF_ITEM_PRICE_SELL);
        menu_list_render(&ctx);
    }

    menu_release_item_model();
    if (selection != -1) {
        inv[selection]--;
        player_state.gold += item_sell_prices[selection][shop_id - 1];
    }
}

/*
 * Item-pickup confirmation for `item_id`: yes adds one copy (returns 0),
 * or leaves a full stack of 99 untouched (returns 2). Cancel/no returns 1.
 */
ADDRESS(0x80021ffc, 0x2b8)
KfItemPickupResult item_pickup_confirm(s32 item_id)
{
    MenuGlyphString accept_label;
    MenuGlyphString decline_label;
    s32 choice = 0;
    s32 confirm = 0;
    s32 input = 0;
    KfItemPickupResult result = KF_ITEM_PICKUP_PENDING;
    s32 stock_count;
    s32 prev;

    stock_count = item_stock[0][item_id];
    if (menu_load_item_model(item_id) != 0)
        return KF_ITEM_PICKUP_NOT_ACQUIRED;

    accept_label.x = 0x3c;
    accept_label.y = 0x1a;
    accept_label.codes[0] = 0x53;
    accept_label.codes[1] = 0x6a;
    accept_label.codes[2] = MENU_TEXT_END;
    decline_label.x = 0x3c;
    decline_label.y = 0x2e;
    decline_label.codes[0] = 0x63;
    decline_label.codes[1] = 0x61;
    decline_label.codes[2] = 0x6a;
    decline_label.codes[3] = MENU_TEXT_END;

    menu_frame_begin();
    menu_draw_item_name_frame(item_id);
    menu_draw_two_option(
        &accept_label,
        &decline_label, 0, 0);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_item_name_frame(item_id);
    menu_draw_two_option(
        &accept_label,
        &decline_label, 0, 0);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_item_name_frame(item_id);
    menu_draw_two_option(
        &accept_label,
        &decline_label, 0, 0);
    menu_play_input_sound(MENU_SOUND_CURSOR);
    while (PadRead(1) != 0)
        ;

    for (;;) {
        menu_present_frame();
        if (result != KF_ITEM_PICKUP_PENDING) {
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
        if (((input & PADLup) != 0 && (prev & PADLup) == 0)
                || ((input & PADLdown) != 0 && (prev & PADLdown) == 0)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (choice != 0)
                choice = 0;
            else
                choice = 1;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = 1;
            if (choice != 0) {
                result = KF_ITEM_PICKUP_NOT_ACQUIRED;
            } else {
                result = KF_ITEM_PICKUP_STACK_FULL;
                if (stock_count != KF_ITEM_STACK_CAPACITY) {
                    item_stock[0][item_id]++;
                    result = KF_ITEM_PICKUP_ACQUIRED;
                }
            }
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            result = KF_ITEM_PICKUP_NOT_ACQUIRED;
        }
        menu_draw_item_name_frame(item_id);
        menu_draw_two_option(
            &accept_label,
            &decline_label, choice, confirm);
    }

    menu_release_item_model();
    return result;
}
