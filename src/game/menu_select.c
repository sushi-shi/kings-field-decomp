#include <kf/address.h>
#include <kf/game_menu.h>
#include <kf/game.h>

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
 * body-armor slot additionally clears arm/leg equipment when Full Plate is
 * chosen, since it occupies those equipment categories too.
 */
ADDRESS(0x800238d8, 0x5c4)
void menu_equip_select(KfEquipmentMenuCategory category)
{
    KfMenuList ctx;
    s16 labels[20][MENU_GLYPHS_PER_ROW];
    KfItemId codes[20];
    s16 *name;
    u8 *owned;
    s32 i;
    s32 j;
    s32 k;
    s32 start;
    s32 end;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = KF_MENU_LIST_PENDING;

    while (PadRead(1) != 0)
        ;

    owned = item_stock[KF_ITEM_STOCK_PLAYER];
    switch (category) {
    case KF_EQUIP_MENU_WEAPON:
        start = KF_WEAPON_ITEM_FIRST;
        end = KF_WEAPON_ITEM_END;
        break;
    case KF_EQUIP_MENU_SHIELD:
        start = KF_SHIELD_ITEM_FIRST;
        end = KF_SHIELD_ITEM_END;
        break;
    case KF_EQUIP_MENU_HEAD:
        start = KF_HEAD_ARMOR_ITEM_FIRST;
        end = KF_HEAD_ARMOR_ITEM_END;
        break;
    case KF_EQUIP_MENU_BODY:
        start = KF_BODY_ARMOR_ITEM_FIRST;
        end = KF_BODY_ARMOR_ITEM_END;
        break;
    case KF_EQUIP_MENU_ARM:
        start = KF_ARM_ARMOR_ITEM_FIRST;
        end = KF_ARM_ARMOR_ITEM_END;
        break;
    case KF_EQUIP_MENU_LEG:
        start = KF_LEG_ARMOR_ITEM_FIRST;
        end = KF_LEG_ARMOR_ITEM_END;
        break;
    case KF_EQUIP_MENU_ACCESSORY:
        start = KF_ACCESSORY_ITEM_FIRST;
        end = KF_ACCESSORY_ITEM_END;
        break;
    }

    k = 0;
    for (i = start; i < end; i++) {
        if (owned[i] != 0) {
            name = item_name_rows[i].codes;
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                labels[k][j] = name[j];
            codes[k] = KF_ENUM_DECODE(KfItemId, i);
            k++;
        }
    }
    labels[k][0] = 0x59;
    labels[k][1] = MENU_TEXT_DAKUTEN | 0x4c;
    labels[k][2] = 0x4c;
    labels[k][3] = MENU_TEXT_END;
    codes[k] = KF_ITEM_NONE;
    k++;

    menu_list_init(&ctx, KF_MENU_WINDOW_EQUIPMENT, KF_ENUM_ENCODE(s32, category));
    ctx.entry_count = k;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = 0;

    if (ctx.entry_count != 0) {
        if (menu_load_item_model(codes[ctx.selected_index]) != KF_RESOURCE_LOADED)
            return;
    }

    for (;;) {
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            if (menu_list_interact(&ctx, KF_MENU_CONFIRM_EQUIP,
                    KF_MENU_PREVIEW_ITEM_MODEL, codes[ctx.selected_index], KF_SHOP_NONE, KF_ITEM_PRICE_BUY)
                    == KF_MENU_CONFIRM_CANCELLED)
                selection = KF_MENU_LIST_PENDING;
            else
                selection = KF_ENUM_ENCODE(u8, codes[ctx.selected_index]);
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (selection != KF_MENU_LIST_PENDING) {
            while (PadRead(1) != 0)
                ;
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = KF_MENU_LIST_NO_SELECTION;
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
            if (menu_load_item_model(codes[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return;
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
            if (menu_load_item_model(codes[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = KF_MENU_LIST_NO_SELECTION;
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_item_model_preview(codes[ctx.selected_index]);
        menu_list_render(&ctx);
        menu_present_frame();
    }

    menu_release_item_model();
    if (selection != KF_MENU_LIST_NO_SELECTION) {
        switch (category) {
        case KF_EQUIP_MENU_WEAPON:
            player_state.equipped_weapon_id = KF_ENUM_DECODE(KfItemId, selection);
            player_equip_weapon(KF_ENUM_DECODE(KfItemId, selection));
            break;
        case KF_EQUIP_MENU_SHIELD:
            player_state.equipped_shield_id = KF_ENUM_DECODE(KfItemId, selection);
            player_set_equipment_slot(KF_ENUM_DECODE(KfItemId, selection), KF_EQUIPMENT_SLOT_SHIELD);
            break;
        case KF_EQUIP_MENU_HEAD:
            player_state.equipped_head_armor_id = KF_ENUM_DECODE(KfItemId, selection);
            player_set_equipment_slot(KF_ENUM_DECODE(KfItemId, selection), KF_EQUIPMENT_SLOT_HEAD);
            break;
        case KF_EQUIP_MENU_BODY:
            player_state.equipped_body_armor_id = KF_ENUM_DECODE(KfItemId, selection);
            player_set_equipment_slot(KF_ENUM_DECODE(KfItemId, selection), KF_EQUIPMENT_SLOT_BODY);
            if (selection == KF_ENUM_ENCODE(s32, KF_ITEM_FULL_PLATE)) {
                player_state.equipped_arm_armor_id = KF_ITEM_NONE;
                player_state.equipped_leg_armor_id = KF_ITEM_NONE;
                player_set_equipment_slot(KF_ITEM_NONE, KF_EQUIPMENT_SLOT_ARM);
                player_set_equipment_slot(KF_ITEM_NONE, KF_EQUIPMENT_SLOT_LEG);
            }
            break;
        case KF_EQUIP_MENU_ARM:
            player_state.equipped_arm_armor_id = KF_ENUM_DECODE(KfItemId, selection);
            player_set_equipment_slot(KF_ENUM_DECODE(KfItemId, selection), KF_EQUIPMENT_SLOT_ARM);
            break;
        case KF_EQUIP_MENU_LEG:
            player_state.equipped_leg_armor_id = KF_ENUM_DECODE(KfItemId, selection);
            player_set_equipment_slot(KF_ENUM_DECODE(KfItemId, selection), KF_EQUIPMENT_SLOT_LEG);
            break;
        case KF_EQUIP_MENU_ACCESSORY:
            player_state.equipped_accessory_id = KF_ENUM_DECODE(KfItemId, selection);
            player_set_equipment_slot(KF_ENUM_DECODE(KfItemId, selection), KF_EQUIPMENT_SLOT_ACCESSORY);
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
    s16 labels[20][MENU_GLYPHS_PER_ROW];
    KfMagicId codes[20];
    s32 code;
    s32 j;
    s32 k;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = KF_MENU_LIST_PENDING;

    while (PadRead(1) != 0)
        ;

    k = 0;
    for (code = KF_ENUM_ENCODE(s32, KF_MAGIC_LIGHTNING_BOLT); code < KF_MAGIC_PLAYER_COUNT; code++) {
        if (magic_records[code].learned == KF_MAGIC_LEARNED) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                labels[k][j] = magic_name_rows[code].codes[j];
            codes[k] = KF_ENUM_DECODE(KfMagicId, code);
            k++;
        }
    }
    labels[k][0] = 0x59;
    labels[k][1] = MENU_TEXT_DAKUTEN | 0x4c;
    labels[k][2] = 0x4c;
    labels[k][3] = MENU_TEXT_END;
    codes[k] = KF_MAGIC_NONE;
    k++;

    menu_list_init(&ctx, KF_MENU_WINDOW_EQUIPMENT, KF_ENUM_ENCODE(s32, KF_EQUIP_MENU_MAGIC));
    ctx.entry_count = k;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = 0;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_texture(menu_texture_from_magic(codes[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED)
            return;
        if (codes[ctx.selected_index] != KF_MAGIC_NONE)
            menu_add_marker_quad();
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            selection = KF_ENUM_ENCODE(s32, menu_list_interact(&ctx, KF_MENU_CONFIRM_EQUIP,
                    KF_MENU_PREVIEW_MAGIC_ICON, codes[ctx.selected_index], KF_SHOP_NONE, KF_ITEM_PRICE_BUY));
            if (selection == KF_ENUM_ENCODE(s32, KF_MENU_CONFIRM_CANCELLED))
                selection = KF_MENU_LIST_PENDING;
            else
                selection = ctx.selected_index;
        }
        if (selection != KF_MENU_LIST_PENDING) {
            while (PadRead(1) != 0)
                ;
            break;
        }

        menu_frame_begin();
        confirm = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = KF_MENU_LIST_NO_SELECTION;
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
            if (menu_load_item_texture(menu_texture_from_magic(codes[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED)
                return;
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
            if (menu_load_item_texture(menu_texture_from_magic(codes[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED)
                return;
        } else if ((input & PADRright) != 0 && (prev & PADRright) == 0) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if ((input & PADRdown) != 0 && (prev & PADRdown) == 0) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = KF_MENU_LIST_NO_SELECTION;
        }

        if (ctx.entry_count != 0) {
            if (codes[ctx.selected_index] != KF_MAGIC_NONE)
                menu_add_marker_quad();
        }
        menu_list_render(&ctx);
    }

    if (selection != KF_MENU_LIST_NO_SELECTION) {
        player_state.selected_magic_id = codes[selection];
        player_select_magic(codes[selection]);
    }
}
