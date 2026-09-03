#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

extern u16 DAT_80095038;

ADDRESS(0x8001bae4, 0xb0)
void func_8001bae4(s32 mode)
{
    if (mode == 5) {
        DAT_80095038 = GetTPage(1, 0, 0x140, 0x100);
        DAT_8009503a = GetTPage(1, 0, 0x180, 0x100);
        DAT_8009503c = GetTPage(1, 0, 0x340, 0x100);
        DAT_80095048 = GetClut(0, 0x1eb);
        DAT_8009504a = GetClut(0, 0x1eb);
        DAT_8009504c = GetClut(0, 0x1eb);
    }
}
