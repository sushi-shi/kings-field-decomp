#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

/* Menu sub-panels dispatched by the hub menu. */
s32 menu_use_item_panel(void);

/*
 * Points the shared save workspace at frame-local header/payload buffers and
 * shows a three-frame confirmation screen, blocking until input is released.
 */
ADDRESS(0x800222b4, 0x94)
void menu_save_confirm(void)
{
    KfSaveHeader header;
    KfSavePayload payload;
    s32 i;

    save_payload_buffer = &payload;
    save_header_buffer = &header;

    i = 0;
    do {
        i++;
        menu_frame_begin();
        menu_draw_dialog_frame(0, 3);
        menu_draw_window(4, 5, 0, 0);
        menu_present_frame();
    } while (i < 3);
    menu_play_input_sound(0);
    while (PadRead(1) != 0)
        ;
    menu_save_panel();
}

/*
 * Hub menu: an eight-row vertical cursor over seven selectable options plus an
 * exit row.  Confirm dispatches to the matching sub-panel; a panel that returns
 * a real value closes the hub and passes it up, one that returns -1 (cancel)
 * keeps the hub running.
 */
RODATA(0x800122c8, 0x1c)
ADDRESS(0x80022348, 0x2c0)
s32 menu_root(void)
{
    s32 cursor = 0;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 result = -99;
    s32 selection = -1;
    s32 i;

    i = 0;
    do {
        i++;
        menu_frame_begin();
        menu_draw_stats_header();
        menu_draw_window(0, 8, cursor, confirm);
        menu_present_frame();
    } while (i < 3);
    menu_play_input_sound(0);
    while (PadRead(1) != 0)
        ;

    for (;;) {
        if (selection != -1 || result == selection) {
            menu_frame_begin();
            menu_draw_stats_header();
            menu_draw_window(0, 8, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }
        switch (selection) {
        case 0:
            result = menu_use_item_panel();
            goto join_result;
        case 1:
            result = -1;
            if (menu_magic_panel() == -1)
                result = -99;
            break;
        case 2:
            menu_option_root();
            break;
        case 3:
            menu_status_panel();
            break;
        case 4:
            menu_drop_item();
            break;
        case 5:
            result = menu_save_load_hub();
        join_result:
            if (result == -1)
                result = -99;
            break;
        case 6:
            menu_config_panel();
            break;
        }
        if (result != -99) {
            selection = -1;
            while (PadRead(1) != 0)
                ;
            return result;
        }
        selection = -1;
        confirm = 0;
        prev = input;
        input = PadRead(1);
        if ((input & 0x1000) != 0 && (prev & 0x1000) == 0) {
            menu_play_input_sound(0);
            if (cursor != 0)
                cursor--;
            else
                cursor = 7;
        } else if ((input & 0x4000) != 0 && (prev & 0x4000) == 0) {
            menu_play_input_sound(0);
            if (cursor != 7)
                cursor++;
            else
                cursor = 0;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            confirm = 1;
            if (cursor < 7)
                selection = cursor;
            else
                result = -1;
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            result = -1;
        }
        menu_frame_begin();
        menu_draw_stats_header();
        menu_draw_window(0, 8, cursor, confirm);
        menu_present_frame();
    }
}

/*
 * Consumable-item panel: builds a scrollable list of the usable items the
 * player holds, runs the windowed cursor, and applies the selected item's
 * effect.  Restoratives (codes 0x2b..0x2f) heal HP/MP and clear status flags
 * in place; special items (0x37, 0x49) are handled by menu_map_viewer.  Returns
 * the chosen item code, or -1 when the item cannot be used.
 */
ADDRESS(0x80022608, 0x774)
s32 menu_use_item_panel(void)
{
    KfMenuList ctx;
    s16 labels[50][10];
    u8 counts[56];
    u8 codes[56];
    u8 *inv;
    s16 *name;
    s32 found;
    s32 code;
    s32 j;
    s32 input = 0;
    s32 prev;
    s32 confirm = 0;
    s32 selection = -99;

    while (PadRead(1) != 0)
        ;
    menu_list_init(&ctx, 0, 0);

    inv = DAT_800652a8;
    found = 0;
    if (inv[0x37] != 0) {
        for (j = 0; j < 10; j++)
            labels[found][j] = item_name_rows[0x37].codes[j];
        counts[found] = inv[0x37];
        codes[found] = 0x37;
        found++;
    }
    if (inv[0x49] != 0) {
        for (j = 0; j < 10; j++)
            labels[found][j] = item_name_rows[0x49].codes[j];
        counts[found] = inv[0x49];
        codes[found] = 0x49;
        found++;
    }
    name = item_name_rows[0x2a].codes;
    for (code = 0x2a; code < 0x30; code++, name += 10) {
        if (code != 0x37 && code != 0x49 && inv[code] != 0) {
            for (j = 0; j < 10; j++)
                labels[found][j] = name[j];
            counts[found] = inv[code];
            codes[found] = code;
            found++;
        }
    }
    name = item_name_rows[0x34].codes;
    for (code = 0x34; code < 0x50; code++, name += 10) {
        if (code != 0x37 && code != 0x49 && inv[code] != 0) {
            for (j = 0; j < 10; j++)
                labels[found][j] = name[j];
            counts[found] = inv[code];
            codes[found] = code;
            found++;
        }
    }
    ctx.entry_count = found;
    ctx.glyphs_per_entry = 10;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = counts;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_model(codes[ctx.selected_index]) != 0)
            return -1;
        menu_item_model_preview(codes[ctx.selected_index]);
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == 1) {
            selection = -99;
            if (menu_list_interact(&ctx, 0, 0, codes[ctx.selected_index], 0, 0) != -1)
                selection = codes[ctx.selected_index];
        }
        if (selection != -99) {
            confirm = 0;
            while (PadRead(1) != 0)
                ;
            break;
        }

        prev = input;
        input = PadRead(1);
        if ((input & 0x1000) != 0 && (prev & 0x1000) == 0) {
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
            if (menu_load_item_model(codes[ctx.selected_index]) != 0)
                return -1;
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
            if (menu_load_item_model(codes[ctx.selected_index]) != 0)
                return -1;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            if (codes[ctx.selected_index] == 0x37 || codes[ctx.selected_index] == 0x49) {
                menu_release_item_model();
                menu_map_viewer(codes[ctx.selected_index]);
                menu_play_input_sound(2);
                if (menu_load_item_model(codes[ctx.selected_index]) != 0)
                    return -1;
            } else {
                confirm = 1;
            }
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            selection = -1;
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_item_model_preview(codes[ctx.selected_index]);
        menu_list_render(&ctx);
    }

    menu_release_item_model();
    if ((u32)(selection - 0x2a) < 6) {
        inv[selection]--;
        if (selection == 0x2b) {
            player_state.vitals.current_hp += 25;
        } else if (selection == 0x2c) {
            player_state.vitals.current_hp += 10;
            player_state.status_effect_flags &= 0xb;
        } else if (selection == 0x2d) {
            player_state.vitals.current_hp += 80;
            player_state.status_effect_flags &= 0x3;
        } else if (selection == 0x2e) {
            player_state.vitals.current_hp += 150;
            player_state.status_effect_flags = 0;
        } else if (selection == 0x2f) {
            player_state.status_effect_flags = 0;
            player_state.vitals.current_hp += 300;
            player_state.vitals.current_mp = player_state.vitals.maximum_mp;
        }
        if (player_state.vitals.current_hp > player_state.vitals.maximum_hp)
            player_state.vitals.current_hp = player_state.vitals.maximum_hp;
        if (player_state.vitals.current_mp > player_state.vitals.maximum_mp)
            player_state.vitals.current_mp = player_state.vitals.maximum_mp;
    }
    return selection;
}
