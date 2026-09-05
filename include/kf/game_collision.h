#ifndef KF_GAME_COLLISION_H
#define KF_GAME_COLLISION_H

/* World/map collision results, shared data, and query operations. */

#include <kf/game_types.h>
#include <kf/psyq.h>

/*
 * Optional output from collision_query_world. The query copies a transform
 * from the selected player, actor, map object, or map event and records the
 * selected object's collision radius. The final six bytes remain opaque.
 */
typedef struct KfCollisionTarget {
    VECTOR position;
    SVECTOR rotation;
    u16 radius;
    u8 unknown_1a[0x06];
} KfCollisionTarget;

/* Attribute-selected rectangle, in oriented cell coordinates and floor offsets. */
typedef struct KfCellHeightRecord {
    s16 x_min;
    s16 y_min;
    s16 x_max;
    s16 y_max;
} KfCellHeightRecord;

extern KfCollisionTarget collision_target;
extern s16 map_cell_attribute_height_table[255];
extern KfCellHeightRecord map_cell_height_records[7];

extern void collision_adjust_cell_occupancy(
    u16 cell_x, u16 cell_z, s32 delta);
extern u32 collision_query_world(
    s32 point_x, s32 point_y, s32 point_z, s32 radius, s32 height,
    u32 flags);
extern s32 map_floor_height_at_position(const VECTOR *position);
extern s32 map_floor_height_for_cell_position(
    u16 cell_index, s32 point_x, s32 point_z);

#endif
