#ifndef KF_GAME_EQUIPMENT_H
#define KF_GAME_EQUIPMENT_H

/* Runtime-loaded weapon, armor, and accessory definitions. */

#include <kf/game_types.h>

/* Armor and accessory record (item ids 13..54 index armor_records[id - 13]). */
typedef struct KfArmorRecord {
    u16 unknown_00;
    u16 defense_component0;
    u16 defense_component1;
    u16 defense_component2;
    u16 status_effect2_resistance;
    u16 defense_component3;
    u16 defense_component4;
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

extern KfWeaponRecord weapon_records[16];
extern KfArmorRecord armor_records[42];

extern void weapon_records_load_and_mirror_angles(
    const KfWeaponRecord *source);
extern void armor_records_load(const KfArmorRecord *source);

#endif
