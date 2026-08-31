#include <kf/game_types.h>

extern void *DAT_80090fcc[];
extern void func_8002059c(u16 index);
extern void func_8001c2b0(void);

void func_8002055c(u16 index, void *asset)
{
    DAT_80090fcc[index] = asset;
    func_8002059c(index);
    func_8001c2b0();
}
