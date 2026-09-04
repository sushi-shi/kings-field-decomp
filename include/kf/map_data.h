#ifndef KF_MAP_DATA_H
#define KF_MAP_DATA_H

/* Shared map-cell resource grids used by the GAME and OPEN overlays. */

#include <kf/semantic_types.h>

extern u8 map_cell_attribute_grid[100][100];
extern u8 map_cell_orientation_grid[100][100];
extern u8 map_collision_flag_grid[100][100];
extern u8 map_collision_grid[100][100];
extern u8 map_floor_height_grid[100][100];

#endif
