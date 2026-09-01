#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfWeaponRecord weapon_records[16];

ADDRESS(0x800150a8)
void player_weapon_load_records_and_mirror_angles(const u32 *source)
{
    u32 *destination = (u32 *)weapon_records;
    s32 count = 176;
    KfWeaponRecord *record;

    do {
        *destination++ = *source++;
    } while (--count != 0);

    count = 15;
    record = weapon_records;
    do {
        record->mirrored_angle = -record->mirrored_angle;
        record++;
    } while (count-- != 0);
}
