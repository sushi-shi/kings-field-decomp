#ifndef KF_GAME_MAP_DATA_H
#define KF_GAME_MAP_DATA_H

struct WorldState;

#include <kf/lib/map_types.h>


s32 map_base_floor_height(WorldState &world, s32 x, s32 z);

KfMapAttribute map_attribute_at_cell(WorldState &world, s32 x, s32 z);

#endif
