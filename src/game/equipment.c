#include <kf/lib/address.h>
#include <kf/game/equipment.h>
#include <kf/lib/math.h>

DATA(0x8009ff10, 0x2c0)
KfWeaponTable weapon_records;

DATA(0x800a0248, 0x498)
KfArmorTable armor_records;

ADDRESS(0x800150a8, 0x54)
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

ADDRESS(0x800150fc, 0x2c)
void armor_records_load(const KfArmorTable *source)
{
    u32 *destination_word = (u32 *)&armor_records;
    const u32 *source_word = (const u32 *)source;
    s32 remaining = sizeof armor_records / sizeof *source_word;

    do {
        *destination_word++ = *source_word++;
    } while (--remaining != 0);
}

ADDRESS(0x80015128, 0x3c)
s32 fixed6_ratio_step(s32 value, s32 span)
{
    return (value << KF_FIXED6_BITS) / (span + 1) + 1;
}
