#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfEffectRecord DAT_8009d040[];
extern void func_8003781c(u8 *object);
extern void func_80038a38(void);

ADDRESS(0x8003a760, 0x7c)
void func_8003a760(void)
{
    KfEffectRecord *record = DAT_8009d040;
    u16 i = 47;

    do {
        if (record->unknown_00[0] != 0xff) {
            func_8003781c((u8 *)record);
            func_80038a38();
        }
        record++;
    } while (i-- != 0);
}
