#include <kf/game_types.h>

struct PolyFT4_8002ad1c {
    u8 bytes[40];
};

struct PrimitiveBuffer8002ad1c {
    u8 unknown_00[8];
    struct PolyFT4_8002ad1c *next_primitive;
};

extern struct PolyFT4_8002ad1c *DAT_80057e88;
extern u32 *DAT_80090ebc;
extern struct PrimitiveBuffer8002ad1c *DAT_80070eb8;
extern void AddPrim(u32 *ordering_table, void *primitive);

void func_8002ad1c(s32 depth)
{
    depth <<= 2;
    AddPrim((u32 *)((u8 *)DAT_80090ebc + depth), DAT_80057e88);
    DAT_80057e88++;
    DAT_80070eb8->next_primitive = DAT_80057e88;
}
