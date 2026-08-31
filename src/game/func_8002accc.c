#include <kf/game_types.h>

struct PolyFT4_8002accc {
    u32 tag;
    u8 r0;
    u8 g0;
    u8 b0;
    u8 code;
    u8 remainder[32];
};

extern struct PolyFT4_8002accc *DAT_80057e88;
extern void SetPolyFT4(struct PolyFT4_8002accc *primitive);

void func_8002accc(void)
{
    SetPolyFT4(DAT_80057e88);
    DAT_80057e88->r0 = 0x60;
    DAT_80057e88->g0 = 0x60;
    DAT_80057e88->b0 = 0x60;
}
