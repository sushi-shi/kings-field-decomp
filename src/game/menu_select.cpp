#include <kf/game/menu_glyphs.h>
#include <kf/lib/null.h>

#include <kf/platform/input.hpp>
#include <kf/game/menu.h>
#include <kf/game/game.h>
static constexpr unsigned MENU_SELECTION_LIST_CAPACITY = 20;


void menu_equip_select(KfEquipmentMenuCategory equipment_category)
{
    KfMenuList ctx;
    s16 labels[MENU_SELECTION_LIST_CAPACITY][MENU_GLYPHS_PER_ROW];
    KfObjectId item_ids[MENU_SELECTION_LIST_CAPACITY];
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
    s32 selection = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);

    kf::host_wait_buttons_released();

    player_stock = item_stock[kf_enum_encode<u8>(KF_ITEM_STOCK_PLAYER)];
    switch (equipment_category) {
    case KF_EQUIP_MENU_WEAPON:
        start = kf_enum_encode<u8>(KF_ITEM_SHORT_SWORD);
        end = kf_enum_encode<u8>(KF_ITEM_IRON_MASK);
        break;
    case KF_EQUIP_MENU_SHIELD:
        start = kf_enum_encode<u8>(KF_ITEM_SMALL_SHIELD);
        end = kf_enum_encode<u8>(KF_ITEM_GAUNTLET);
        break;
    case KF_EQUIP_MENU_HEAD:
        start = kf_enum_encode<u8>(KF_ITEM_IRON_MASK);
        end = kf_enum_encode<u8>(KF_ITEM_BREASTPLATE);
        break;
    case KF_EQUIP_MENU_BODY:
        start = kf_enum_encode<u8>(KF_ITEM_BREASTPLATE);
        end = kf_enum_encode<u8>(KF_ITEM_SMALL_SHIELD);
        break;
    case KF_EQUIP_MENU_ARM:
        start = kf_enum_encode<u8>(KF_ITEM_GAUNTLET);
        end = kf_enum_encode<u8>(KF_ITEM_IRON_BOOTS);
        break;
    case KF_EQUIP_MENU_LEG:
        start = kf_enum_encode<u8>(KF_ITEM_IRON_BOOTS);
        end = kf_enum_encode<u8>(KF_ITEM_GOLD_COIN);
        break;
    case KF_EQUIP_MENU_ACCESSORY:
        start = kf_enum_encode<u8>(KF_ITEM_LIGHT_RING);
        end = kf_enum_encode<u8>(KF_ITEM_GOLD_CROSS);
        break;
    }

    found = 0;
    for (item_id = start; item_id < end; item_id++) {
        if (player_stock[item_id] != 0) {
            name = item_name_rows[item_id].codes;
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                labels[found][j] = name[j];
            item_ids[found] = kf_enum_decode<KfObjectId>(item_id);
            found++;
        }
    }
    labels[found][0] = menu_glyphs::unequip[0];
    labels[found][1] = menu_glyphs::unequip[1];
    labels[found][2] = menu_glyphs::unequip[2];
    labels[found][3] = MENU_TEXT_END;
    item_ids[found] = KF_OBJECT_NONE;
    found++;

    menu_list_init(&ctx, KF_MENU_WINDOW_EQUIPMENT, kf_enum_encode<s32>(equipment_category));
    ctx.entry_count = found;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = NULL;

    if (ctx.entry_count != 0) {
        if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
            return;
    }

    for (;;) {
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            if (menu_list_confirm(&ctx, KF_MENU_CONFIRM_EQUIP,
                    KF_MENU_PREVIEW_ITEM_MODEL, item_ids[ctx.selected_index], KF_ITEM_STOCK_PLAYER, KF_TRADE_BUY)
                    == KF_MENU_RESULT_CANCELLED)
                selection = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);
            else
                selection = kf_enum_encode<u8>(item_ids[ctx.selected_index]);
        }
        confirm = KF_MENU_CONFIRM_IDLE;
        if (selection != kf_enum_encode<s32>(KF_MENU_RESULT_PENDING)) {
            kf::host_wait_buttons_released();
            break;
        }

        prev = input;
        input = kf::host_read_buttons();
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
            }
        } else if (menu_list_handle_navigation(ctx, input, prev)) {
            if (menu_load_item_model(item_ids[ctx.selected_index]) != KF_RESOURCE_LOADED)
                return;
        } else if (kf::button_pressed(input, prev, kf::Button::Confirm)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if (kf::button_pressed(input, prev, kf::Button::Back)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
        }

        menu_frame_begin();
        if (ctx.entry_count != 0)
            menu_item_model_preview(item_ids[ctx.selected_index]);
        menu_list_render(&ctx);
        menu_present_frame();
    }

    menu_release_item_model();
    if (selection != kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED)) {
        switch (equipment_category) {
        case KF_EQUIP_MENU_WEAPON:
            player_state.equipped_weapon_id = kf_enum_decode<KfObjectId>(selection);
            player_equip_weapon(kf_enum_decode<KfObjectId>(selection));
            break;
        case KF_EQUIP_MENU_SHIELD:
            player_state.equipped_shield_id = kf_enum_decode<KfObjectId>(selection);
            player_set_equipment_slot(kf_enum_decode<KfObjectId>(selection), KF_EQUIPMENT_SLOT_SHIELD);
            break;
        case KF_EQUIP_MENU_HEAD:
            player_state.equipped_head_armor_id = kf_enum_decode<KfObjectId>(selection);
            player_set_equipment_slot(kf_enum_decode<KfObjectId>(selection), KF_EQUIPMENT_SLOT_HEAD);
            break;
        case KF_EQUIP_MENU_BODY:
            player_state.equipped_body_armor_id = kf_enum_decode<KfObjectId>(selection);
            player_set_equipment_slot(kf_enum_decode<KfObjectId>(selection), KF_EQUIPMENT_SLOT_BODY);
            if (selection == kf_enum_encode<s32>(KF_ITEM_FULL_PLATE)) {
                player_state.equipped_arm_armor_id = KF_OBJECT_NONE;
                player_state.equipped_leg_armor_id = KF_OBJECT_NONE;
                player_set_equipment_slot(KF_OBJECT_NONE, KF_EQUIPMENT_SLOT_ARM);
                player_set_equipment_slot(KF_OBJECT_NONE, KF_EQUIPMENT_SLOT_LEG);
            }
            break;
        case KF_EQUIP_MENU_ARM:
            player_state.equipped_arm_armor_id = kf_enum_decode<KfObjectId>(selection);
            player_set_equipment_slot(kf_enum_decode<KfObjectId>(selection), KF_EQUIPMENT_SLOT_ARM);
            break;
        case KF_EQUIP_MENU_LEG:
            player_state.equipped_leg_armor_id = kf_enum_decode<KfObjectId>(selection);
            player_set_equipment_slot(kf_enum_decode<KfObjectId>(selection), KF_EQUIPMENT_SLOT_LEG);
            break;
        case KF_EQUIP_MENU_ACCESSORY:
            player_state.equipped_accessory_id = kf_enum_decode<KfObjectId>(selection);
            player_set_equipment_slot(kf_enum_decode<KfObjectId>(selection), KF_EQUIPMENT_SLOT_ACCESSORY);
            break;
        }
    }
}

void menu_spell_select(void)
{
    KfMenuList ctx;
    s16 labels[MENU_SELECTION_LIST_CAPACITY][MENU_GLYPHS_PER_ROW];
    KfEffectKind magic_ids[MENU_SELECTION_LIST_CAPACITY];
    s32 magic_id;
    s32 j;
    s32 found;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    s32 selection = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);

    kf::host_wait_buttons_released();

    found = 0;
    for (magic_id = kf_enum_encode<s32>(KF_MAGIC_LIGHTNING_BOLT); magic_id < KF_MAGIC_PLAYER_COUNT; magic_id++) {
        if (effect_state.magic.entries[magic_id].learned == KF_MAGIC_LEARNED) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                labels[found][j] = magic_name_rows[magic_id].codes[j];
            magic_ids[found] = kf_enum_decode<KfEffectKind>(magic_id);
            found++;
        }
    }
    labels[found][0] = menu_glyphs::unequip[0];
    labels[found][1] = menu_glyphs::unequip[1];
    labels[found][2] = menu_glyphs::unequip[2];
    labels[found][3] = MENU_TEXT_END;
    magic_ids[found] = KF_MAGIC_NONE;
    found++;

    menu_list_init(&ctx, KF_MENU_WINDOW_EQUIPMENT, kf_enum_encode<s32>(KF_EQUIP_MENU_MAGIC));
    ctx.entry_count = found;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = NULL;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_texture(menu_texture_from_magic(magic_ids[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED)
            return;
        if (magic_ids[ctx.selected_index] != KF_MAGIC_NONE)
            menu_add_magic_artwork_quad();
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            selection = kf_enum_encode<s32>(menu_list_confirm(&ctx, KF_MENU_CONFIRM_EQUIP,
                    KF_MENU_PREVIEW_MAGIC_ARTWORK, magic_ids[ctx.selected_index], KF_ITEM_STOCK_PLAYER, KF_TRADE_BUY));
            if (selection == kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED))
                selection = kf_enum_encode<s32>(KF_MENU_RESULT_PENDING);
            else
                selection = ctx.selected_index;
        }
        if (selection != kf_enum_encode<s32>(KF_MENU_RESULT_PENDING)) {
            kf::host_wait_buttons_released();
            break;
        }

        menu_frame_begin();
        confirm = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = kf::host_read_buttons();
        if (ctx.entry_count == 0) {
            if (input != 0) {
                menu_play_input_sound(MENU_SOUND_CURSOR);
                selection = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
            }
        } else if (menu_list_handle_navigation(ctx, input, prev)) {
            if (menu_load_texture(menu_texture_from_magic(magic_ids[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED)
                return;
        } else if (kf::button_pressed(input, prev, kf::Button::Confirm)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if (kf::button_pressed(input, prev, kf::Button::Back)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED);
        }

        if (ctx.entry_count != 0) {
            if (magic_ids[ctx.selected_index] != KF_MAGIC_NONE)
                menu_add_magic_artwork_quad();
        }
        menu_list_render(&ctx);
    }

    if (selection != kf_enum_encode<s32>(KF_MENU_RESULT_CANCELLED)) {
        player_state.selected_magic_id = magic_ids[selection];
        player_select_magic(magic_ids[selection]);
    }
}
