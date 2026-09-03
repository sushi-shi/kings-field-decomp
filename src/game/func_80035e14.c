#include <kf/address.h>
#include <kf/semantic_types.h>

extern void pool_release_all(void);
extern void audio_close_vab(void);
extern void func_80035b5c(void);

ADDRESS(0x80035e14, 0x30)
void func_80035e14(void)
{
    pool_release_all();
    audio_close_vab();
    func_80035b5c();
}
