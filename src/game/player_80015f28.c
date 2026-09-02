#include <kf/address.h>
#include <kf/semantic_types.h>

RODATA(0x80012000, 0x2c)

extern KfPlayerState player_state;
extern KfWeaponRecord weapon_records[16];
extern KfArmorRecord armor_records[42];
/* Magic records of 20 bytes; the first byte of records 0, 1, 4 and 6 gate milestones. */
extern u8 DAT_8009ce60[];
extern void func_8001fa44(s32 arg0);
extern void player_recalculate_combat_stats(void);

/*
 * Rebuilds physical power, magic, the five attack lanes and the six
 * defense lanes from the base stats, the poison status, the weapon, the
 * five armor pieces and the accessory, then fires the magic milestones
 * and clamps both powers below 1000.
 */
ADDRESS(0x80015714, 0x814)
void player_recalculate_combat_stats(void)
{
    const KfWeaponRecord *weapon;
    const KfArmorRecord *armor;
    s32 power;

    player_state.attack_component0 = 0;
    player_state.attack_component1 = 0;
    player_state.attack_component2 = 0;
    player_state.attack_component3 = 0;
    player_state.attack_component4 = 0;
    player_state.damage_defense_component0 = 0;
    player_state.damage_defense_component1 = 0;
    player_state.damage_defense_component2 = 0;
    player_state.status_effect2_resistance = 0;
    player_state.damage_defense_component3 = 0;
    player_state.damage_defense_component4 = 0;
    player_state.physical_power = player_state.base_physical_power;
    player_state.magic = player_state.base_magic;
    if (player_state.status_effect_flags & 1) {
        power = player_state.physical_power - 20;
        if (power < 0) {
            power = 0;
        }
        player_state.physical_power = power;
    }
    if (player_state.equipped_weapon_id != 0xff) {
        weapon = &weapon_records[player_state.equipped_weapon_id];
        player_state.attack_component0 += weapon->attack_components[0];
        player_state.attack_component1 += weapon->attack_components[1];
        player_state.attack_component2 += weapon->attack_components[2];
        player_state.attack_component3 += weapon->attack_components[3];
        player_state.attack_component4 += weapon->attack_components[4];
    }
    if (player_state.equipped_shield_id != 0xff) {
        armor = &armor_records[player_state.equipped_shield_id - 13];
        player_state.damage_defense_component0 += armor->defense_component0;
        player_state.damage_defense_component0 += armor->defense_component0;
        player_state.damage_defense_component1 += armor->defense_component1;
        player_state.damage_defense_component2 += armor->defense_component2;
        player_state.status_effect2_resistance += armor->status_effect2_resistance;
        player_state.damage_defense_component3 += armor->defense_component3;
        player_state.damage_defense_component4 += armor->defense_component4;
    }
    if (player_state.equipped_head_armor_id != 0xff) {
        armor = &armor_records[player_state.equipped_head_armor_id - 13];
        player_state.damage_defense_component0 += armor->defense_component0;
        player_state.damage_defense_component0 += armor->defense_component0;
        player_state.damage_defense_component1 += armor->defense_component1;
        player_state.damage_defense_component2 += armor->defense_component2;
        player_state.status_effect2_resistance += armor->status_effect2_resistance;
        player_state.damage_defense_component3 += armor->defense_component3;
        player_state.damage_defense_component4 += armor->defense_component4;
    }
    if (player_state.equipped_arm_armor_id != 0xff) {
        armor = &armor_records[player_state.equipped_arm_armor_id - 13];
        player_state.damage_defense_component0 += armor->defense_component0;
        player_state.damage_defense_component0 += armor->defense_component0;
        player_state.damage_defense_component1 += armor->defense_component1;
        player_state.damage_defense_component2 += armor->defense_component2;
        player_state.status_effect2_resistance += armor->status_effect2_resistance;
        player_state.damage_defense_component3 += armor->defense_component3;
        player_state.damage_defense_component4 += armor->defense_component4;
    }
    if (player_state.equipped_leg_armor_id != 0xff) {
        armor = &armor_records[player_state.equipped_leg_armor_id - 13];
        player_state.damage_defense_component0 += armor->defense_component0;
        player_state.damage_defense_component0 += armor->defense_component0;
        player_state.damage_defense_component1 += armor->defense_component1;
        player_state.damage_defense_component2 += armor->defense_component2;
        player_state.status_effect2_resistance += armor->status_effect2_resistance;
        player_state.damage_defense_component3 += armor->defense_component3;
        player_state.damage_defense_component4 += armor->defense_component4;
    }
    if (player_state.equipped_body_armor_id != 0xff) {
        armor = &armor_records[player_state.equipped_body_armor_id - 13];
        player_state.damage_defense_component0 += armor->defense_component0;
        player_state.damage_defense_component0 += armor->defense_component0;
        player_state.damage_defense_component1 += armor->defense_component1;
        player_state.damage_defense_component2 += armor->defense_component2;
        player_state.status_effect2_resistance += armor->status_effect2_resistance;
        player_state.damage_defense_component3 += armor->defense_component3;
        player_state.damage_defense_component4 += armor->defense_component4;
    }
    switch (player_state.equipped_accessory_id) {
    case 48:
        player_state.attack_component3 += 5;
        break;
    case 49:
        player_state.damage_defense_component3 += 7;
        break;
    case 50:
        player_state.damage_defense_component4 += 7;
        break;
    case 51:
        player_state.magic += 8;
        break;
    case 42:
        player_state.magic += 1;
        break;
    case 52:
        player_state.attack_component3 += 3;
        break;
    }
    if (player_state.equipped_shield_id == 16) {
        player_state.physical_power -= 8;
    }
    if (player_state.status_effect_flags & 0x10) {
        player_state.damage_defense_component4 += 10;
    }
    if (player_state.base_magic >= 37 && DAT_8009ce60[0] != 0 && DAT_8009ce60[20] == 0) {
        DAT_8009ce60[20] = 1;
        func_8001fa44(1);
    }
    if (player_state.base_magic >= 70 && DAT_8009ce60[120] == 0) {
        DAT_8009ce60[120] = 1;
        func_8001fa44(1);
    }
    if (player_state.base_magic >= 75 && DAT_8009ce60[80] == 0) {
        DAT_8009ce60[80] = 1;
        func_8001fa44(1);
    }
    if (player_state.physical_power >= 1000) {
        player_state.physical_power = 999;
    }
    if (player_state.magic >= 1000) {
        player_state.magic = 999;
    }
}

ADDRESS(0x80015f28, 0x98)
void player_increment_physical_power_training(void)
{
    player_state.physical_power_training++;
    if (player_state.physical_power_training >= 100) {
        player_state.base_physical_power++;
        player_state.physical_power_training = 0;
        if (player_state.base_physical_power >= 1000) {
            player_state.base_physical_power = 999;
        } else {
            func_8001fa44(0x1e);
        }
        player_recalculate_combat_stats();
    }
}

ADDRESS(0x80015fc0, 0x98)
void player_increment_magic_training(void)
{
    player_state.magic_training++;
    if (player_state.magic_training >= 100) {
        player_state.base_magic++;
        player_state.magic_training = 0;
        if (player_state.base_magic >= 1000) {
            player_state.base_magic = 999;
        } else {
            func_8001fa44(0x1f);
        }
        player_recalculate_combat_stats();
    }
}
