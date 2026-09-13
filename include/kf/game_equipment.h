#ifndef KF_GAME_EQUIPMENT_H
#define KF_GAME_EQUIPMENT_H

#include <kf/game_types.h>
#include <kf/combat.h>
#include <kf/enum.h>
#include <kf/item.h>
#include <kf/game_math.h>

enum class KfEquipmentSlot : u8 {
    KF_EQUIPMENT_SLOT_HEAD = 0,
    KF_EQUIPMENT_SLOT_BODY = 1,
    KF_EQUIPMENT_SLOT_ARM = 2,
    KF_EQUIPMENT_SLOT_LEG = 3,
    KF_EQUIPMENT_SLOT_SHIELD = 4,
    KF_EQUIPMENT_SLOT_ACCESSORY = 5,
    KF_EQUIPMENT_SLOT_REFRESH_ONLY = 0xff
}; using enum KfEquipmentSlot;

enum {
    KF_WEAPON_RECORD_COUNT = 16,
    KF_ARMOR_RECORD_COUNT = 42,
    KF_WEAPON_TABLE_WORD_COUNT = 176,
    KF_ARMOR_TABLE_WORD_COUNT = 294,
};

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

typedef struct KfWeaponRecord {
    u8 unknown_00;
    u8 charge_rate;
    u16 attack_components[KF_COMBAT_COMPONENT_COUNT];
    u16 hp_regen_interval;
    u16 mp_regen_interval;
    u16 projection_distance;
    u16 attack_z_offset;
    u8 unknown_14[0x08];
    struct KfVec3s render_translation;
    u16 unknown_22;
    SVECTOR render_rotation;
} KfWeaponRecord;

typedef struct KfWeaponTable {
    KfWeaponRecord entries[KF_WEAPON_RECORD_COUNT];
} KfWeaponTable;

typedef struct KfArmorTable {
    KfArmorRecord entries[KF_ARMOR_RECORD_COUNT];
} KfArmorTable;

extern KfWeaponTable weapon_records;
extern KfArmorTable armor_records;

extern void weapon_records_load_and_mirror_angles(
    const KfWeaponTable *source);
extern void armor_records_load(const KfArmorTable *source);

#endif
