#include <kf/address.h>
#include <kf/game_player.h>
#include <kf/psyq_libc.h>
#include <kf/game.h>

enum {
    CURSE_PHYSICAL_POWER_PENALTY = 20,
    FIRE_DEFENSE_STATUS_BONUS = 10
};

DATA(0x80055810, 0x9)
SoundRef player_sound_refs[3] = {
    {7, 0, 80},
    {7, 1, 89},
    {13, 0, 67}
};

DATA(0x800652a8, 0xf0)
u8 item_stock[3][80];

/*
 * Player death, vitals, and combat run, one contiguous band
 * 0x80015164..0x80016848 (GAME.EXE): death sequence and restart, HP/MP
 * adjustment, damage application and combat-stat recalculation, experience and
 * training, and the physical damage-component formula. Assembled from seven
 * address-adjacent single-purpose player units; module boundary is WIP.
 */
ADDRESS(0x80015164, 0x68)
void player_death_begin(void)
{
    player_state.update_state = KF_PLAYER_UPDATE_DYING;
    player_state.death_camera_pitch_step = 0;
    player_state.death_visual_blend = 0;
    sound_ref_play(&player_sound_refs[1], 0x7f);
    ReadColorMatrix(&player_death_saved_color_matrix);
    player_death_saved_fog_near = render_state.fog_near_distance;
}

/*
 * The two byte loops clear whole BSS runs that start at the named objects
 * (0x2134 bytes of map-event state, 0xf0 bytes of item stock) before seeding
 * the initial player quantities and shop availability.
 */
ADDRESS(0x800151cc, 0x2e4)
void game_state_initialize(void)
{
    u8 *cursor;
    s32 count;

    player_state.experience = 0;
    player_state.progress_state.level = 1;
    player_state.progress_state.current_floor = 1;
    player_state.progress_state.highest_floor = 1;
    player_state.gold = 0x96;
    player_state.attack_charge_state.current = 0;
    player_state.magic_charge = 0;
    player_state.weapon_charge_delay = 0;
    player_state.cutting_defense = 5;
    player_state.striking_defense = 5;
    player_state.piercing_defense = 5;
    player_state.poison_resistance = 5;
    player_state.magic_defense = 5;
    player_state.fire_defense = 5;
    player_state.view_bob_offset = 0;
    player_state.view_bob_phase = 0;
    player_state.vertical_state = 0;
    player_state.vertical_velocity = 0;
    player_state.vitals.current_hp = player_level_growth_table[0].maximum_hp;
    player_state.vitals.maximum_hp = player_level_growth_table[0].maximum_hp;
    player_state.vitals.current_mp = player_level_growth_table[0].maximum_mp;
    player_state.vitals.maximum_mp = player_level_growth_table[0].maximum_mp;
    player_state.base_physical_power = player_level_growth_table[0].physical_power_step;
    player_state.base_magic = player_level_growth_table[0].magic_step;
    player_state.next_level_experience = player_level_growth_table[0].experience_threshold;
    player_equip_weapon(0);
    player_state.equipped_shield_id = KF_ITEM_NONE;
    player_state.equipped_head_armor_id = KF_ITEM_NONE;
    player_state.equipped_arm_armor_id = KF_ITEM_NONE;
    player_state.equipped_leg_armor_id = KF_ITEM_NONE;
    player_state.equipped_body_armor_id = KF_ITEM_NONE;
    player_state.equipped_accessory_id = KF_ITEM_NONE;
    player_set_equipment_slot(0, KF_EQUIPMENT_SLOT_REFRESH_ONLY);
    player_select_magic(8);
    player_state.fire_defense_timer = KF_PLAYER_STATUS_TIMER_INACTIVE;
    player_state.light_effect_timer = -1;
    player_state.slowed_timer = KF_PLAYER_STATUS_TIMER_INACTIVE;
    player_state.poison_timer = KF_PLAYER_STATUS_TIMER_INACTIVE;
    player_state.darkness_timer = KF_PLAYER_STATUS_TIMER_INACTIVE;
    player_state.curse_timer = KF_PLAYER_STATUS_TIMER_INACTIVE;
    cursor = MAP_WORLD_STATE_BYTES;
    count = 0x2133;
    do {
        *cursor++ = 0;
    } while (--count != -1);
    cursor = (u8 *)&item_stock;
    count = 0xef;
    do {
        *cursor++ = 0;
    } while (--count != -1);
    item_stock[0][0x00] = 1;
    item_stock[0][0x2b] = 1;
    item_stock[1][0x00] = 1;
    item_stock[1][0x01] = 1;
    item_stock[1][0x02] = 1;
    item_stock[1][0x0d] = 1;
    item_stock[1][0x0e] = 1;
    item_stock[1][0x14] = 1;
    item_stock[1][0x1a] = 1;
    item_stock[1][0x1b] = 1;
    item_stock[1][0x20] = 1;
    item_stock[1][0x23] = 1;
    item_stock[1][0x2b] = 1;
    item_stock[1][0x2c] = 1;
    item_stock[1][0x2d] = 1;
    item_stock[1][0x2f] = 1;
    item_stock[2][0x02] = 1;
    item_stock[2][0x03] = 1;
    item_stock[2][0x06] = 1;
    item_stock[2][0x0e] = 1;
    item_stock[2][0x0f] = 1;
    item_stock[2][0x13] = 1;
    item_stock[2][0x15] = 1;
    item_stock[2][0x16] = 1;
    item_stock[2][0x1c] = 1;
    item_stock[2][0x25] = 1;
    item_stock[2][0x2b] = 1;
    item_stock[2][0x2c] = 1;
    item_stock[2][0x2d] = 1;
    item_stock[2][0x2e] = 1;
    item_stock[2][0x2f] = 1;
    item_stock[2][0x30] = 1;
    item_stock[2][KF_ITEM_GOLD_CROSS] = 1;
}

ADDRESS(0x800154b0, 0x19c)
void player_death_restart(void)
{
    s32 floor = player_state.progress_state.current_floor;

    if (map_floor1_script.revival_enabled == KF_MAP_SCRIPT_SET && item_stock[0][0x2f] != 0) {
        item_stock[0][0x2f]--;
        map_world_state_persist();
        player_state.camera_position.vx = 0xfa00;
        player_state.vitals.current_hp = player_state.vitals.maximum_hp;
        player_state.vitals.current_mp = player_state.vitals.maximum_mp;
        player_state.camera_position.vz = 0x4e20;
        player_state.camera_rotation.vy = 0;
    } else {
        player_state.camera_position.vx = 0x7918;
        player_state.camera_position.vz = 0x1388;
        player_state.camera_rotation.vy = 0;
        game_state_initialize();
        floor = 0xff;
    }
    player_state.status_effect_flags = 0;
    player_state.camera_rotation.vz = 0;
    player_state.camera_rotation.vx = 0;
    if (floor != 1) {
        player_state.progress_state.current_floor = 1;
        player_state.map_variant = 0;
        pool_release_all();
        audio_close_vab();
        func_800365f8();
    }
    player_sync_position_to_map();
    player_state.view_bob_offset = 0;
    player_state.update_state = KF_PLAYER_UPDATE_RECOVERY_FADE;
    player_state.death_camera_pitch_step = 0;
    player_state.death_visual_blend = 0;
    DAT_80095064 = 0;
    player_state.view_rotation_offset.vz = 0;
    player_state.view_rotation_offset.vy = 0;
    player_state.view_rotation_offset.vx = 0;
    player_state.previous_map_cell.x = player_state.map_cell.x;
    player_state.previous_map_cell.z = player_state.map_cell.z;
    player_state.camera_position.vy = player_state.floor_height - 1500;
}


ADDRESS(0x8001564c, 0x70)
void player_adjust_hp(s32 delta)
{
    s32 value = player_state.vitals.current_hp;

    value += delta;

    if (value <= 0) {
        player_state.vitals.current_hp = 0;
        player_death_begin();
        return;
    }
    if (player_state.vitals.maximum_hp < value) {
        player_state.vitals.current_hp = player_state.vitals.maximum_hp;
    } else {
        player_state.vitals.current_hp = value;
    }
}

ADDRESS(0x800156bc, 0x58)
void player_adjust_mp(s32 delta)
{
    s32 value = player_state.vitals.current_mp;

    value += delta;

    if (value <= 0) {
        player_state.vitals.current_mp = 0;
        return;
    }
    if (player_state.vitals.maximum_mp < value) {
        player_state.vitals.current_mp = player_state.vitals.maximum_mp;
    } else {
        player_state.vitals.current_mp = value;
    }
}


RODATA(0x80012000, 0x2c)

/* Magic records of 20 bytes; the first byte of records 0, 1, 4 and 6 gate milestones. */

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

    player_state.cutting_attack = 0;
    player_state.striking_attack = 0;
    player_state.piercing_attack = 0;
    player_state.holy_attack = 0;
    player_state.fire_attack = 0;
    player_state.cutting_defense = 0;
    player_state.striking_defense = 0;
    player_state.piercing_defense = 0;
    player_state.poison_resistance = 0;
    player_state.magic_defense = 0;
    player_state.fire_defense = 0;
    player_state.physical_power = player_state.base_physical_power;
    player_state.magic = player_state.base_magic;
    if (player_state.status_effect_flags & KF_PLAYER_STATUS_CURSE) {
        power = player_state.physical_power - CURSE_PHYSICAL_POWER_PENALTY;
        if (power < 0) {
            power = 0;
        }
        player_state.physical_power = power;
    }
    if (player_state.equipped_weapon_id != KF_ITEM_NONE) {
        weapon = &weapon_records[player_state.equipped_weapon_id];
        player_state.cutting_attack += weapon->attack_components[0];
        player_state.striking_attack += weapon->attack_components[1];
        player_state.piercing_attack += weapon->attack_components[2];
        player_state.holy_attack += weapon->attack_components[3];
        player_state.fire_attack += weapon->attack_components[4];
    }
    if (player_state.equipped_shield_id != KF_ITEM_NONE) {
        armor = &armor_records[player_state.equipped_shield_id - KF_ARMOR_ITEM_FIRST];
        player_state.cutting_defense += armor->cutting_defense;
        player_state.cutting_defense += armor->cutting_defense;
        player_state.striking_defense += armor->striking_defense;
        player_state.piercing_defense += armor->piercing_defense;
        player_state.poison_resistance += armor->poison_resistance;
        player_state.magic_defense += armor->magic_defense;
        player_state.fire_defense += armor->fire_defense;
    }
    if (player_state.equipped_head_armor_id != KF_ITEM_NONE) {
        armor = &armor_records[player_state.equipped_head_armor_id - KF_ARMOR_ITEM_FIRST];
        player_state.cutting_defense += armor->cutting_defense;
        player_state.cutting_defense += armor->cutting_defense;
        player_state.striking_defense += armor->striking_defense;
        player_state.piercing_defense += armor->piercing_defense;
        player_state.poison_resistance += armor->poison_resistance;
        player_state.magic_defense += armor->magic_defense;
        player_state.fire_defense += armor->fire_defense;
    }
    if (player_state.equipped_arm_armor_id != KF_ITEM_NONE) {
        armor = &armor_records[player_state.equipped_arm_armor_id - KF_ARMOR_ITEM_FIRST];
        player_state.cutting_defense += armor->cutting_defense;
        player_state.cutting_defense += armor->cutting_defense;
        player_state.striking_defense += armor->striking_defense;
        player_state.piercing_defense += armor->piercing_defense;
        player_state.poison_resistance += armor->poison_resistance;
        player_state.magic_defense += armor->magic_defense;
        player_state.fire_defense += armor->fire_defense;
    }
    if (player_state.equipped_leg_armor_id != KF_ITEM_NONE) {
        armor = &armor_records[player_state.equipped_leg_armor_id - KF_ARMOR_ITEM_FIRST];
        player_state.cutting_defense += armor->cutting_defense;
        player_state.cutting_defense += armor->cutting_defense;
        player_state.striking_defense += armor->striking_defense;
        player_state.piercing_defense += armor->piercing_defense;
        player_state.poison_resistance += armor->poison_resistance;
        player_state.magic_defense += armor->magic_defense;
        player_state.fire_defense += armor->fire_defense;
    }
    if (player_state.equipped_body_armor_id != KF_ITEM_NONE) {
        armor = &armor_records[player_state.equipped_body_armor_id - KF_ARMOR_ITEM_FIRST];
        player_state.cutting_defense += armor->cutting_defense;
        player_state.cutting_defense += armor->cutting_defense;
        player_state.striking_defense += armor->striking_defense;
        player_state.piercing_defense += armor->piercing_defense;
        player_state.poison_resistance += armor->poison_resistance;
        player_state.magic_defense += armor->magic_defense;
        player_state.fire_defense += armor->fire_defense;
    }
    switch (player_state.equipped_accessory_id) {
    case 48:
        player_state.holy_attack += 5;
        break;
    case 49:
        player_state.magic_defense += 7;
        break;
    case 50:
        player_state.fire_defense += 7;
        break;
    case 51:
        player_state.magic += 8;
        break;
    case 42:
        player_state.magic += 1;
        break;
    case KF_ITEM_GOLD_CROSS:
        player_state.holy_attack += 3;
        break;
    }
    if (player_state.equipped_shield_id == 16) {
        player_state.physical_power -= 8;
    }
    if (player_state.status_effect_flags & KF_PLAYER_STATUS_FIRE_DEFENSE_BOOST) {
        player_state.fire_defense += FIRE_DEFENSE_STATUS_BONUS;
    }
    if (player_state.base_magic >= 37 && magic_records[0].learned != 0 && magic_records[1].learned == 0) {
        magic_records[1].learned = 1;
        notify_enqueue(1);
    }
    if (player_state.base_magic >= 70 && magic_records[6].learned == 0) {
        magic_records[6].learned = 1;
        notify_enqueue(1);
    }
    if (player_state.base_magic >= 75 && magic_records[4].learned == 0) {
        magic_records[4].learned = 1;
        notify_enqueue(1);
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
            notify_enqueue(0x1e);
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
            notify_enqueue(0x1f);
        }
        player_recalculate_combat_stats();
    }
}

ADDRESS(0x80016058, 0x224)
void player_add_experience(s16 amount)
{
    const KfPlayerLevelGrowth *growth;
    u8 level;

    player_state.experience += amount;
    if (player_state.experience > 99999) {
        player_state.experience = 99999;
    }
    while (player_state.experience >= player_state.next_level_experience) {
        level = player_state.progress_state.level;
        if (player_state.progress_state.level >= 255) {
            break;
        }
        player_state.progress_state.level = level + 1;
        if (level >= 40) {
            player_state.vitals.maximum_hp +=
                player_level_growth_table[39].maximum_hp
                - player_level_growth_table[38].maximum_hp;
            player_state.vitals.maximum_mp +=
                player_level_growth_table[39].maximum_mp
                - player_level_growth_table[38].maximum_mp;
            player_state.base_physical_power += player_level_growth_table[39].physical_power_step;
            player_state.base_magic += player_level_growth_table[39].magic_step;
            player_state.next_level_experience +=
                player_level_growth_table[39].experience_threshold
                - player_level_growth_table[38].experience_threshold;
        } else {
            growth = &player_level_growth_table[level];
            player_state.vitals.maximum_hp = growth->maximum_hp;
            player_state.vitals.maximum_mp = growth->maximum_mp;
            player_state.base_physical_power += growth->physical_power_step;
            player_state.base_magic += growth->magic_step;
            player_state.next_level_experience = growth->experience_threshold;
        }
        if (player_state.vitals.maximum_hp >= 10000) {
            player_state.vitals.maximum_hp = 9999;
        }
        if (player_state.vitals.maximum_mp >= 10000) {
            player_state.vitals.maximum_mp = 9999;
        }
        if (player_state.base_physical_power >= 1000) {
            player_state.base_physical_power = 999;
        }
        if (player_state.base_magic >= 1000) {
            player_state.base_magic = 999;
        }
        player_recalculate_combat_stats();
        notify_enqueue(0);
        sound_ref_play(&player_sound_refs[2], 0x7f);
    }
}


/*
 * The parameters are reused as the working values: retail keeps the
 * threshold in $a0 and the excess in $a1 for the whole body.
 */
ADDRESS(0x8001627c, 0xa8)
s32 player_calculate_damage_component(s32 base_power, s32 defense, s32 attack)
{
    if (attack == 0) {
        return 0;
    }
    base_power = defense + base_power / 5;
    defense = attack - base_power;
    if (defense < 0) {
        defense = 0;
    }
    if (base_power == 0) {
        base_power = 1;
    }
    return defense + (attack * attack) / (base_power * 2);
}


/*
 * Applies curse, darkness (blocked by accessory 0x31), resisted poison
 * and slowed movement, then combines the five defended damage
 * components in tenths, scales the sum, and subtracts it from the hit
 * points, flagging the update state on any damage.
 */
ADDRESS(0x80016324, 0x390)
void player_apply_damage(
    u16 component0,
    u16 component1,
    u16 component2,
    u16 status_effect_flags,
    u16 component3,
    u16 component4,
    u16 scale_q12,
    u16 multiplier_tenths)
{
    s32 damage;
    s32 loss;
    s32 remaining;

    if (status_effect_flags & KF_PLAYER_STATUS_CURSE) {
        player_state.curse_timer = KF_CURSE_DURATION_UPDATES;
        player_state.status_effect_flags |= KF_PLAYER_STATUS_CURSE;
    }
    if ((status_effect_flags & KF_PLAYER_STATUS_DARKNESS)
        && player_state.equipped_accessory_id != 0x31) {
        if (player_state.darkness_timer != KF_PLAYER_STATUS_TIMER_INACTIVE) {
            if (player_state.darkness_timer < KF_DARKNESS_REAPPLY_TIMER) {
                player_state.darkness_timer = KF_DARKNESS_REAPPLY_TIMER;
            }
        } else {
            player_state.darkness_timer = KF_DARKNESS_DURATION_UPDATES;
        }
        player_state.status_effect_flags |= KF_PLAYER_STATUS_DARKNESS;
    }
    if (status_effect_flags & KF_PLAYER_STATUS_POISON) {
        if (player_state.poison_resistance < (rand() * 100) >> 15) {
            player_state.poison_timer = KF_POISON_DURATION_UPDATES;
            player_state.status_effect_flags |= KF_PLAYER_STATUS_POISON;
        }
    }
    if (status_effect_flags & KF_PLAYER_STATUS_SLOWED) {
        player_state.slowed_timer = KF_SLOWED_DURATION_UPDATES;
        player_state.status_effect_flags |= KF_PLAYER_STATUS_SLOWED;
    }
    damage = player_calculate_damage_component(
        player_state.physical_power * 10, player_state.cutting_defense * 10, component0 * 10);
    damage += player_calculate_damage_component(
        player_state.physical_power * 10, player_state.striking_defense * 10, component1 * 10);
    damage += player_calculate_damage_component(
        player_state.physical_power * 10, player_state.piercing_defense * 10, component2 * 10);
    damage += player_calculate_damage_component(
        player_state.physical_power * 10, player_state.magic_defense * 10, component3 * 10);
    damage += player_calculate_damage_component(
        player_state.physical_power * 10, player_state.fire_defense * 10, component4 * 10);
    damage += 5;
    damage = (scale_q12 * (damage / 10)) >> 12;
    loss = (multiplier_tenths * damage) / 10;
    if (loss != 0) {
        remaining = player_state.vitals.current_hp - loss;
        if (remaining <= 0) {
            remaining = 0;
        }
        player_state.vitals.current_hp = remaining;
        if (player_state.update_state != KF_PLAYER_UPDATE_DYING) {
            player_state.update_state = KF_PLAYER_DAMAGE_FRAME_FIRST;
        }
    }
}

ADDRESS(0x800166b4, 0x130)
void player_apply_radial_damage(
    const struct KfVec3i *origin,
    u32 radius,
    u16 falloff_q12,
    u16 base_power,
    u16 component0,
    u16 component1,
    u16 component2,
    u16 component3,
    u16 component4,
    u16 scale_q12,
    u16 multiplier_tenths)
{
    s32 distance;
    u16 attenuation;
    u32 value;

    distance = player_distance_to_point(origin->x, origin->y, origin->z, radius, radius);
    if (distance == -1) {
        return;
    }
    if (falloff_q12 != 0x1000) {
        attenuation = (distance << 12) / radius;
        value = attenuation * (0x1000 - falloff_q12);
        attenuation = 0x1000 - (value >> 12);
        value = scale_q12 * attenuation;
        attenuation = value >> 12;
    } else {
        attenuation = scale_q12;
    }
    player_apply_damage(
        component0, component1, component2, 0, component3, component4,
        attenuation, multiplier_tenths);
}



ADDRESS(0x800167e4, 0x64)
void player_select_magic(u8 magic_id)
{
    player_state.magic_charge = 0;
    player_state.selected_magic_id = magic_id;
    if (magic_id == KF_MAGIC_NONE) {
        player_state.selected_magic_record = 0;
    } else {
        player_state.selected_magic_record =
            &magic_records[player_state.selected_magic_id];
    }
}
