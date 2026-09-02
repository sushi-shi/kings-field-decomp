#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfEffectRecord DAT_8009d040[];

ADDRESS(0x80036f00, 0x44)
char *effect_pool_find_free(void)
{
    KfEffectRecord *record = DAT_8009d040;
    u16 i = 48;

    do {
        if (record->type == 0xff) {
            return (char *)record;
        }
        record++;
    } while (--i != 0);
    return 0;
}
