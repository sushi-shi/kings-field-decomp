#include <kf/game_types.h>

extern void func_80020364(const struct KfMatrix *from, const struct KfMatrix *to, s32 blend);
extern void func_8001fde4(s32 first, s32 second);
extern void func_800149f4(void);

void func_80033d80(const struct KfMatrix *from, const struct KfMatrix *to)
{
    s32 blend = 0;

    do {
        func_80020364(from, to, blend);
        func_8001fde4(0, 0);
        func_800149f4();
        blend += 0x400;
    } while (blend <= 0x1000);
}
