#ifndef KF_GAME_SNAPSHOT_H
#define KF_GAME_SNAPSHOT_H

#include <kf/game/world.h>
#include <memory>
#include <span>
#include <vector>

struct WorldSnapshotInfo {
    u32 epoch {};
    u32 tick {};
    bool full {};
    KfFloorId floor = KF_FLOOR_1;
    KfMapVariant variant = KF_MAP_VARIANT_DEFAULT;
};

inline constexpr std::size_t world_snapshot_limit = 128 * 1024;
void world_clone_simulation(const WorldState &source, WorldState &destination);
void world_apply_snapshot(const WorldState &source, WorldState &destination);
bool world_snapshot_info(std::span<const u8> bytes, WorldSnapshotInfo &info);
bool world_snapshot_encode(WorldState &world, WorldSnapshotInfo info, std::vector<u8> &bytes);
std::unique_ptr<WorldState> world_snapshot_decode(std::span<const u8> bytes,
    const WorldState &base, WorldSnapshotInfo &info);
void player_rebind_records(WorldState &world, PlayerContext &player);

#endif
