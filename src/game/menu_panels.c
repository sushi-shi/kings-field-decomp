#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/* Player object and the double-buffered display state. */

/* Twenty-byte magic records (learned flag + MP cost) and the spell-name
 * string table (10 halfwords per label). */
extern s16 DAT_80059400[];

/* Shared menu primitives: frame begin/flush, hub background, list-panel
 * background, input sound cue, and the vsync/pad poll. */

/* Cursor/list widget helpers (init, render, query). */

/* Sub-panel handlers dispatched by the option menu. */

/* menu_option_root selection dispatch table. */
RODATA(0x800122f0, 0x20)

/*
 * Magic panel: builds the list of learned spells (magic_records[0..3]) with
 * their names, runs the windowed cursor, and on confirm deducts the spell's
 * MP cost and applies its effect.  Returns the cast spell index, or -1 when
 * the panel is cancelled.
 */
ADDRESS(0x8002317c, 0x530)
s32 menu_magic_panel(void)
{
    KfMenuList ctx;
    s16 labels[10][10];
    u8 codes[16];
    s16 *name;
    s32 found;
    s32 code;
    s32 j;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 selection = -99;

    while (pad_read(1) != 0)
        ;
    menu_list_init(&ctx, 0, 1);

    found = 0;
    name = DAT_80059400;
    for (code = 0; code < 4; code++, name += 10) {
        if (magic_records[code].learned == 1) {
            for (j = 0; j < 10; j++)
                labels[found][j] = name[j];
            codes[found] = code;
            found++;
        }
    }
    ctx.entry_count = found;
    ctx.glyphs_per_entry = 10;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = 0;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_texture(codes[ctx.selected_index]) == 1)
            return -1;
        menu_add_marker_quad();
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == 1) {
            selection = -99;
            if (menu_list_interact(&ctx, 0, 2, codes[ctx.selected_index], 0, 0) != -1)
                selection = codes[ctx.selected_index];
        }
        if (selection != -99) {
            while (pad_read(1) != 0)
                ;
            break;
        }

        menu_frame_begin();
        confirm = 0;
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
            if (menu_load_item_texture(codes[ctx.selected_index]) == 1)
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
            if (menu_load_item_texture(codes[ctx.selected_index]) == 1)
                return -1;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            confirm = 1;
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            selection = -1;
        }

        if (ctx.entry_count != 0)
            menu_add_marker_quad();
        menu_list_render(&ctx);
    }

    if (selection == -1)
        return selection;
    if (player_state.vitals.current_mp < magic_records[selection].mp_cost)
        return selection;
    player_state.vitals.current_mp -= magic_records[selection].mp_cost;
    if (selection == 0) {
        player_state.vitals.current_hp += player_state.magic;
    } else if (selection == 1) {
        player_state.status_effect_flags &= 3;
    } else if (selection == 2) {
        player_state.status_effect_flags |= 0x10;
        player_status_apply_effect4();
    } else if (selection == 3) {
        player_state.status_effect_flags &= 0xc;
        player_state.vitals.current_hp += player_state.magic * 3;
    }
    if (player_state.vitals.current_hp > player_state.vitals.maximum_hp)
        player_state.vitals.current_hp = player_state.vitals.maximum_hp;
    return selection;
}

/*
 * Option menu: a nine-row cursor (eight slots plus an exit row) that dispatches
 * each slot to menu_equip_select, except slot 1 (menu_spell_select) and slots 5/6 which
 * are blocked while the special head-armour is equipped.  Loops until the exit
 * row or cancel.
 */
ADDRESS(0x800236ac, 0x22c)
void menu_option_root(void)
{
    s32 cursor = 0;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 result = -99;
    s32 selection = -1;

    menu_frame_begin();
    menu_draw_name_list();
    menu_draw_window(1, 9, 0, 0);

    for (;;) {
        menu_present_frame();
        if (selection != -1 || result == selection) {
            menu_frame_begin();
            menu_draw_name_list();
            menu_draw_window(1, 9, cursor, confirm);
            menu_present_frame();
            while (pad_read(1) != 0)
                ;
        }
        switch (selection) {
        case 5:
        case 6:
            if (player_state.equipped_head_armor_id == 0x15) {
                menu_play_input_sound(2);
                break;
            }
            /* fallthrough */
        case 0:
        case 2:
        case 3:
        case 4:
        case 7:
            menu_equip_select(selection);
            break;
        case 1:
            menu_spell_select();
            break;
        }
        if (result != -99)
            return;
        selection = -1;
        menu_frame_begin();
        confirm = 0;
        prev = input;
        input = pad_read(1);
        if ((input & 0x1000) != 0 && (prev & 0x1000) == 0) {
            menu_play_input_sound(0);
            if (cursor != 0)
                cursor--;
            else
                cursor = 8;
        } else if ((input & 0x4000) != 0 && (prev & 0x4000) == 0) {
            menu_play_input_sound(0);
            if (cursor != 8)
                cursor++;
            else
                cursor = 0;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            confirm = 1;
            if (cursor < 8)
                selection = cursor;
            else
                result = -1;
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            result = -1;
        }
        menu_draw_name_list();
        menu_draw_window(1, 9, cursor, confirm);
    }
}
