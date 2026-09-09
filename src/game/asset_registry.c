#include <kf/game_graphics.h>
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
        KfAssetHeader *asset = (KfAssetHeader *)archive;

        ((KfAssetHeader **)game_graphics_runtime.unknown_registry_20134)[first_asset_id] = asset;
        asset_registry_select(first_asset_id);
        tmd_prepare_primitive_indices();
        first_asset_id++;
        archive += asset->byte_size;
    }
}

ADDRESS(0x8002055c, 0x40)
void asset_registry_set(u16 index, KfAssetHeader *asset)
{
    ((KfAssetHeader **)game_graphics_runtime.unknown_registry_20134)[index] = asset;
    asset_registry_select(index);
    tmd_prepare_primitive_indices();
}

ADDRESS(0x8002059c, 0x38)
void asset_registry_select(u16 index)
{
    KfAssetHeader *asset = ((KfAssetHeader **)game_graphics_runtime.unknown_registry_20134)[index];

    game_graphics_runtime.tmd_state.current_asset =
        (KfTmdHeader *)((u8 *)asset + asset->tmd_data_offset);
}
