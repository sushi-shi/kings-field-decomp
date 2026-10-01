#ifndef KF_GAME_CAMPAIGN_H
#define KF_GAME_CAMPAIGN_H
#include <kf/platform/frame_task.hpp>
#include <kf/game/world.h>
#include <kf/game/save.h>
#include <vector>

struct CampaignRuntime {
    std::vector<u8> checkpoint;
    std::vector<u8> pending_checkpoint;
    PlayerContext save_point;
    kf::SaveFileResult result = kf::SaveFileResult::Unavailable;
    u32 tick {};
    bool writing {};
};
bool campaign_capture(WorldState &world, u32 tick, std::vector<u8> &bytes, const PlayerContext *revive_at = nullptr);
bool campaign_read(kf::SaveSlot slot, std::vector<u8> &snapshot);
void campaign_poll(WorldState &world);
kf::FrameTask<KfSaveResult> campaign_save(WorldState &world, PlayerContext &player, kf::SaveSlot slot);
KfSaveResult campaign_read_catalog(WorldState &world, KfSaveSlotSummary *summaries);
#endif
