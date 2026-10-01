#include "../src/lib/tmd.cpp"
#include "../src/game/animation_cache.cpp"
#include "../src/game/asset_registry.cpp"

#include <cassert>
#include <string_view>

KfGraphicsRuntimeGame game_graphics_runtime{};

namespace kf {
[[noreturn]] void host_fail(const char *message)
{
    std::fprintf(stderr, "%s\n", message);
    std::exit(77);
}
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    const std::string_view scenario = argv[1];
    alignas(u32) std::array<u8, 68> archive{};
    archive[0] = 2;
    for (std::size_t at : {4, 36}) {
        archive[at] = 32;
        archive[at + 8] = 20;
    }
    if (scenario == "overflow") archive[0] = 11;
    asset_registry_load_tmd_archive(KF_ASSET_ACTOR_FIRST, archive.data(),
        archive.size() - (scenario == "truncated"));
    assert(game_graphics_runtime.asset_registry_tmds[1].data);
    asset_registry_select(1);
    KfAnimationCacheRecord *owner = animation_cache_allocate();
    owner->owner_slot = &owner;
    owner->asset_index = 1;
    owner->state = KF_ANIMATION_CACHE_LIVE;
    owner->cached_vertices.resize(2);
    game_graphics_runtime.current_tmd_vertices = owner->cached_vertices;
    archive[0] = 1;
    asset_registry_load_tmd_archive(KF_ASSET_ACTOR_FIRST, archive.data(), archive.size());
    assert(!owner);
    assert(!game_graphics_runtime.asset_registry_tmds[1].data);
    assert(game_graphics_runtime.current_tmd_vertices.empty());
    asset_registry_set(KF_ASSET_WEAPON, archive.data() + 4, 32);
    asset_registry_set(KF_ASSET_HUD_MODELS, archive.data() + 4, 32);
    tmd_register(tmd_context(), KF_TMD_SLOT_MAP, archive.data() + 24, 12);
    asset_registry_clear_floor();
    assert(!game_graphics_runtime.tmd_state.current_tmd.data);
    assert(!game_graphics_runtime.tmd_state.slots[0].data);
    for (u16 id = 0; id < KF_ASSET_REGISTRY_KNOWN_ENTRIES; ++id)
        assert(bool(game_graphics_runtime.asset_registry_tmds[id].data) ==
            (id == KF_ASSET_WEAPON || id == KF_ASSET_HUD_MODELS));
}
