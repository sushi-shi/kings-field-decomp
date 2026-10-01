#pragma once
#include <kf/net/world.h>
struct WorldState;
struct PlayerContext;
inline constexpr u8 party_story_none = 0, party_story_transfer = 1, party_story_weapon = 2, party_story_boss = 3, party_story_ending = 4;
bool party_story_begin(WorldState &world, PlayerContext &initiator, u16 event);
bool party_story_ready(WorldState &world, u8 slot, u16 page);
bool party_ending_begin(WorldState &world);
bool party_ending_waiting(const WorldState &world);
void party_story_tick(WorldState &world);
void party_story_render(WorldState &world, PlayerContext &viewer);
