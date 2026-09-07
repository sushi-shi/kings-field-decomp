#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

/* Player object and the double-buffered display state. */

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
    s32 found;
    s32 code;
    s32 j;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    s32 selection = -99;

    while (PadRead(1) != 0)
        ;
    menu_list_init(&ctx, KF_MENU_WINDOW_ROOT, KF_ENUM_ENCODE(s32, KF_ROOT_CHOICE_USE_MAGIC));

    found = 0;
    for (code = KF_MAGIC_HEALING; code < KF_ENUM_ENCODE(s32, KF_MAGIC_LIGHTNING_BOLT); code++) {
        if (magic_records[code].learned == KF_MAGIC_LEARNED) {
            for (j = 0; j < 10; j++)
                labels[found][j] = magic_name_rows[code].codes[j];
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
            if (menu_list_interact(&ctx, KF_MENU_CONFIRM_USE,
                    KF_MENU_PREVIEW_MAGIC_ICON, codes[ctx.selected_index], 0, KF_ITEM_PRICE_BUY)
                    == KF_MENU_CONFIRM_CANCELLED)
                selection = -99;
            else
                selection = codes[ctx.selected_index];
        }
        if (selection != -99) {
            while (PadRead(1) != 0)
                ;
            break;
        }

        menu_frame_begin();
        confirm = 0;
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
                return -1;
        } else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
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
                return -1;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = 1;
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = -1;
        }

        if (ctx.entry_count != 0)
            menu_add_marker_quad();
        menu_list_render(&ctx);
    }

    if (selection != -1) {
        if (player_state.vitals.current_mp < magic_records[selection].mp_cost)
            return selection;
        player_state.vitals.current_mp -= magic_records[selection].mp_cost;
        if (selection == KF_MAGIC_HEALING) {
            player_state.vitals.current_hp += player_state.magic;
        } else if (selection == KF_MAGIC_DISPOISON) {
            player_state.status_effect_flags &= KF_PLAYER_STATUS_CURSE | KF_PLAYER_STATUS_DARKNESS;
        } else if (selection == KF_MAGIC_RESIST_FIRE) {
            player_state.status_effect_flags |= KF_PLAYER_STATUS_FIRE_DEFENSE_BOOST;
            player_apply_fire_defense_boost();
        } else if (selection == KF_MAGIC_BLESS) {
            player_state.status_effect_flags &= KF_PLAYER_STATUS_POISON | KF_PLAYER_STATUS_SLOWED;
            player_state.vitals.current_hp += player_state.magic * 3;
        }
        if (player_state.vitals.current_hp > player_state.vitals.maximum_hp)
            player_state.vitals.current_hp = player_state.vitals.maximum_hp;
    }
    return selection;
}

/*
 * Option menu: a nine-row cursor (eight slots plus an exit row) that dispatches
 * each slot to menu_equip_select, except slot 1 (menu_spell_select) and slots 5/6 which
 * are blocked while the Full Plate is equipped.  Loops until the exit
 * row or cancel.
 */
ADDRESS(0x800236ac, 0x22c)
void menu_option_root(void)
{
    s32 cursor = 0;
    s32 confirm = 0;
    s32 input = 0;
    s32 prev;
    KfMenuPanelPhase phase = KF_MENU_PANEL_OPEN;
    KfEquipmentMenuCategory selection = KF_EQUIP_MENU_NONE;

    menu_frame_begin();
    menu_draw_name_list();
    menu_draw_window(KF_MENU_WINDOW_EQUIPMENT, KF_MENU_EQUIPMENT_ROW_COUNT, 0, 0);

    for (;;) {
        menu_present_frame();
        if (selection != KF_EQUIP_MENU_NONE || KF_ENUM_ENCODE(s32, phase) == KF_ENUM_ENCODE(s32, selection)) {
            menu_frame_begin();
            menu_draw_name_list();
            menu_draw_window(KF_MENU_WINDOW_EQUIPMENT, KF_MENU_EQUIPMENT_ROW_COUNT, cursor, confirm);
            menu_present_frame();
            while (PadRead(1) != 0)
                ;
        }
        switch (selection) {
        case KF_EQUIP_MENU_ARM:
        case KF_EQUIP_MENU_LEG:
            if (player_state.equipped_body_armor_id == KF_ITEM_FULL_PLATE) {
                menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
                break;
            }
            /* fallthrough */
        case KF_EQUIP_MENU_WEAPON:
        case KF_EQUIP_MENU_SHIELD:
        case KF_EQUIP_MENU_HEAD:
        case KF_EQUIP_MENU_BODY:
        case KF_EQUIP_MENU_ACCESSORY:
            menu_equip_select(selection);
            break;
        case KF_EQUIP_MENU_MAGIC:
            menu_spell_select();
            break;
        }
        selection = KF_EQUIP_MENU_NONE;
        if (phase != KF_MENU_PANEL_OPEN)
            return;
        menu_frame_begin();
        confirm = 0;
        prev = input;
        input = PadRead(1);
        if ((input & PADLup) != 0 && (prev & PADLup) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 0)
                cursor--;
            else
                cursor = KF_MENU_EQUIPMENT_RETURN_ROW;
        } else if ((input & PADLdown) != 0 && (prev & PADLdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_MENU_EQUIPMENT_RETURN_ROW)
                cursor++;
            else
                cursor = 0;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = 1;
            if (cursor < KF_MENU_EQUIPMENT_RETURN_ROW)
                selection = KF_ENUM_DECODE(KfEquipmentMenuCategory, cursor);
            else
                phase = KF_MENU_PANEL_CLOSED;
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            phase = KF_MENU_PANEL_CLOSED;
        }
        menu_draw_name_list();
        menu_draw_window(KF_MENU_WINDOW_EQUIPMENT, KF_MENU_EQUIPMENT_ROW_COUNT, cursor, confirm);
    }
}
