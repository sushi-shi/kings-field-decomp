#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/* Psy-Q LIBC. */
extern s32 rand(void);
extern void *memset();
extern void *memcpy();
extern void exit(s32 status);

/* libcd ISO9660 CdSearchFile; reclassified vendored (see functions_vendored.tsv). */
extern void *CdSearchFile(void *slot, char *filename);
extern KfCdFileEntry cd_file_table[80];

/* Item stat banks loaded contiguously from COM\STAT.DAT (opaque records). */
extern u8 DAT_800580e8[];
extern u8 DAT_80058dc0[];
extern u8 DAT_80059400[];
extern u8 DAT_800594b8[];
extern u8 DAT_800595f8[];

/* Shared menu primitives (frame begin/flush, item draw, input sound, poll). */
extern void menu_draw_window(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern u32 pad_read();

/* Item sub-panels dispatched by the item menu (defined below). */
void item_menu_buy(s32 arg);
void item_menu_sell(s32 arg);

extern u8 DAT_800652a8[240];

/*
 * Expands the map resource stream's floor-item placement chunk into the runtime
 * floor-item table.  The first pass counts the placements up to the 0xffff
 * terminator; the second pass converts each tile/offset pair into a world
 * position sunk onto the floor and seeds a random flicker value.
 */
ADDRESS(0x80020b4c, 0x1b0)
void item_load_floor_placements(KfFloorItemPlacement *placements)
{
    KfFloorItemPlacement *placement;
    KfFloorItem *item;
    u16 *count = &floor_item_count;

    placement = placements;
    *count = 0;
    if (placement->item_id != 0xffff) {
        placement++;
        do {
            (*count)++;
        } while (placement++->item_id != 0xffff);
    }

    item = floor_items;
    placement = placements;
    if (placement->item_id != 0xffff) {
        do {
            s32 height;

            item->item_id = placement->item_id;
            item->unknown_02 = placement->unknown_02;
            item->unknown_03 = placement->unknown_03;
            item->position_x = placement->tile_x * 2000 + placement->local_x;
            item->position_z = placement->tile_z * 2000 + placement->local_z;
            height = map_floor_height_grid[placement->tile_z][placement->tile_x] * 100;
            item->position_y = placement->local_y - height;
            item->flicker = (rand() * item->unknown_02) >> 15;
            item++;
            placement++;
        } while (placement->item_id != 0xffff);
    }
}

RODATA(0x800122a0, 0x28)

/*
 * Loads the item database: COM\STAT.DAT is read into the six contiguous stat
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
    memcpy(DAT_800580e8, src, 912);
    src += 912;
    memcpy(DAT_80058478, src, 2376);
    src += 2376;
    memcpy(DAT_80058dc0, src, 1600);
    src += 1600;
    memcpy(DAT_80059400, src, 180);
    src += 180;
    memcpy(DAT_800594b8, src, 320);
    src += 320;
    memcpy(DAT_800595f8, src, 320);

    memory_release_last();

    for (i = 0; i < 80; i++) {
        s32 n = i + 1;
        s32 rem;

        name[8] = i / 30 + '1';
        name[11] = n / 100 + '0';
        rem = n % 100;
        name[12] = rem / 10 + '0';
        name[13] = rem % 10 + '0';
        if (CdSearchFile(&cd_file_table[i], name) != 0) {
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
void item_menu_root(s32 arg)
{
    s32 cursor = 0;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 done = -99;
    s32 selection = -1;

    menu_frame_begin();
    menu_draw_window(7, 3, 0, 0);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_window(7, 3, 0, 0);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_window(7, 3, 0, 0);
    menu_play_input_sound(0);
    while (pad_read(1) != 0)
        ;

    for (;;) {
        menu_present_frame();
        if (selection != -1 || done == selection) {
            menu_frame_begin();
            menu_draw_window(7, 3, cursor, confirm);
            menu_present_frame();
            while (pad_read(1) != 0)
                ;
        }
        switch (selection) {
        case 0:
            item_menu_buy(arg);
            break;
        case 1:
            item_menu_sell(arg);
            break;
        }
        selection = -1;
        if (done != -99) {
            while (pad_read(1) != 0)
                ;
            return;
        }

        menu_frame_begin();
        confirm = 0;
        prev = input;
        input = pad_read(1);
        if ((input & 0x1000) != 0 && (prev & 0x1000) == 0) {
            menu_play_input_sound(0);
            if (cursor != 0)
                cursor--;
            else
                cursor = 2;
        } else if ((input & 0x4000) != 0 && (prev & 0x4000) == 0) {
            menu_play_input_sound(0);
            if (cursor != 2)
                cursor++;
            else
                cursor = 0;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            confirm = 1;
            if (cursor < 2)
                selection = cursor;
            else
                done = -1;
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            done = -1;
        }
        menu_draw_window(7, 3, cursor, confirm);
    }
}

/*
 * Buy panel: builds a display list of the affordable items held in inventory
 * page `arg`, then runs a windowed grid cursor.  Confirm on an entry the player
 * can afford deducts its price from gold and adds the item to inventory.
 */
ADDRESS(0x80021538, 0x5c4)
void item_menu_buy(s32 arg)
{
    KfMenuList ctx;
    s16 entries[80][10];
    u8 category[80];
    u8 index[80];
    u8 *inv;
    s32 slot;
    s32 found;
    s32 j;
    s32 input = 0;
    s32 prev;
    s32 confirm = 0;
    s32 selection = -99;

    while (pad_read(1) != 0)
        ;
    menu_list_init(&ctx, 7, 0);

    inv = &DAT_800652a8[arg * 80];
    found = 0;
    for (slot = 42; slot < 80; slot++) {
        if (inv[slot] != 0 && DAT_800652a8[slot] < 99) {
            for (j = 0; j < 10; j++)
                entries[found][j] = DAT_80059108[(slot - 42) * 10 + j];
            category[found] = inv[slot];
            index[found] = slot;
            found++;
        }
    }
    for (slot = 0; slot < 42; slot++) {
        if (inv[slot] != 0 && DAT_800652a8[slot] < 99) {
            for (j = 0; j < 10; j++)
                entries[found][j] = ((s16 *)DAT_80058dc0)[slot * 10 + j];
            category[found] = inv[slot];
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
        menu_draw_item_detail(index[ctx.selected_index], arg, 0);
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == 1) {
            selection = -99;
            if (menu_list_interact(&ctx, 3, 1, index[ctx.selected_index], arg, 0) != -1)
                selection = index[ctx.selected_index];
        }
        if (selection != -99) {
            confirm = 0;
            while (pad_read(1) != 0)
                ;
            break;
        }

        prev = input;
        input = pad_read(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(0);
                selection = -1;
            }
        } else if ((input & 0x1000) != 0 && (prev & 0x1000) == 0) {
            menu_play_input_sound(0);
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
        } else if ((input & 0x4000) != 0 && (prev & 0x4000) == 0) {
            menu_play_input_sound(0);
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
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            if (player_state.unknown_2c
                    < ((u16 *)DAT_800594b8)[index[ctx.selected_index] * 2 + arg - 1]) {
                menu_play_input_sound(2);
            } else {
                menu_play_input_sound(1);
                confirm = 1;
            }
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            selection = -1;
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_draw_item_detail(index[ctx.selected_index], arg, 0);
        menu_list_render(&ctx);
    }

    game_state_acknowledge_pending();
    if (selection != -1) {
        if (selection == 0x34)
            inv[52]--;
        player_state.unknown_2c -= ((u16 *)DAT_800594b8)[selection * 2 + arg - 1];
        DAT_800652a8[selection]++;
    }
}

/*
 * Sell panel: lists the items in inventory page `arg`, subtracting any copy that
 * is currently equipped so it cannot be sold.  Confirm on an entry removes one
 * copy from inventory and credits its sell price to gold.
 */
ADDRESS(0x80021afc, 0x500)
void item_menu_sell(s32 arg)
{
    KfMenuList ctx;
    s16 entries[80][10];
    u8 category[80];
    u8 index[80];
    u8 *inv;
    s32 slot;
    s32 found;
    s32 j;
    s32 input = 0;
    s32 prev;
    s32 confirm = 0;
    s32 selection = -99;

    while (pad_read(1) != 0)
        ;
    menu_list_init(&ctx, 7, 1);

    inv = DAT_800652a8;
    found = 0;
    for (slot = 0; slot < 52; slot++) {
        if (inv[slot] != 0) {
            category[found] = inv[slot];
            if (slot == player_state.equipped_weapon_id
                    || slot == player_state.equipped_shield_id
                    || slot == player_state.equipped_head_armor_id
                    || slot == player_state.equipped_body_armor_id
                    || slot == player_state.equipped_arm_armor_id
                    || slot == player_state.equipped_leg_armor_id
                    || slot == player_state.equipped_accessory_id)
                category[found]--;
            if (category[found] != 0) {
                for (j = 0; j < 10; j++)
                    entries[found][j] = ((s16 *)DAT_80058dc0)[slot * 10 + j];
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
        menu_draw_item_detail(index[ctx.selected_index], arg, 1);
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == 1) {
            selection = -99;
            if (menu_list_interact(&ctx, 4, 1, index[ctx.selected_index], arg, confirm)
                    != -1)
                selection = index[ctx.selected_index];
        }
        if (selection != -99) {
            confirm = 0;
            while (pad_read(1) != 0)
                ;
            break;
        }

        prev = input;
        input = pad_read(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(0);
                selection = -1;
            }
        } else if ((input & 0x1000) != 0 && (prev & 0x1000) == 0) {
            menu_play_input_sound(0);
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
        } else if ((input & 0x4000) != 0 && (prev & 0x4000) == 0) {
            menu_play_input_sound(0);
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
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            confirm = 1;
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            selection = -1;
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_draw_item_detail(index[ctx.selected_index], arg, 1);
        menu_list_render(&ctx);
    }

    game_state_acknowledge_pending();
    if (selection != -1) {
        inv[selection]--;
        player_state.unknown_2c += ((u16 *)DAT_800595f8)[selection * 2 + arg - 1];
    }
}

/*
 * Use-item confirmation dialog for inventory slot `arg`.  Presents a yes/no
 * prompt; choosing yes consumes one copy of a normal item (returns 0), leaves a
 * key item untouched (returns 2), and cancel/no returns 1.  Returns -99 when the
 * item cannot be used at all.
 */
ADDRESS(0x80021ffc, 0x2b8)
s32 item_use_confirm(s32 arg)
{
    s16 prompt[12];
    s16 options[12];
    s32 choice = 0;
    s32 confirm = 0;
    s32 input = 0;
    s32 result = -99;
    s32 item;
    s32 prev;

    item = DAT_800652a8[arg];
    if (menu_load_item_model(arg) != 0)
        return 1;

    prompt[0] = 0x3c;
    prompt[1] = 0x1a;
    prompt[2] = 0x53;
    prompt[3] = 0x6a;
    prompt[4] = -1;
    options[0] = 0x3c;
    options[1] = 0x2e;
    options[2] = 0x63;
    options[3] = 0x61;
    options[4] = 0x6a;
    options[5] = -1;

    menu_frame_begin();
    menu_draw_item_name_frame(arg);
    menu_draw_two_option(
        (const MenuGlyphString *)prompt,
        (const MenuGlyphString *)options, 0, 0);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_item_name_frame(arg);
    menu_draw_two_option(
        (const MenuGlyphString *)prompt,
        (const MenuGlyphString *)options, 0, 0);
    menu_present_frame();
    menu_frame_begin();
    menu_draw_item_name_frame(arg);
    menu_draw_two_option(
        (const MenuGlyphString *)prompt,
        (const MenuGlyphString *)options, 0, 0);
    menu_play_input_sound(0);
    while (pad_read(1) != 0)
        ;

    for (;;) {
        menu_present_frame();
        if (result != -99) {
            menu_frame_begin();
            menu_draw_item_name_frame(arg);
            menu_draw_two_option(
                (const MenuGlyphString *)prompt,
                (const MenuGlyphString *)options, choice, confirm);
            menu_present_frame();
            while (pad_read(1) != 0)
                ;
            break;
        }

        menu_frame_begin();
        prev = input;
        input = pad_read(1);
        if (((input & 0x1000) != 0 && (prev & 0x1000) == 0)
                || ((input & 0x4000) != 0 && (prev & 0x4000) == 0)) {
            menu_play_input_sound(0);
            if (choice != 0)
                choice = 0;
            else
                choice = 1;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            confirm = 1;
            if (choice != 0) {
                result = 1;
            } else {
                result = 2;
                if (item != 0x63) {
                    DAT_800652a8[arg]++;
                    result = 0;
                }
            }
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            result = 1;
        }
        menu_draw_item_name_frame(arg);
        menu_draw_two_option(
            (const MenuGlyphString *)prompt,
            (const MenuGlyphString *)options, choice, confirm);
    }

    game_state_acknowledge_pending();
    return result;
}
