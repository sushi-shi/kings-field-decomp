#include <kf/address.h>
#include <kf/game_asset.h>
#include <kf/game_render.h>

/*
 * Registers every asset of a TMD archive: a u16 count, then chunks that each
 * start with their own byte length, assigned consecutive ids from
 * FIRST_ASSET_ID.
 */
ADDRESS(0x800204c0, 0x9c)
void asset_registry_load_tmd_archive(u16 first_asset_id, u8 *archive)
{
    u16 count = *(u16 *)archive;

    archive += KF_ASSET_ARCHIVE_HEADER_BYTES;
    while (count-- != 0) {
        asset_registry_entries[first_asset_id] = (KfAssetHeader *)archive;
        asset_registry_select(first_asset_id);
        tmd_prepare_primitive_indices();
        first_asset_id++;
        archive += ((KfAssetHeader *)archive)->byte_size;
    }
}

ADDRESS(0x8002055c, 0x40)
void asset_registry_set(u16 index, KfAssetHeader *asset)
{
    asset_registry_entries[index] = asset;
    asset_registry_select(index);
    tmd_prepare_primitive_indices();
}

ADDRESS(0x8002059c, 0x38)
void asset_registry_select(u16 index)
{
    KfAssetHeader *asset = asset_registry_entries[index];

    tmd_state.current_asset = (u8 *)asset + asset->tmd_data_offset;
}
