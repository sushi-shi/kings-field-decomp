#ifndef KF_GAME_COLLISION_H
#define KF_GAME_COLLISION_H

#include <kf/lib/types.h>
#include <kf/lib/map_data.h>
#include <kf/lib/geometry_types.h>

// Original actors and wandering events reject cells carrying this authored bit.
inline constexpr int KF_COLLISION_CELL_BLOCKS_WANDER = 0x80;

enum {
    KF_COLLISION_SKIP_TERRAIN = 0x1,
    KF_COLLISION_SKIP_ACTORS = 0x10,
    KF_COLLISION_SKIP_MAP_OBJECTS = 0x20,
    KF_COLLISION_SKIP_MAP_EVENTS = 0x40,
    KF_COLLISION_SKIP_PLAYER = 0x80,
    KF_COLLISION_CAPTURE_TARGET = 0x800,
    KF_COLLISION_CELL_FLAG_SHIFT = 8,
    KF_COLLISION_CELL_FLAG_MASK = 0xf0,
    KF_COLLISION_IGNORE_HEIGHT = 0xffff,
    KF_COLLISION_PLAYER_RADIUS = 800,
    KF_COLLISION_PLAYER_HEIGHT = 1700
};

// Packed world-query results and signed distance/index probes have distinct domains.
inline constexpr u32 KF_COLLISION_NONE = 0xffffffffu;
inline constexpr s32 KF_PROXIMITY_NONE = -1;

enum {
    KF_COLLISION_TERRAIN = 0x10000,
    KF_COLLISION_BELOW_FLOOR = 0x1fff0,
    KF_COLLISION_CEILING = 0x1fff1,
    KF_COLLISION_MISSING_ATTRIBUTE = 0x1fff2,
    KF_COLLISION_ACTOR = 0x100000,
    KF_COLLISION_MAP_OBJECT = 0x200000,
    KF_COLLISION_MAP_EVENT = 0x400000,
    KF_COLLISION_PLAYER = 0x800000
};

enum {
    KF_COLLISION_KIND_SHIFT = 16,
    KF_COLLISION_DETAIL_MASK = 0xffff,
    KF_COLLISION_DETAIL_BELOW_FLOOR = KF_COLLISION_BELOW_FLOOR & KF_COLLISION_DETAIL_MASK,
    KF_COLLISION_DETAIL_CEILING = KF_COLLISION_CEILING & KF_COLLISION_DETAIL_MASK
};

typedef struct KfCollisionTarget {
    VECTOR position;
    SVECTOR rotation;
    u16 radius;
    u8 unknown_1a[0x06];
} KfCollisionTarget;

enum { KF_MAP_CELL_HEIGHT_RECORD_COUNT = 7 };

typedef struct KfCellHeightRecord {
    s16 x_min;
    s16 y_min;
    s16 x_max;
    s16 y_max;
} KfCellHeightRecord;

extern KfCollisionTarget collision_target;
extern s16 map_cell_attribute_height_table[KF_MAP_ATTRIBUTE_COUNT];
extern KfCellHeightRecord map_cell_height_records[KF_MAP_CELL_HEIGHT_RECORD_COUNT];

extern void collision_adjust_cell_occupancy(
    u16 cell_x, u16 cell_z, s32 delta);
extern u32 collision_query_world(
    s32 point_x, s32 point_y, s32 point_z, s32 radius, s32 height,
    u32 flags);
extern s32 map_floor_height_at_position(const VECTOR *position);
extern s32 map_floor_height_for_cell_position(
    u16 cell_index, s32 point_x, s32 point_z);

#endif
