#include <kf/game/equipment.h>
#include <kf/lib/math.h>

KfWeaponTable weapon_records;

KfArmorTable armor_records;

void weapon_records_load_and_mirror_angles(const KfWeaponTable *source)
{
    u32 *destination_word = (u32 *)&weapon_records;
    const u32 *source_word = (const u32 *)source;
    s32 remaining = sizeof weapon_records / sizeof *source_word;
    KfWeaponRecord *record;

    do {
        *destination_word++ = *source_word++;
    } while (--remaining != 0);

    remaining = KF_WEAPON_RECORD_COUNT - 1;
    record = weapon_records.entries;
    do {
        record->render_rotation.vy = -record->render_rotation.vy;
        record++;
    } while (remaining-- != 0);
}

void armor_records_load(const KfArmorTable *source)
{
    u32 *destination_word = (u32 *)&armor_records;
    const u32 *source_word = (const u32 *)source;
    s32 remaining = sizeof armor_records / sizeof *source_word;

    do {
        *destination_word++ = *source_word++;
    } while (--remaining != 0);
}

s32 fixed6_ratio_step(s32 value, s32 span)
{
    return (value << KF_FIXED6_BITS) / (span + 1) + 1;
}


void equipment_reset_module_state(void)
{
    kf::restore_initial_value<weapon_records>();
    kf::restore_initial_value<armor_records>();
}
