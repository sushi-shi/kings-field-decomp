#include <kf/lib/null.h>

#include <kf/game/input.h>
#include <kf/game/menu.h>
#include <kf/game/player.h>
#include <kf/game/effect.h>
#include <psyq/pad.h>

void menu_equip_select(KfEquipmentMenuCategory equipment_category)
{
    KfMenuList ctx;
    s16 labels[20][MENU_GLYPHS_PER_ROW];
    KfObjectId item_ids[20];
    s16 *name;
    u8 *player_stock;
    s32 item_id;
    s32 j;
    s32 found;
    s32 start;
    s32 end;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = ((s32)(KF_MENU_RESULT_PENDING));

    while (PadRead(1) != 0) {
    }

    player_stock = item_stock[((u8)(KF_ITEM_STOCK_PLAYER))];
    switch (equipment_category) {
    case KF_EQUIP_MENU_WEAPON:
        start = ((u8)(KF_ITEM_SHORT_SWORD));
        end = ((u8)(KF_ITEM_IRON_MASK));
        break;
    case KF_EQUIP_MENU_SHIELD:
        start = ((u8)(KF_ITEM_SMALL_SHIELD));
        end = ((u8)(KF_ITEM_GAUNTLET));
        break;
    case KF_EQUIP_MENU_HEAD:
        start = ((u8)(KF_ITEM_IRON_MASK));
        end = ((u8)(KF_ITEM_BREASTPLATE));
        break;
    case KF_EQUIP_MENU_BODY:
        start = ((u8)(KF_ITEM_BREASTPLATE));
        end = ((u8)(KF_ITEM_SMALL_SHIELD));
        break;
    case KF_EQUIP_MENU_ARM:
        start = ((u8)(KF_ITEM_GAUNTLET));
        end = ((u8)(KF_ITEM_IRON_BOOTS));
        break;
    case KF_EQUIP_MENU_LEG:
        start = ((u8)(KF_ITEM_IRON_BOOTS));
        end = ((u8)(KF_ITEM_GOLD_COIN));
        break;
    case KF_EQUIP_MENU_ACCESSORY:
        start = ((u8)(KF_ITEM_LIGHT_RING));
        end = ((u8)(KF_ITEM_GOLD_CROSS));
        break;
    }

    found = 0;
    for (item_id = start; item_id < end; item_id++) {
        if (player_stock[item_id] != 0) {
            name = item_name_rows[item_id].codes;
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++) {
                labels[found][j] = name[j];
            }
            item_ids[found] = ((KfObjectId)(item_id));
            found++;
        }
    }
    labels[found][0] = 0x59;
    labels[found][1] = MENU_TEXT_DAKUTEN | 0x4c;
    labels[found][2] = 0x4c;
    labels[found][3] = MENU_TEXT_END;
    item_ids[found] = KF_OBJECT_NONE;
    found++;

    menu_list_init(&ctx, KF_MENU_WINDOW_EQUIPMENT, ((s32)(equipment_category)));
    ctx.entry_count = found;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = NULL;

    if (ctx.entry_count != 0) {
        if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED) {
            return;
        }
    }

    for (;;) {
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            if (menu_list_confirm(&ctx, KF_MENU_CONFIRM_EQUIP,
                    KF_MENU_PREVIEW_ITEM_MODEL, item_ids[ctx.selected_index], KF_ITEM_STOCK_PLAYER, KF_TRADE_BUY)
                    == KF_MENU_RESULT_CANCELLED) {
                selection = ((s32)(KF_MENU_RESULT_PENDING));
            } else {
                selection = ((u8)(item_ids[ctx.selected_index]));
            }
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (selection != ((s32)(KF_MENU_RESULT_PENDING))) {
            while (PadRead(1) != 0) {
            }
            break;
        }

        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = ((s32)(KF_MENU_RESULT_CANCELLED));
            }
        } else if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_previous(&ctx);
            if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED) {
                return;
            }
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_next(&ctx);
            if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED) {
                return;
            }
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = ((s32)(KF_MENU_RESULT_CANCELLED));
        }

        menu_frame_begin();
        if (ctx.entry_count != 0) {
            menu_item_model_preview(item_ids[ctx.selected_index]);
        }
        menu_list_render(&ctx);
        menu_present_frame();
    }

    menu_release_item_model();
    if (selection != ((s32)(KF_MENU_RESULT_CANCELLED))) {
        switch (equipment_category) {
        case KF_EQUIP_MENU_WEAPON:
            player_state.equipped_weapon_id = ((KfObjectId)(selection));
            player_equip_weapon(((KfObjectId)(selection)));
            break;
        case KF_EQUIP_MENU_SHIELD:
            player_state.equipped_shield_id = ((KfObjectId)(selection));
            player_set_equipment_slot(((KfObjectId)(selection)), KF_EQUIPMENT_SLOT_SHIELD);
            break;
        case KF_EQUIP_MENU_HEAD:
            player_state.equipped_head_armor_id = ((KfObjectId)(selection));
            player_set_equipment_slot(((KfObjectId)(selection)), KF_EQUIPMENT_SLOT_HEAD);
            break;
        case KF_EQUIP_MENU_BODY:
            player_state.equipped_body_armor_id = ((KfObjectId)(selection));
            player_set_equipment_slot(((KfObjectId)(selection)), KF_EQUIPMENT_SLOT_BODY);
            if (selection == ((s32)(KF_ITEM_FULL_PLATE))) {
                player_state.equipped_arm_armor_id = KF_OBJECT_NONE;
                player_state.equipped_leg_armor_id = KF_OBJECT_NONE;
                player_set_equipment_slot(KF_OBJECT_NONE, KF_EQUIPMENT_SLOT_ARM);
                player_set_equipment_slot(KF_OBJECT_NONE, KF_EQUIPMENT_SLOT_LEG);
            }
            break;
        case KF_EQUIP_MENU_ARM:
            player_state.equipped_arm_armor_id = ((KfObjectId)(selection));
            player_set_equipment_slot(((KfObjectId)(selection)), KF_EQUIPMENT_SLOT_ARM);
            break;
        case KF_EQUIP_MENU_LEG:
            player_state.equipped_leg_armor_id = ((KfObjectId)(selection));
            player_set_equipment_slot(((KfObjectId)(selection)), KF_EQUIPMENT_SLOT_LEG);
            break;
        case KF_EQUIP_MENU_ACCESSORY:
            player_state.equipped_accessory_id = ((KfObjectId)(selection));
            player_set_equipment_slot(((KfObjectId)(selection)), KF_EQUIPMENT_SLOT_ACCESSORY);
            break;
        }
    }
}

void menu_spell_select(void)
{
    KfMenuList ctx;
    s16 labels[20][MENU_GLYPHS_PER_ROW];
    KfEffectKind magic_ids[20];
    s32 magic_id;
    s32 j;
    s32 found;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = ((s32)(KF_MENU_RESULT_PENDING));

    while (PadRead(1) != 0) {
    }

    found = 0;
    for (magic_id = ((s32)(KF_MAGIC_LIGHTNING_BOLT)); magic_id < KF_MAGIC_PLAYER_COUNT; magic_id++) {
        if (effect_state.magic.entries[magic_id].learned == KF_MAGIC_LEARNED) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++) {
                labels[found][j] = magic_name_rows[magic_id].codes[j];
            }
            magic_ids[found] = ((KfEffectKind)(magic_id));
            found++;
        }
    }
    labels[found][0] = 0x59;
    labels[found][1] = MENU_TEXT_DAKUTEN | 0x4c;
    labels[found][2] = 0x4c;
    labels[found][3] = MENU_TEXT_END;
    magic_ids[found] = KF_MAGIC_NONE;
    found++;

    menu_list_init(&ctx, KF_MENU_WINDOW_EQUIPMENT, ((s32)(KF_EQUIP_MENU_MAGIC)));
    ctx.entry_count = found;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = NULL;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_texture(menu_texture_from_magic(magic_ids[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED) {
            return;
        }
        if (magic_ids[ctx.selected_index] != KF_MAGIC_NONE) {
            menu_add_magic_artwork_quad();
        }
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            selection = ((s32)(menu_list_confirm(&ctx, KF_MENU_CONFIRM_EQUIP,
                    KF_MENU_PREVIEW_MAGIC_ARTWORK, magic_ids[ctx.selected_index], KF_ITEM_STOCK_PLAYER, KF_TRADE_BUY)));
            if (selection == ((s32)(KF_MENU_RESULT_CANCELLED))) {
                selection = ((s32)(KF_MENU_RESULT_PENDING));
            } else {
                selection = ctx.selected_index;
            }
        }
        if (selection != ((s32)(KF_MENU_RESULT_PENDING))) {
            while (PadRead(1) != 0) {
            }
            break;
        }

        menu_frame_begin();
        confirm = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = PadRead(1);
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = ((s32)(KF_MENU_RESULT_CANCELLED));
            }
        } else if (PAD_PRESSED(input, prev, PADLup)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_previous(&ctx);
            if (menu_load_texture(menu_texture_from_magic(magic_ids[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED) {
                return;
            }
        } else if (PAD_PRESSED(input, prev, PADLdown)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_next(&ctx);
            if (menu_load_texture(menu_texture_from_magic(magic_ids[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED) {
                return;
            }
        } else if (PAD_PRESSED(input, prev, PADRright)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if (PAD_PRESSED(input, prev, PADRdown)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = ((s32)(KF_MENU_RESULT_CANCELLED));
        }

        if (ctx.entry_count != 0) {
            if (magic_ids[ctx.selected_index] != KF_MAGIC_NONE) {
                menu_add_magic_artwork_quad();
            }
        }
        menu_list_render(&ctx);
    }

    if (selection != ((s32)(KF_MENU_RESULT_CANCELLED))) {
        player_state.selected_magic_id = magic_ids[selection];
        player_select_magic(magic_ids[selection]);
    }
}
