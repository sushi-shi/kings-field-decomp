#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfEffectRecord DAT_8009d040[];
extern KfMagicRecord magic_records[24];

ADDRESS(0x8003a244, 0x30)
void effect_pool_reset(void)
{
    KfEffectRecord *record = DAT_8009d040;
    u16 i;

    for (i = 0; i < 48; i++) {
        record->type = 0xff;
        record++;
    }
}

ADDRESS(0x8003a274, 0x2c)
void magic_load_records(const u32 *source)
{
    u32 *destination = (u32 *)magic_records;
    s32 count;

    for (count = 120; count != 0; count--) {
        *destination++ = *source++;
    }
}
