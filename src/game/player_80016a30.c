#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

RODATA(0x80012030, 0x18)

/* "...\0" image path whose two decimal digits at [9] and [10] select the weapon. */
extern s32 cd_file_load_into(u8 *buffer, const char *path);
extern void exit(s32 status);

/* Stores the item id into one of the six armor slots, re-resolves the five armor record pointers, and recalculates. */
ADDRESS(0x80016848, 0x1e8)
void player_set_equipment_slot(u8 item_id, u8 slot)
{
    switch (slot) {
    case 0:
        player_state.equipped_shield_id = item_id;
        break;
    case 1:
        player_state.equipped_head_armor_id = item_id;
        break;
    case 2:
        player_state.equipped_arm_armor_id = item_id;
        break;
    case 3:
        player_state.equipped_leg_armor_id = item_id;
        break;
    case 4:
        player_state.equipped_body_armor_id = item_id;
        break;
    case 5:
        player_state.equipped_accessory_id = item_id;
        break;
    }
    if (player_state.equipped_shield_id != 0xff) {
        player_state.equipped_shield_record = &armor_records[player_state.equipped_shield_id - 13];
    } else {
        player_state.equipped_shield_record = 0;
    }
    if (player_state.equipped_head_armor_id != 0xff) {
        player_state.equipped_head_armor_record =
            &armor_records[player_state.equipped_head_armor_id - 13];
    } else {
        player_state.equipped_head_armor_record = 0;
    }
    if (player_state.equipped_arm_armor_id != 0xff) {
        player_state.equipped_arm_armor_record =
            &armor_records[player_state.equipped_arm_armor_id - 13];
    } else {
        player_state.equipped_arm_armor_record = 0;
    }
    if (player_state.equipped_leg_armor_id != 0xff) {
        player_state.equipped_leg_armor_record =
            &armor_records[player_state.equipped_leg_armor_id - 13];
    } else {
        player_state.equipped_leg_armor_record = 0;
    }
    if (player_state.equipped_body_armor_id != 0xff) {
        player_state.equipped_body_armor_record =
            &armor_records[player_state.equipped_body_armor_id - 13];
    } else {
        player_state.equipped_body_armor_record = 0;
    }
    player_recalculate_combat_stats();
}

ADDRESS(0x80016a30, 0xf4)
void player_equip_weapon(u8 weapon_id)
{
    player_state.attack_charge_state.current = 0;
    player_state.attack_charge_state.committed = 0;
    player_state.weapon_charge_delay = 10;
    player_state.equipped_weapon_id = weapon_id;
    if (weapon_id != 0xff) {
        player_state.equipped_weapon_record = &weapon_records[weapon_id];
        weapon_image_path_template[9] = '0' + weapon_id / 10;
        weapon_image_path_template[10] = '0' + weapon_id % 10;
        if (cd_file_load_into(player_state.weapon_asset_buffer, weapon_image_path_template) != 0) {
            exit(1);
        }
        asset_registry_set(0x14, player_state.weapon_asset_buffer);
    }
    player_state.weapon_attack_phase = -1;
    player_state.unknown_74 = 0;
    player_recalculate_combat_stats();
}
