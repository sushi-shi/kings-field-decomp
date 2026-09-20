#include <kf/lib/null.h>

#include <kf/game/input.h>
#include <kf/game/menu.h>
#include <kf/game/game.h>
static constexpr unsigned MENU_MAGIC_LABEL_CAPACITY = 10;
static constexpr unsigned MENU_MAGIC_ENTRY_CAPACITY = 16;


enum {
    BLESS_HP_RECOVERY_MAGIC_MULTIPLIER = 3
};

KfMagicPanelResult menu_magic_panel(void)
{
    KfMenuList ctx;
    s16 labels[MENU_MAGIC_LABEL_CAPACITY][MENU_GLYPHS_PER_ROW];
    KfEffectKind codes[MENU_MAGIC_ENTRY_CAPACITY];
    s32 found;
    s32 code;
    s32 j;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    KfMagicPanelResult selection = KF_MENU_RESULT_PENDING;

    kf::host_wait_buttons_released();
    menu_list_init(&ctx, KF_MENU_WINDOW_ROOT, kf_enum_encode<s32>(KF_ROOT_CHOICE_USE_MAGIC));

    found = 0;
    for (code = kf_enum_encode<s32>(KF_MAGIC_HEALING); code < kf_enum_encode<s32>(KF_MAGIC_LIGHTNING_BOLT); code++) {
        if (magic_records[code].learned == KF_MAGIC_LEARNED) {
            for (j = 0; j < MENU_GLYPHS_PER_ROW; j++)
                labels[found][j] = magic_name_rows[code].codes[j];
            codes[found] = kf_enum_decode<KfEffectKind>(code);
            found++;
        }
    }
    ctx.entry_count = found;
    ctx.glyphs_per_entry = MENU_GLYPHS_PER_ROW;
    ctx.glyph_rows = &labels[0][0];
    ctx.quantities = NULL;

    menu_frame_begin();
    if (ctx.entry_count != 0) {
        if (menu_load_item_texture(menu_texture_from_magic(codes[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED)
            return KF_MENU_RESULT_CANCELLED;
        menu_add_marker_quad();
    }
    menu_list_render(&ctx);

    for (;;) {
        menu_present_frame();
        if (confirm == KF_MENU_CONFIRM_REQUESTED) {
            if (menu_list_interact(&ctx, KF_MENU_CONFIRM_USE,
                    KF_MENU_PREVIEW_MAGIC_ICON, codes[ctx.selected_index], KF_ITEM_STOCK_PLAYER, KF_TRADE_BUY)
                    == KF_MENU_RESULT_CANCELLED)
                selection = KF_MENU_RESULT_PENDING;
            else
                selection = codes[ctx.selected_index];
        }
        if (selection != KF_MENU_RESULT_PENDING) {
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
                selection = KF_MENU_RESULT_CANCELLED;
            }
        } else if (BUTTON_PRESSED(input, prev, kf::Button::Up)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_previous(&ctx);
            if (menu_load_item_texture(menu_texture_from_magic(codes[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED)
                return KF_MENU_RESULT_CANCELLED;
        } else if (BUTTON_PRESSED(input, prev, kf::Button::Down)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            menu_list_next(&ctx);
            if (menu_load_item_texture(menu_texture_from_magic(codes[ctx.selected_index])) == KF_RESOURCE_LOAD_FAILED)
                return KF_MENU_RESULT_CANCELLED;
        } else if (BUTTON_PRESSED(input, prev, kf::Button::Confirm)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
        } else if (BUTTON_PRESSED(input, prev, kf::Button::Back)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            selection = KF_MENU_RESULT_CANCELLED;
        }

        if (ctx.entry_count != 0)
            menu_add_marker_quad();
        menu_list_render(&ctx);
    }

    if (selection != KF_MENU_RESULT_CANCELLED) {
        if (player_state.vitals.current_mp < magic_records[kf_enum_encode<s32>(selection)].mp_cost)
            return selection;
        player_state.vitals.current_mp -= magic_records[kf_enum_encode<s32>(selection)].mp_cost;
        if (selection == KF_MAGIC_HEALING) {
            player_state.vitals.current_hp += player_state.magic;
        } else if (selection == KF_MAGIC_DISPOISON) {
            player_state.status_effect_flags &= KF_PLAYER_STATUS_CURSE | KF_PLAYER_STATUS_DARKNESS;
        } else if (selection == KF_MAGIC_RESIST_FIRE) {
            player_state.status_effect_flags |= KF_PLAYER_STATUS_FIRE_DEFENSE_BOOST;
            player_apply_fire_defense_boost();
        } else if (selection == KF_MAGIC_BLESS) {
            player_state.status_effect_flags &= KF_PLAYER_STATUS_POISON | KF_PLAYER_STATUS_SLOWED;
            player_state.vitals.current_hp += player_state.magic * BLESS_HP_RECOVERY_MAGIC_MULTIPLIER;
        }
        if (player_state.vitals.current_hp > player_state.vitals.maximum_hp)
            player_state.vitals.current_hp = player_state.vitals.maximum_hp;
    }
    return selection;
}

void menu_option_root(void)
{
    s32 cursor = 0;
    KfMenuConfirmState confirm = KF_MENU_CONFIRM_IDLE;
    s32 input = 0;
    s32 prev;
    KfMenuResult phase = KF_MENU_RESULT_PENDING;
    KfEquipmentMenuCategory selection = KF_EQUIP_MENU_NONE;

    menu_frame_begin();
    menu_draw_name_list();
    menu_draw_window(KF_MENU_WINDOW_EQUIPMENT, KF_MENU_EQUIPMENT_ROW_COUNT, 0, KF_MENU_CONFIRM_IDLE);

    for (;;) {
        menu_present_frame();
        if (selection != KF_EQUIP_MENU_NONE || kf_enum_encode<s32>(phase) == kf_enum_encode<s32>(selection)) {
            menu_frame_begin();
            menu_draw_name_list();
            menu_draw_window(KF_MENU_WINDOW_EQUIPMENT, KF_MENU_EQUIPMENT_ROW_COUNT, cursor, confirm);
            menu_present_frame();
            kf::host_wait_buttons_released();
        }
        switch (selection) {
        case KF_EQUIP_MENU_ARM:
        case KF_EQUIP_MENU_LEG:
            if (player_state.equipped_body_armor_id == KF_ITEM_FULL_PLATE) {
                menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
                break;
            }

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
        if (phase != KF_MENU_RESULT_PENDING)
            return;
        menu_frame_begin();
        confirm = KF_MENU_CONFIRM_IDLE;
        prev = input;
        input = kf::host_read_buttons();
        if (BUTTON_PRESSED(input, prev, kf::Button::Up)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != 0)
                cursor--;
            else
                cursor = KF_MENU_EQUIPMENT_RETURN_ROW;
        } else if (BUTTON_PRESSED(input, prev, kf::Button::Down)) {
            menu_play_input_sound(MENU_SOUND_CURSOR);
            if (cursor != KF_MENU_EQUIPMENT_RETURN_ROW)
                cursor++;
            else
                cursor = 0;
        } else if (BUTTON_PRESSED(input, prev, kf::Button::Confirm)) {
            menu_play_input_sound(MENU_SOUND_CONFIRM);
            confirm = KF_MENU_CONFIRM_REQUESTED;
            if (cursor < KF_MENU_EQUIPMENT_RETURN_ROW)
                selection = kf_enum_decode<KfEquipmentMenuCategory>(cursor);
            else
                phase = KF_MENU_RESULT_CANCELLED;
        } else if (BUTTON_PRESSED(input, prev, kf::Button::Back)) {
            menu_play_input_sound(MENU_SOUND_CANCEL_OR_ERROR);
            phase = KF_MENU_RESULT_CANCELLED;
        }
        menu_draw_name_list();
        menu_draw_window(KF_MENU_WINDOW_EQUIPMENT, KF_MENU_EQUIPMENT_ROW_COUNT, cursor, confirm);
    }
}
