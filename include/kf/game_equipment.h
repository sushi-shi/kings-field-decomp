#ifndef KF_GAME_EQUIPMENT_H
#define KF_GAME_EQUIPMENT_H

/* Runtime-loaded weapon, armor, and accessory definitions. */

#include <kf/game_types.h>
#include <kf/enum.h>

KF_ENUM_BEGIN(KfEquipmentSlot, u8)
    KF_EQUIPMENT_SLOT_SHIELD = 0,
    KF_EQUIPMENT_SLOT_HEAD = 1,
    KF_EQUIPMENT_SLOT_ARM = 2,
    KF_EQUIPMENT_SLOT_LEG = 3,
    KF_EQUIPMENT_SLOT_BODY = 4,
    KF_EQUIPMENT_SLOT_ACCESSORY = 5,
    KF_EQUIPMENT_SLOT_REFRESH_ONLY = 0xff
KF_ENUM_END(KfEquipmentSlot)

enum {
    KF_WEAPON_RECORD_COUNT = 16,
    KF_ARMOR_RECORD_COUNT = 42,
    KF_ARMOR_ITEM_FIRST = 13,
    KF_WEAPON_ITEM_FIRST = 0,
    KF_WEAPON_ITEM_END = 13,
    KF_SHIELD_ITEM_FIRST = 13,
    KF_SHIELD_ITEM_END = 19,
    KF_HEAD_ARMOR_ITEM_FIRST = 19,
    KF_HEAD_ARMOR_ITEM_END = 26,
    KF_BODY_ARMOR_ITEM_FIRST = 26,
    KF_BODY_ARMOR_ITEM_END = 32,
    KF_ARM_ARMOR_ITEM_FIRST = 32,
    KF_ARM_ARMOR_ITEM_END = 35,
    KF_LEG_ARMOR_ITEM_FIRST = 35,
    KF_LEG_ARMOR_ITEM_END = 39,
    KF_ACCESSORY_ITEM_FIRST = 48,
    KF_ACCESSORY_ITEM_END = 52
};

/* Armor and accessory record (item ids 13..54 index armor_records[id - 13]). */
typedef struct KfArmorRecord {
    u16 unknown_00;
    u16 cutting_defense;
    u16 striking_defense;
    u16 piercing_defense;
    u16 poison_resistance;
    u16 magic_defense;
    u16 fire_defense;
    u16 hp_regen_interval;
    u16 hp_drain_interval;
    u8 unknown_12[0x0a];
} KfArmorRecord;

/*
 * Equipped weapons use a 0x2c-byte runtime-loaded record. Reviewed combat and
 * attack code establishes only the fields below; the remaining resource bytes
 * deliberately retain opaque identities.
 */
typedef struct KfWeaponRecord {
    u8 unknown_00;
    u8 charge_rate;
    u16 attack_components[5];
    u16 hp_regen_interval;
    u16 mp_regen_interval;
    u8 unknown_10[0x02];
    u16 attack_z_offset;
    u8 unknown_14[0x12];
    u16 mirrored_angle;
    u8 unknown_28[0x04];
} KfWeaponRecord;

extern KfWeaponRecord weapon_records[KF_WEAPON_RECORD_COUNT];
extern KfArmorRecord armor_records[KF_ARMOR_RECORD_COUNT];

extern void weapon_records_load_and_mirror_angles(
    const KfWeaponRecord *source);
extern void armor_records_load(const KfArmorRecord *source);

#endif
