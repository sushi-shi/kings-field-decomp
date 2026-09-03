#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfEffectRecord DAT_8009d040[];
extern void effect_pool_set_current(u8 *object);
extern void effect_update_dispatch(void);

ADDRESS(0x8003a760, 0x7c)
void effect_pool_sweep(void)
{
    KfEffectRecord *record = DAT_8009d040;
    u16 i = 47;

    do {
        if (record->type != 0xff) {
            effect_pool_set_current((u8 *)record);
            effect_update_dispatch();
        }
        record++;
    } while (i-- != 0);
}
