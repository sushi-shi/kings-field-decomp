#include <kf/platform/prelude.h>
#include <kf/game/asset.h>
#include <kf/game/graphics.h>
#include <kf/game/render.h>

#include <new>
#include <stdexcept>

static void asset_registry_clear(u16 asset_id)
{
    auto &resource = game_graphics_runtime.asset_registry_tmds[asset_id];
    if (game_graphics_runtime.tmd_state.current_tmd.data == resource.data) {
        game_graphics_runtime.tmd_state.current_tmd = {};
        game_graphics_runtime.current_tmd_vertices = {};
    }
    for (auto &record : game_graphics_runtime.animation_cache_records)
        if (record.state != KF_ANIMATION_CACHE_FREE && record.asset_index == asset_id)
            animation_cache_release(&record);
    resource = {};
    game_graphics_runtime.asset_animations[asset_id] = {};
}

void asset_registry_clear_floor()
{
    animation_cache_release_all();
    game_graphics_runtime.tmd_state = {};
    game_graphics_runtime.current_tmd_vertices = {};
    for (u16 id = 0; id < KF_ASSET_REGISTRY_KNOWN_ENTRIES; ++id)
        if (id != KF_ASSET_WEAPON && id != KF_ASSET_HUD_MODELS)
            asset_registry_clear(id);
}

void asset_registry_load_tmd_archive(u16 first_asset_id, u8 *archive, std::size_t size)
{
    if (!archive || size < KF_ASSET_ARCHIVE_HEADER_BYTES)
        kf::host_fail("Truncated model archive header");
    const u16 count = archive[0] | (static_cast<u16>(archive[1]) << 8);
    u16 end;
    switch (first_asset_id) {
    case KF_ASSET_ACTOR_FIRST: end = KF_ASSET_MAP_EVENT_FIRST; break;
    case KF_ASSET_MAP_EVENT_FIRST: end = KF_ASSET_WEAPON; break;
    case KF_ASSET_EFFECT_FIRST: end = KF_ASSET_REGISTRY_KNOWN_ENTRIES; break;
    default: kf::host_fail("Invalid model archive bank");
    }
    if (count > end - first_asset_id)
        kf::host_fail("Model archive exceeds its asset bank");
    // A shorter replacement must not leave views into the previous archive.
    for (u16 id = first_asset_id; id < end; ++id)
        asset_registry_clear(id);

    archive += KF_ASSET_ARCHIVE_HEADER_BYTES;
    size -= KF_ASSET_ARCHIVE_HEADER_BYTES;
    for (u16 i = 0; i < count; ++i) {
        const auto asset_size = asset_registry_set(first_asset_id + i, archive, size);
        archive += asset_size;
        size -= asset_size;
    }
}

std::size_t asset_registry_set(u16 asset_id, void *data, std::size_t size) try
{
    if (asset_id >= KF_ASSET_REGISTRY_KNOWN_ENTRIES)
        kf::host_fail("Invalid model asset header");
    const KfAsset asset({static_cast<const u8 *>(data), size});
    const auto tmd = tmd_resource_view(static_cast<u8 *>(data) + asset.tmd_offset(), asset.tmd().size());
    game_graphics_runtime.asset_registry_tmds[asset_id] = tmd;
    asset_registry_select(asset_id);
    KfAnimationData animation;
    if (asset.animated()) {
        animation.vertex_count = tmd_read_object(tmd_context(), 0).vertex_count;
        if (animation.vertex_count > KF_PROJECTED_VERTEX_CAPACITY)
            kf::host_fail("Animated model exceeds vertex capacity.");
        tmd_select_object_vertices(tmd_context(), 0);
    }
    animation = asset.animation(animation.vertex_count);
    for (auto &record : game_graphics_runtime.animation_cache_records)
        if (record.state != KF_ANIMATION_CACHE_FREE && record.asset_index == asset_id)
            animation_cache_release(&record);
    game_graphics_runtime.asset_animations[asset_id] = std::move(animation);
    return asset.encoded_size();
} catch (const kf::codec::Error &error) {
    error.report();
    kf::host_fail("Cannot decode model animation resource.");
} catch (const std::bad_alloc &) {
    kf::host_fail("Cannot allocate decoded animation data.");
} catch (const std::length_error &) {
    kf::host_fail("Decoded animation exceeds container capacity.");
}

void asset_registry_select(u16 asset_id)
{
    if (asset_id >= KF_ASSET_REGISTRY_KNOWN_ENTRIES ||
        !game_graphics_runtime.asset_registry_tmds[asset_id].data)
        kf::host_fail("Unregistered model asset");
    game_graphics_runtime.tmd_state.current_tmd = game_graphics_runtime.asset_registry_tmds[asset_id];
    game_graphics_runtime.current_tmd_vertices = {};
}
