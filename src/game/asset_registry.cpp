#include <kf/game/graphics.h>

#include <kf/game/asset.h>
#include <kf/game/render.h>

void asset_registry_load_tmd_archive(u16 first_asset_id, u8 *archive, std::size_t size)
{
    if (!archive || size < KF_ASSET_ARCHIVE_HEADER_BYTES)
        kf::host_fail("Truncated model archive header");
    const u16 count = archive[0] | (static_cast<u16>(archive[1]) << 8);
    if (first_asset_id > KF_ASSET_REGISTRY_KNOWN_ENTRIES ||
        count > KF_ASSET_REGISTRY_KNOWN_ENTRIES - first_asset_id)
        kf::host_fail("Model archive exceeds the asset registry");

    archive += KF_ASSET_ARCHIVE_HEADER_BYTES;
    size -= KF_ASSET_ARCHIVE_HEADER_BYTES;
    for (u16 i = 0; i < count; ++i) {
        asset_registry_set(first_asset_id + i, archive, size);
        const auto asset_size = tmd_read_word(archive);
        archive += asset_size;
        size -= asset_size;
    }
}

void asset_registry_set(u16 index, void *data, std::size_t size)
{
    static_assert(sizeof(KfAssetHeader) == 20);
    if (index >= KF_ASSET_REGISTRY_KNOWN_ENTRIES || !data || size < sizeof(KfAssetHeader))
        kf::host_fail("Invalid model asset header");
    auto *bytes = static_cast<u8 *>(data);
    const std::size_t asset_size = tmd_read_word(bytes);
    const std::size_t tmd_offset = tmd_read_word(bytes + 8);
    if (asset_size < sizeof(KfAssetHeader) || asset_size > size ||
        tmd_offset < sizeof(KfAssetHeader) || tmd_offset > asset_size ||
        reinterpret_cast<std::uintptr_t>(data) % alignof(KfAssetHeader))
        kf::host_fail("Invalid model asset extent or alignment");
    const auto tmd = tmd_resource_view(bytes + tmd_offset, asset_size - tmd_offset);
    game_graphics_runtime.asset_registry_entries[index] = static_cast<KfAssetHeader *>(data);
    game_graphics_runtime.asset_registry_tmds[index] = tmd;
    asset_registry_select(index);
}

void asset_registry_select(u16 index)
{
    if (index >= KF_ASSET_REGISTRY_KNOWN_ENTRIES ||
        !game_graphics_runtime.asset_registry_tmds[index].data)
        kf::host_fail("Unregistered model asset");
    game_graphics_runtime.tmd_state.current_asset = game_graphics_runtime.asset_registry_tmds[index];
}
