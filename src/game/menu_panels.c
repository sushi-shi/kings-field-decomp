#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/* Player object and the double-buffered display state. */

/* Twenty-byte magic records (learned flag + MP cost) and the spell-name
 * string table (10 halfwords per label). */
extern KfMagicRecord magic_records[24];
extern s16 DAT_80059400[];

/* Menu sprite bank streamed from COM\STAT.DAT: the map-viewer frame quads are
 * the leading double-buffered POLY_FT4 rows. */
extern POLY_FT4 DAT_800580e8[2][4];

/* Shared menu primitives: frame begin/flush, hub background, list-panel
 * background, input sound cue, and the vsync/pad poll. */
extern u32 pad_read();

/* Cursor/list widget helpers (init, render, query). */
extern void menu_list_init(u16 *ctx, s32 arg1, s32 arg2);
extern u32 menu_load_item_texture(s32 magic_id);
extern void menu_draw_window(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Sub-panel handlers dispatched by the option menu. */

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

RODATA(0x800122e4, 0x2c)

/*
 * Map-item viewer: loads MAP\M{1,2}{floor}. into the scratch primitive buffer,
 * uploads the TIM, then draws the map background quad, the player-position
 * marker, and the four cardinal frame quads through the double-buffered
 * ordering table until a button is pressed.
 */
ADDRESS(0x80022d7c, 0x400)
void menu_map_viewer(s32 item_code)
{
    s32 frame = 0;
    POLY_FT4 poly_bg[2];
    POLY_FT4 poly_marker[2];
    char path[16] = "MAP\\M00.";
    u8 *buffer;
    s32 map_number;

    map_number = 2;
    if (item_code == 0x37)
        map_number = 1;
    path[5] = map_number + '0';
    path[6] = player_state.progress_state.current_floor + '0';

    buffer = display_state.primitive_buffer->cursor;
    if (cd_file_load_into(buffer, path) != 0)
        return;
    tim_upload_images((u_long *)buffer);

    SetPolyFT4(&poly_bg[0]);
    SetSemiTrans(&poly_bg[0], 1);
    poly_bg[0].r0 = 0x40;
    poly_bg[0].g0 = 0x40;
    poly_bg[0].b0 = 0x40;
    poly_bg[0].clut = 0x7d40;
    poly_bg[0].tpage = 0x1f;
    poly_bg[0].u0 = 0;
    poly_bg[0].v0 = 0;
    poly_bg[0].u1 = 0xdb;
    poly_bg[0].v1 = 0;
    poly_bg[0].u2 = 0;
    poly_bg[0].v2 = 0xdb;
    poly_bg[0].u3 = 0xdb;
    poly_bg[0].v3 = 0xdb;
    poly_bg[0].x0 = 0x32;
    poly_bg[0].y0 = 0xa;
    poly_bg[0].x1 = 0x10d;
    poly_bg[0].y1 = 0xa;
    poly_bg[0].x2 = 0x32;
    poly_bg[0].y2 = 0xe5;
    poly_bg[0].x3 = 0x10d;
    poly_bg[0].y3 = 0xe5;
    poly_bg[1] = poly_bg[0];

    SetPolyFT4(&poly_marker[0]);
    poly_marker[0].r0 = 0x40;
    poly_marker[0].g0 = 0x40;
    poly_marker[0].b0 = 0x40;
    poly_marker[0].clut = 0x7c40;
    poly_marker[0].tpage = 0x1b;
    poly_marker[0].u0 = 0;
    poly_marker[0].v0 = 0;
    poly_marker[0].u1 = 4;
    poly_marker[0].v1 = 0;
    poly_marker[0].u2 = 0;
    poly_marker[0].v2 = 4;
    poly_marker[0].u3 = 4;
    poly_marker[0].v3 = 4;
    poly_marker[0].x0 = player_state.map_cell.x * 2 + 58;
    poly_marker[0].y0 = 0xd8 - player_state.map_cell.z * 2;
    poly_marker[0].x1 = player_state.map_cell.x * 2 + 62;
    poly_marker[0].y1 = 0xd8 - player_state.map_cell.z * 2;
    poly_marker[0].x2 = player_state.map_cell.x * 2 + 58;
    poly_marker[0].y2 = 0xdc - player_state.map_cell.z * 2;
    poly_marker[0].x3 = player_state.map_cell.x * 2 + 62;
    poly_marker[0].y3 = 0xdc - player_state.map_cell.z * 2;
    poly_marker[1] = poly_marker[0];

    for (;;) {
        menu_frame_begin();
        AddPrim(display_state.ordering_table + 500,
                &poly_marker[display_state.buffer_index]);
        AddPrim(display_state.ordering_table + 1000,
                &poly_bg[display_state.buffer_index]);
        AddPrim(display_state.ordering_table + 3000,
                &DAT_800580e8[display_state.buffer_index][3]);
        AddPrim(display_state.ordering_table + 3000,
                &DAT_800580e8[display_state.buffer_index][2]);
        AddPrim(display_state.ordering_table + 3000,
                &DAT_800580e8[display_state.buffer_index][1]);
        AddPrim(display_state.ordering_table + 3000,
                &DAT_800580e8[display_state.buffer_index][0]);
        menu_present_frame();
        if (frame < 2) {
            frame++;
        } else if (frame == 2) {
            while (pad_read(1) != 0)
                ;
            frame++;
        } else {
            if (pad_read(1) == 0)
                continue;
            while (pad_read(1) != 0)
                ;
            return;
        }
    }
}

/*
 * Magic panel: builds the list of learned spells (magic_records[0..3]) with
 * their names, runs the windowed cursor, and on confirm deducts the spell's
 * MP cost and applies its effect.  Returns the cast spell index, or -1 when
 * the panel is cancelled.
 */
ADDRESS(0x8002317c, 0x530)
s32 menu_magic_panel(void)
{
    KfItemMenu ctx;
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
    menu_list_init((u16 *)&ctx, 0, 1);

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
    ctx.count = found;
    ctx.rows = 10;
    ctx.entries = &labels[0][0];
    ctx.unknown_24 = 0;

    menu_frame_begin();
    if (ctx.count != 0) {
        if (menu_load_item_texture(codes[ctx.cursor]) == 1)
            return -1;
        menu_add_marker_quad();
    }
    menu_list_render((s16 *)&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == 1) {
            selection = -99;
            if (menu_list_interact((u32)&ctx, 0, 2, codes[ctx.cursor], 0, 0) != -1)
                selection = codes[ctx.cursor];
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
        if (ctx.count == 0) {
            if (input != 0) {
                menu_play_input_sound(0);
                selection = -1;
            }
        } else if ((input & 0x1000) != 0 && (prev & 0x1000) == 0) {
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
            if (menu_load_item_texture(codes[ctx.cursor]) == 1)
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
            if (menu_load_item_texture(codes[ctx.cursor]) == 1)
                return -1;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            confirm = 1;
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            selection = -1;
        }

        if (ctx.count != 0)
            menu_add_marker_quad();
        menu_list_render((s16 *)&ctx);
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
