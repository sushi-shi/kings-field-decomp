#ifndef KF_GAME_RESOURCES_H
#define KF_GAME_RESOURCES_H

struct WorldState;

struct PlayerContext;

#include <kf/lib/memory.h>

extern KfMemoryArena memory_arena;

extern void common_resources_load(WorldState &world, PlayerContext &player);

#endif
