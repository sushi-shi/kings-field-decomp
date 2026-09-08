#ifndef KF_GAME_EQUIPMENT_H
#define KF_GAME_EQUIPMENT_H

/* Runtime-loaded weapon, armor, and accessory definitions. */

#include <kf/game_types.h>
#include <kf/enum.h>
#include <kf/item.h>
#include <kf/game_math.h>

KF_ENUM_BEGIN(KfEquipmentSlot, u8)
    KF_EQUIPMENT_SLOT_HEAD = 0,
    KF_EQUIPMENT_SLOT_BODY = 1,
    KF_EQUIPMENT_SLOT_ARM = 2,
    KF_EQUIPMENT_SLOT_LEG = 3,
    KF_EQUIPMENT_SLOT_SHIELD = 4,
    KF_EQUIPMENT_SLOT_ACCESSORY = 5,
    KF_EQUIPMENT_SLOT_REFRESH_ONLY = 0xff
KF_ENUM_END(KfEquipmentSlot)

/* Halfword positions in the weapon record's attack array. */
enum {
    KF_WEAPON_ATTACK_CUTTING = 0,
    KF_WEAPON_ATTACK_STRIKING = 1,
    KF_WEAPON_ATTACK_PIERCING = 2,
    KF_WEAPON_ATTACK_HOLY = 3,
    KF_WEAPON_ATTACK_FIRE = 4,
    KF_WEAPON_ATTACK_COMPONENT_COUNT = 5
};

enum {
    KF_WEAPON_RECORD_COUNT = 16,
    KF_ARMOR_RECORD_COUNT = 42,
    KF_WEAPON_TABLE_WORD_COUNT = 176,
    KF_ARMOR_TABLE_WORD_COUNT = 294,
    KF_ARMOR_ITEM_FIRST = KF_ENUM_ENCODE(u8, KF_ITEM_IRON_MASK),
    KF_WEAPON_ITEM_FIRST = KF_ENUM_ENCODE(u8, KF_ITEM_SHORT_SWORD),
    KF_WEAPON_ITEM_END = KF_ENUM_ENCODE(u8, KF_ITEM_IRON_MASK),
    KF_HEAD_ARMOR_ITEM_FIRST = KF_ENUM_ENCODE(u8, KF_ITEM_IRON_MASK),
    KF_HEAD_ARMOR_ITEM_END = KF_ENUM_ENCODE(u8, KF_ITEM_BREASTPLATE),
    KF_BODY_ARMOR_ITEM_FIRST = KF_ENUM_ENCODE(u8, KF_ITEM_BREASTPLATE),
    KF_BODY_ARMOR_ITEM_END = KF_ENUM_ENCODE(u8, KF_ITEM_SMALL_SHIELD),
    KF_SHIELD_ITEM_FIRST = KF_ENUM_ENCODE(u8, KF_ITEM_SMALL_SHIELD),
    KF_SHIELD_ITEM_END = KF_ENUM_ENCODE(u8, KF_ITEM_GAUNTLET),
    KF_ARM_ARMOR_ITEM_FIRST = KF_ENUM_ENCODE(u8, KF_ITEM_GAUNTLET),
    KF_ARM_ARMOR_ITEM_END = KF_ENUM_ENCODE(u8, KF_ITEM_IRON_BOOTS),
    KF_LEG_ARMOR_ITEM_FIRST = KF_ENUM_ENCODE(u8, KF_ITEM_IRON_BOOTS),
    KF_LEG_ARMOR_ITEM_END = KF_ENUM_ENCODE(u8, KF_ITEM_GOLD_COIN),
    KF_ACCESSORY_ITEM_FIRST = KF_ENUM_ENCODE(u8, KF_ITEM_LIGHT_RING),
    KF_ACCESSORY_ITEM_END = KF_ENUM_ENCODE(u8, KF_ITEM_GOLD_CROSS)
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
 * Equipped weapons use a 0x2c-byte runtime-loaded combat and render record.
 */
typedef struct KfWeaponRecord {
    u8 unknown_00;
    u8 charge_rate;
    u16 attack_components[KF_WEAPON_ATTACK_COMPONENT_COUNT];
    u16 hp_regen_interval;
    u16 mp_regen_interval;
    u16 projection_distance;
    u16 attack_z_offset;
    u8 unknown_14[0x08];
    struct KfVec3s render_translation;
    u16 unknown_22;
    SVECTOR render_rotation;
} KfWeaponRecord;

typedef char check_weapon_record_size[sizeof(KfWeaponRecord) == 0x2c ? 1 : -1];
#define KF_WEAPON_OFFSET_CHECK(field, offset) \
    typedef char check_weapon_##field[ \
        (unsigned long)&((KfWeaponRecord *)0)->field == (offset) ? 1 : -1]
KF_WEAPON_OFFSET_CHECK(projection_distance, 0x10);
KF_WEAPON_OFFSET_CHECK(attack_z_offset, 0x12);
KF_WEAPON_OFFSET_CHECK(render_translation, 0x1c);
KF_WEAPON_OFFSET_CHECK(unknown_22, 0x22);
KF_WEAPON_OFFSET_CHECK(render_rotation, 0x24);
#undef KF_WEAPON_OFFSET_CHECK

/* The resource loaders copy the complete tables in aligned words. */
typedef union KfWeaponTable {
    KfWeaponRecord entries[KF_WEAPON_RECORD_COUNT];
    u32 words[KF_WEAPON_TABLE_WORD_COUNT];
} KfWeaponTable;

typedef union KfArmorTable {
    KfArmorRecord entries[KF_ARMOR_RECORD_COUNT];
    u32 words[KF_ARMOR_TABLE_WORD_COUNT];
} KfArmorTable;

typedef char check_weapon_table_size[sizeof(KfWeaponTable) == 0x2c0 ? 1 : -1];
typedef char check_armor_table_size[sizeof(KfArmorTable) == 0x498 ? 1 : -1];

extern KfWeaponTable weapon_records;
extern KfArmorTable armor_records;

extern void weapon_records_load_and_mirror_angles(
    const KfWeaponTable *source);
extern void armor_records_load(const KfArmorTable *source);

#endif
