#include <kf/address.h>
#include <kf/game_equipment.h>
#include <kf/game_math.h>

ADDRESS(0x800150a8, 0x54)
void weapon_records_load_and_mirror_angles(const KfWeaponRecord *source)
{
    u32 *destination_word = (u32 *)weapon_records;
    const u32 *source_word = (const u32 *)source;
    s32 remaining = 176;
    KfWeaponRecord *record;

    do {
        *destination_word++ = *source_word++;
    } while (--remaining != 0);

    remaining = 15;
    record = weapon_records;
    do {
        record->mirrored_angle = -record->mirrored_angle;
        record++;
    } while (remaining-- != 0);
}

ADDRESS(0x800150fc, 0x2c)
void armor_records_load(const KfArmorRecord *source)
{
    u32 *destination_word = (u32 *)armor_records;
    const u32 *source_word = (const u32 *)source;
    s32 remaining = 294;

    do {
        *destination_word++ = *source_word++;
    } while (--remaining != 0);
}

ADDRESS(0x80015128, 0x3c)
s32 fixed6_ratio_step(s32 value, s32 span)
{
    return (value << 6) / (span + 1) + 1;
}
