#include <kf/game_types.h>

struct AssetRecord800150a8 {
    u16 angle;
    u8 unknown_02[42];
};

struct AssetBlock800150a8 {
    u8 unknown_000[38];
    struct AssetRecord800150a8 records[16];
};

typedef char AssetRecord800150a8_size_is_44[
    (sizeof(struct AssetRecord800150a8) == 44) ? 1 : -1];

extern struct AssetBlock800150a8 DAT_8009ff10;

void func_800150a8(const u32 *source)
{
    u32 *destination = (u32 *)&DAT_8009ff10;
    s32 count = 176;
    struct AssetRecord800150a8 *record;

    do {
        *destination++ = *source++;
    } while (--count != 0);

    count = 15;
    record = DAT_8009ff10.records;
    do {
        record->angle = -record->angle;
        record++;
    } while (count-- != 0);
}
