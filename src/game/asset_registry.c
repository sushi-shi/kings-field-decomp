#include <kf/address.h>
#include <kf/semantic_types.h>

extern KfTmdState tmd_state;

extern void asset_registry_select(u16 index);
extern void tmd_prepare_primitive_indices(void);

struct AssetHeader8002059c {
    u8 unknown_00[8];
    u32 data_offset;
};

extern struct AssetHeader8002059c *asset_registry_entries[];

/*
 * Registers every asset of a TMD archive: a u16 count, then chunks that each
 * start with their own byte length, assigned consecutive ids from
 * FIRST_ASSET_ID.
 */
ADDRESS(0x800204c0, 0x9c)
void asset_registry_load_tmd_archive(u16 first_asset_id, u8 *archive)
{
    u16 count = *(u16 *)archive;

    archive += 4;
    while (count-- != 0) {
        asset_registry_entries[first_asset_id] = (struct AssetHeader8002059c *)archive;
        asset_registry_select(first_asset_id);
        tmd_prepare_primitive_indices();
        first_asset_id++;
        archive += *(u32 *)archive;
    }
}

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

    tmd_state.current_asset = (u8 *)asset + asset->data_offset;
}
