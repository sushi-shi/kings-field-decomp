#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

ADDRESS(0x800150a8, 0x54)
void player_weapon_load_records_and_mirror_angles(const KfWeaponRecord *source)
{
    u32 *destination = (u32 *)weapon_records;
    const u32 *source_word = (const u32 *)source;
    s32 count = 176;
    KfWeaponRecord *record;

    do {
        *destination++ = *source_word++;
    } while (--count != 0);

    count = 15;
    record = weapon_records;
    do {
        record->mirrored_angle = -record->mirrored_angle;
        record++;
    } while (count-- != 0);
}
