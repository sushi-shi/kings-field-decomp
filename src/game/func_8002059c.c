#include <kf/game_types.h>

struct AssetHeader8002059c {
    u8 unknown_00[8];
    u32 data_offset;
};

extern struct AssetHeader8002059c *DAT_80090fcc[];
extern void *DAT_80090fc8;

void func_8002059c(u16 index)
{
    struct AssetHeader8002059c *asset = DAT_80090fcc[index];

    DAT_80090fc8 = (u8 *)asset + asset->data_offset;
}
