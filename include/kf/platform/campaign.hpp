#pragma once
#include <kf/platform/saves.hpp>
#include <kf/net/world.h>
#include <span>
#include <string_view>
#include <vector>

namespace kf {
std::vector<u8> campaign_file_pack(std::span<const u8> snapshot, std::string_view resources, std::string_view recipe);
bool campaign_file_unpack(std::span<const u8> file, std::string_view resources, std::string_view recipe,
    std::vector<u8> &snapshot);
SaveFileResult campaign_file_summary(SaveSlot slot, std::string_view resources, std::string_view recipe,
    KfNetWorldSummary &summary);
}
