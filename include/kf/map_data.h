#ifndef KF_MAP_DATA_H
#define KF_MAP_DATA_H

/* Shared map-cell resource grids used by the GAME and OPEN overlays. */

#include <kf/game_types.h>

enum {
    KF_MAP_COLUMNS = 100,
    KF_MAP_ROWS = 100,
    KF_MAP_TILE_SIZE = 2000,
    KF_MAP_TILE_CENTER = 1000,
    KF_MAP_HEIGHT_STEP = 100,
    KF_MAP_MESHES_PER_BANK = 100,
    KF_MAP_ATTRIBUTE_COUNT = 255,
    KF_MAP_ATTRIBUTE_NONE = 0xff,
    KF_MAP_ATTRIBUTE_WARP = 0x40,
    KF_MAP_ATTRIBUTE_BOTTOMLESS_PIT = 0x5d,
    KF_MAP_CELL_COORD_INVALID = 0xff
};

enum {
    KF_MAP_CELL_BLOCKED = 0,
    KF_MAP_CELL_FLOOR = 1,
    /* Traversable diagonal half-planes in cell-local X/Z coordinates. */
    KF_MAP_CELL_X_GE_Z = 2,
    KF_MAP_CELL_SUM_LE_SIZE = 3,
    KF_MAP_CELL_Z_GE_X = 4,
    KF_MAP_CELL_SUM_GE_SIZE = 5,
    KF_MAP_CELL_STEP = 6,
    KF_MAP_HALF_CELL_STEP_HEIGHT = 300,
    KF_OCCUPANCY_CELL_RADIUS = 2,
    KF_OCCUPANCY_CELL_SPAN = 5,
    KF_CELL_OCCUPANT_COUNT_MASK = 0x1f,
    KF_CELL_PRESERVED_FLAGS_MASK = 0xe0
};

/* Serialized orientations are one-based; render matrices are zero-based. */
enum {
    KF_MAP_ORIENT_UNROTATED = 1,
    KF_MAP_ORIENT_QUARTER_TURN = 2,
    KF_MAP_ORIENT_HALF_TURN = 3,
    KF_MAP_ORIENT_THREE_QUARTER_TURN = 4
};

extern u8 map_cell_attribute_grid[KF_MAP_ROWS][KF_MAP_COLUMNS];
extern u8 map_cell_orientation_grid[KF_MAP_ROWS][KF_MAP_COLUMNS];
extern u8 map_collision_flag_grid[KF_MAP_ROWS][KF_MAP_COLUMNS];
extern u8 map_collision_grid[KF_MAP_ROWS][KF_MAP_COLUMNS];
extern u8 map_floor_height_grid[KF_MAP_ROWS][KF_MAP_COLUMNS];

#endif
