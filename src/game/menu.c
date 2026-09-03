#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/* Shared save workspace pointers (rebound to the caller's frame buffers). */

/* Player object and the 240-byte inventory / progress-flag block. */
extern u8 DAT_800652a8[240];

/* Item-name string tables (10 halfwords per label). */

/* Shared menu primitives: frame begin/flush, background draw, header draw,
 * input sound cue, and the vsync/pad poll. */
extern void menu_draw_dialog_frame(void *arg0, s32 arg1);
extern void menu_draw_window(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern u32 pad_read();

/* Item-list widget helpers (init, render, query). */
extern void menu_list_init(u16 *ctx, s32 arg1, s32 arg2);

/* Menu sub-panels dispatched by the hub menu. */
s32 menu_use_item_panel(void);

/*
 * Item-list display context: a shared menu list header with a visible-window
 * cursor over the label entries that follow it on the frame.
 */
typedef struct KfItemMenu {
    u8 unknown_00[26];
    u8 count;
    u8 page;
    u8 scroll;
    u8 cursor;
    u8 window;
    u8 rows;
    s16 *entries;
    s32 unknown_24;
} KfItemMenu;

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
        menu_draw_dialog_frame((void *)0, 3);
        menu_draw_window(4, 5, 0, 0);
        menu_present_frame();
    } while (i < 3);
    menu_play_input_sound(0);
    while (pad_read(1) != 0)
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
    while (pad_read(1) != 0)
        ;

    for (;;) {
        if (selection != -1 || result == selection) {
            menu_frame_begin();
            menu_draw_stats_header();
            menu_draw_window(0, 8, cursor, confirm);
            menu_present_frame();
            while (pad_read(1) != 0)
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
            result = -99;
            break;
        case 3:
            menu_status_panel();
            result = -99;
            break;
        case 4:
            menu_drop_item();
            result = -99;
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
            while (pad_read(1) != 0)
                ;
            return result;
        }
        selection = -1;
        confirm = 0;
        prev = input;
        input = pad_read(1);
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
    KfItemMenu ctx;
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

    while (pad_read(1) != 0)
        ;
    menu_list_init((u16 *)&ctx, 0, 0);

    inv = DAT_800652a8;
    found = 0;
    if (inv[0x37] != 0) {
        for (j = 0; j < 10; j++)
            labels[found][j] = DAT_8005920c[j];
        counts[found] = inv[0x37];
        codes[found] = 0x37;
        found++;
    }
    if (inv[0x49] != 0) {
        for (j = 0; j < 10; j++)
            labels[found][j] = DAT_80059374[j];
        counts[found] = inv[0x49];
        codes[found] = 0x49;
        found++;
    }
    name = DAT_80059108;
    for (code = 0x2a; code < 0x30; code++, name += 10) {
        if (code != 0x37 && code != 0x49 && inv[code] != 0) {
            for (j = 0; j < 10; j++)
                labels[found][j] = name[j];
            counts[found] = inv[code];
            codes[found] = code;
            found++;
        }
    }
    name = DAT_800591d0;
    for (code = 0x34; code < 0x50; code++, name += 10) {
        if (code != 0x37 && code != 0x49 && inv[code] != 0) {
            for (j = 0; j < 10; j++)
                labels[found][j] = name[j];
            counts[found] = inv[code];
            codes[found] = code;
            found++;
        }
    }
    ctx.count = found;
    ctx.rows = 10;
    ctx.entries = &labels[0][0];
    ctx.unknown_24 = (s32)counts;

    menu_frame_begin();
    if (ctx.count != 0) {
        if (menu_load_item_model(codes[ctx.cursor]) != 0)
            return -1;
        menu_item_model_preview(codes[ctx.cursor]);
    }
    menu_list_render((s16 *)&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == 1) {
            selection = -99;
            if (menu_list_interact((u32)&ctx, 0, 0, codes[ctx.cursor], 0, 0) != -1)
                selection = codes[ctx.cursor];
        }
        if (selection != -99) {
            confirm = 0;
            while (pad_read(1) != 0)
                ;
            break;
        }

        prev = input;
        input = pad_read(1);
        if ((input & 0x1000) != 0 && (prev & 0x1000) == 0) {
            menu_play_input_sound(0);
            if (ctx.cursor != 0) {
                ctx.cursor--;
                if (ctx.window != 0)
                    ctx.window--;
                else
                    ctx.scroll--;
            } else if (ctx.count < ctx.page) {
                ctx.cursor = ctx.count - 1;
                ctx.scroll = 0;
                ctx.window = ctx.count - 1;
            } else {
                ctx.scroll = ctx.count - ctx.page;
                ctx.window = ctx.page - 1;
            }
            if (menu_load_item_model(codes[ctx.cursor]) != 0)
                return -1;
        } else if ((input & 0x4000) != 0 && (prev & 0x4000) == 0) {
            menu_play_input_sound(0);
            if (ctx.cursor < ctx.count - 1) {
                ctx.cursor++;
                if (ctx.window != ctx.page - 1)
                    ctx.window++;
                else
                    ctx.scroll++;
            } else {
                ctx.cursor = 0;
                ctx.scroll = 0;
                ctx.window = 0;
            }
            if (menu_load_item_model(codes[ctx.cursor]) != 0)
                return -1;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            if (codes[ctx.cursor] == 0x37 || codes[ctx.cursor] == 0x49) {
                game_state_acknowledge_pending();
                menu_map_viewer(codes[ctx.cursor]);
                menu_play_input_sound(2);
                if (menu_load_item_model(codes[ctx.cursor]) != 0)
                    return -1;
            } else {
                confirm = 1;
            }
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            selection = -1;
        }

        menu_frame_begin();
        if (ctx.count != 0)
            menu_item_model_preview(codes[ctx.cursor]);
        menu_list_render((s16 *)&ctx);
    }

    game_state_acknowledge_pending();
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
