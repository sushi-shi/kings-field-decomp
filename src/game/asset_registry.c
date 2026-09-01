#include <kf/address.h>
#include <kf/game_types.h>

extern void asset_registry_select(u16 index);
extern void tmd_prepare_primitive_indices(void);

struct AssetHeader8002059c {
    u8 unknown_00[8];
    u32 data_offset;
};

extern struct AssetHeader8002059c *asset_registry_entries[];
extern void *current_asset_data;

ADDRESS(0x8002055c, 0x40)
void asset_registry_set(u16 index, void *asset)
{
    asset_registry_entries[index] = asset;
    asset_registry_select(index);
    tmd_prepare_primitive_indices();
}

ADDRESS(0x8002059c, 0x38)
void asset_registry_select(u16 index)
{
    struct AssetHeader8002059c *asset = asset_registry_entries[index];

    current_asset_data = (u8 *)asset + asset->data_offset;
}
