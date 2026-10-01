#include <kf/platform/prelude.h>
#include <kf/platform/frame_task.hpp>
#include <kf/game/game.h>
#include <kf/game/campaign.h>
#include <kf/game/snapshot.h>
#include <kf/game/party_runtime.h>

bool campaign_capture(WorldState &world, u32 tick, std::vector<u8> &bytes, const PlayerContext *revive_at)
{
    auto checkpoint = std::make_unique<WorldState>();
    world_clone_simulation(world, *checkpoint);
    if (revive_at) party_revive_spectators(*checkpoint, *revive_at);
    return world_snapshot_encode(*checkpoint, {world.epoch, tick, true}, bytes);
}

bool campaign_read(kf::SaveSlot slot, std::vector<u8> &snapshot)
{
    std::vector<u8> bytes(kf::campaign_file_capacity);
    std::size_t size = 0;
    if (kf::campaign_file_read(slot, bytes.data(), bytes.size(), &size) != kf::SaveFileResult::Ok)
        return false;
    const auto &config = kf::net::application_config;
    return kf::campaign_file_unpack(std::span<const u8>(bytes).first(size), config.resources, config.avatar_recipe, snapshot);
}

void campaign_poll(WorldState &world)
{
    auto *campaign = world.campaign;
    if (!campaign || !campaign->writing) return;
    const auto result = kf::campaign_file_write_poll();
    if (result == kf::SaveFileResult::Pending) return;
    campaign->writing = false;
    campaign->result = result;
    if (result == kf::SaveFileResult::Ok) {
        campaign->checkpoint = std::move(campaign->pending_checkpoint);
        party_revive_spectators(world, campaign->save_point);
    }
    campaign->pending_checkpoint.clear();
}

kf::FrameTask<KfSaveResult> campaign_save(WorldState &world, PlayerContext &player, kf::SaveSlot slot)
{
    auto *campaign = world.campaign;
    if (!campaign || campaign->writing || world.prediction || world.story.kind || player.party_slot != 0)
        co_return KF_SAVE_RESULT_FAILED;
    if (!campaign_capture(world, campaign->tick, campaign->pending_checkpoint, &player))
        co_return KF_SAVE_RESULT_FAILED;
    const auto &config = kf::net::application_config;
    const auto bytes = kf::campaign_file_pack(campaign->pending_checkpoint, config.resources, config.avatar_recipe);
    if (!kf::campaign_file_write_begin(static_cast<kf::SaveSlot>(slot), bytes.data(), bytes.size()))
        co_return KF_SAVE_RESULT_FAILED;
    campaign->save_point = player;
    campaign->writing = true;
    while (campaign->writing) co_await kf::FrameDelay {1};
    co_return campaign->result == kf::SaveFileResult::Ok ? KF_SAVE_RESULT_OK : KF_SAVE_RESULT_FAILED;
}

KfSaveResult campaign_read_catalog(WorldState &, KfSaveSlotSummary *summaries)
{
    const auto &config = kf::net::application_config;
    for (unsigned slot = 0; slot < KF_SAVE_SLOT_COUNT; ++slot) {
        summaries[slot] = {};
        KfNetWorldSummary saved {};
        const auto result = kf::campaign_file_summary(static_cast<kf::SaveSlot>(slot + 1), config.resources, config.avatar_recipe, saved);
        if (result == kf::SaveFileResult::Missing) continue;
        if (result != kf::SaveFileResult::Ok) {
            summaries[slot].state = result == kf::SaveFileResult::Invalid ? KfSaveSlotState::Damaged : KfSaveSlotState::Unavailable;
            continue;
        }
        summaries[slot] = {saved.experience, kf_enum_decode<KfFloorId>(saved.floor),
            saved.hp, saved.maximum_hp, saved.mp, saved.maximum_mp, KfSaveSlotState::Ready};
    }
    return KF_SAVE_RESULT_OK;
}
