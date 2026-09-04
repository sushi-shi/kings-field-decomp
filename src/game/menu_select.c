#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/* Item-name and spell-name string tables (10 halfwords per label). The
 * equipment/item panels index DAT_80058dc0 by item id; the spell panel starts
 * at DAT_80059450 (the fifth spell name). */
extern s16 DAT_80058dc0[];

/* Shared menu primitives: frame begin/flush, input sound cue, vsync/pad poll,
 * and the deferred state acknowledgement. */

/* Item-list widget helpers (init, render, preview, query). */

/* Player equip/select operations. */

/* Two eight-entry jump tables for the equipment panel: the category-range
 * switch and the slot-write switch, both indexed by the object argument. */
RODATA(0x80012310, 0x40)

/*
 * Equipment-selection panel dispatched by the option menu.  The object index
 * chooses one equipment category: it selects the item-id range to list from
 * the owned-item block, runs the windowed cursor, and on confirm writes the
 * chosen id into the matching player slot and recomputes combat stats.  The
 * head-armour slot (object 4) additionally clears the arm/leg slots when the
 * special helm (id 0x15) is chosen.
 */
ADDRESS(0x800238d8, 0x5c4)
void menu_equip_select(s32 object)
{
    KfMenuList ctx;
    s16 labels[20][10];
    u8 codes[20];
    s16 *name;
    u8 *owned;
    s32 i;
    s32 j;
    s32 k;
    s32 start;
    s32 end;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 selection = -99;

    while (pad_read(1) != 0)
        ;

    switch (object) {
    case 0:
        start = 0x00;
        end = 0x0d;
        break;
    case 2:
        start = 0x1a;
        end = 0x20;
        break;
    case 3:
        start = 0x0d;
        end = 0x13;
        break;
    case 4:
        start = 0x13;
        end = 0x1a;
        break;
    case 5:
        start = 0x20;
        end = 0x23;
        break;
    case 6:
        start = 0x23;
        end = 0x27;
        break;
    case 7:
        start = 0x30;
        end = 0x34;
        break;
    }

    k = 0;
    owned = &DAT_800652a8[start];
    for (i = start; i < end; i++, owned++) {
        if (*owned != 0) {
            name = &DAT_80058dc0[i * 10];
            for (j = 0; j < 10; j++)
                labels[k][j] = name[j];
            codes[k] = i;
            k++;
        }
    }
    labels[k][0] = 0x59;
    labels[k][1] = 0x104c;
    labels[k][2] = 0x4c;
    labels[k][3] = -1;
    codes[k] = 0xff;
    k++;

    menu_list_init(&ctx, 1, object);
    ctx.entry_count = k;
    ctx.glyphs_per_entry = 10;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = 0;

    if (ctx.entry_count != 0) {
        if (menu_load_item_model(codes[ctx.selected_index]) != 0)
            return;
    }

    for (;;) {
        if (confirm == 1) {
            if (menu_list_interact(&ctx, 5, 0, codes[ctx.selected_index], 0, 0) == -1)
                selection = -99;
            else
                selection = codes[ctx.selected_index];
        }
        confirm = 0;
        if (selection != -99) {
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
                if (ctx.cursor_row == 0)
                    ctx.scroll_offset--;
                else
                    ctx.cursor_row--;
            } else {
                ctx.selected_index = ctx.entry_count - 1;
                if (ctx.entry_count < ctx.visible_rows) {
                    ctx.scroll_offset = 0;
                    ctx.cursor_row = ctx.entry_count - 1;
                } else {
                    ctx.scroll_offset = ctx.entry_count - ctx.visible_rows;
                    ctx.cursor_row = ctx.visible_rows - 1;
                }
            }
            if (menu_load_item_model(codes[ctx.selected_index]) != 0)
                return;
        } else if ((input & 0x4000) != 0 && (prev & 0x4000) == 0) {
            menu_play_input_sound(0);
            if (ctx.selected_index < ctx.entry_count - 1) {
                ctx.selected_index++;
                if (ctx.cursor_row == ctx.visible_rows - 1)
                    ctx.scroll_offset++;
                else
                    ctx.cursor_row++;
            } else {
                ctx.selected_index = 0;
                ctx.scroll_offset = 0;
                ctx.cursor_row = 0;
            }
            if (menu_load_item_model(codes[ctx.selected_index]) != 0)
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
            menu_item_model_preview(codes[ctx.selected_index]);
        menu_list_render(&ctx);
        menu_present_frame();
    }

    game_state_acknowledge_pending();
    if (selection != -1) {
        switch (object) {
        case 0:
            player_state.equipped_weapon_id = selection;
            player_equip_weapon((u8)selection);
            break;
        case 2:
            player_state.equipped_body_armor_id = selection;
            player_set_equipment_slot((u8)selection, 4);
            break;
        case 3:
            player_state.equipped_shield_id = selection;
            player_set_equipment_slot((u8)selection, 0);
            break;
        case 4:
            player_state.equipped_head_armor_id = selection;
            player_set_equipment_slot((u8)selection, 1);
            if (selection == 0x15) {
                player_state.equipped_arm_armor_id = 0xff;
                player_state.equipped_leg_armor_id = 0xff;
                player_set_equipment_slot(0xff, 2);
                player_set_equipment_slot(0xff, 3);
            }
            break;
        case 5:
            player_state.equipped_arm_armor_id = selection;
            player_set_equipment_slot((u8)selection, 2);
            break;
        case 6:
            player_state.equipped_leg_armor_id = selection;
            player_set_equipment_slot((u8)selection, 3);
            break;
        case 7:
            player_state.equipped_accessory_id = selection;
            player_set_equipment_slot((u8)selection, 5);
            break;
        }
    }
}

/*
 * Spell-selection panel dispatched by the option menu (slot 1).  Lists the
 * learned attack spells (magic records 4..8), runs the windowed cursor, and on
 * confirm stores the chosen spell as the active magic and resolves its record.
 */
ADDRESS(0x80023e9c, 0x470)
void menu_spell_select(void)
{
    KfMenuList ctx;
    s16 labels[20][10];
    u8 codes[20];
    s16 *name;
    s32 code;
    s32 j;
    s32 k;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 selection = -99;

    while (pad_read(1) != 0)
        ;

    k = 0;
    name = DAT_80059450;
    for (code = 4; code < 9; code++, name += 10) {
        if (magic_records[code].learned == 1) {
            for (j = 0; j < 10; j++)
                labels[k][j] = name[j];
            codes[k] = code;
            k++;
        }
    }
    labels[k][0] = 0x59;
    labels[k][1] = 0x104c;
    labels[k][2] = 0x4c;
    labels[k][3] = -1;
    codes[k] = 0xff;
    k++;

    menu_list_init(&ctx, 1, 1);
    ctx.entry_count = k;
    ctx.glyphs_per_entry = 10;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = 0;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_texture(codes[ctx.selected_index]) == 1)
            return;
        if (codes[ctx.selected_index] != 0xff)
            menu_add_marker_quad();
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == 1) {
            if (menu_list_interact(&ctx, 5, 2, codes[ctx.selected_index], 0, 0) == -1)
                selection = -99;
            else
                selection = ctx.selected_index;
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
                if (ctx.cursor_row == 0)
                    ctx.scroll_offset--;
                else
                    ctx.cursor_row--;
            } else {
                ctx.selected_index = ctx.entry_count - 1;
                if (ctx.entry_count < ctx.visible_rows) {
                    ctx.scroll_offset = 0;
                    ctx.cursor_row = ctx.entry_count - 1;
                } else {
                    ctx.scroll_offset = ctx.entry_count - ctx.visible_rows;
                    ctx.cursor_row = ctx.visible_rows - 1;
                }
            }
            if (menu_load_item_texture(codes[ctx.selected_index]) == 1)
                return;
        } else if ((input & 0x4000) != 0 && (prev & 0x4000) == 0) {
            menu_play_input_sound(0);
            if (ctx.selected_index < ctx.entry_count - 1) {
                ctx.selected_index++;
                if (ctx.cursor_row == ctx.visible_rows - 1)
                    ctx.scroll_offset++;
                else
                    ctx.cursor_row++;
            } else {
                ctx.selected_index = 0;
                ctx.scroll_offset = 0;
                ctx.cursor_row = 0;
            }
            if (menu_load_item_texture(codes[ctx.selected_index]) == 1)
                return;
        } else if ((input & 0x20) != 0 && (prev & 0x20) == 0) {
            menu_play_input_sound(1);
            confirm = 1;
        } else if ((input & 0x40) != 0 && (prev & 0x40) == 0) {
            menu_play_input_sound(2);
            selection = -1;
        }

        if (ctx.entry_count != 0) {
            if (codes[ctx.selected_index] != 0xff)
                menu_add_marker_quad();
        }
        menu_list_render(&ctx);
    }

    if (selection != -1) {
        player_state.selected_magic_id = codes[selection];
        player_select_magic(codes[selection]);
    }
}
