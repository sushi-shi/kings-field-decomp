#ifndef KF_GAME_RESOURCES_H
#define KF_GAME_RESOURCES_H

struct WorldState;

struct PlayerContext;

#include <kf/lib/memory.h>
#include <kf/platform/language.h>

extern KfMemoryArena memory_arena;

extern void common_resources_load(WorldState &world, PlayerContext &player);
extern bool game_apply_language(void);
kf::Language game_text_language();
void game_update_language_comparison();

#endif // KF_GAME_RESOURCES_H
