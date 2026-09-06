#ifndef KF_ITEM_H
#define KF_ITEM_H

/* Floor-item resource loading shared by GAME and OPEN. */

#include <kf/game_types.h>

enum {
    KF_FLOOR_ITEM_CAPACITY = 64,
    KF_FLOOR_ITEM_END = 0xffff,
    KF_ITEM_STACK_CAPACITY = 99,
    KF_ITEM_VERDITE = 42,
    KF_ITEM_MEDICINAL_HERB = 43,
    KF_ITEM_ANTIDOTE_HERB = 44,
    KF_ITEM_RECOVERY_MEDICINE = 45,
    KF_ITEM_DRAGON_KING_GRASS_LEAF = 46,
    KF_ITEM_DRAGON_KING_GRASS_FRUIT = 47,
    KF_ITEM_LIGHT_RING = 48,
    KF_ITEM_MOON_AMULET = 49,
    KF_ITEM_WIND_BLADE_BRACELET = 50,
    KF_ITEM_TWO_HEADED_DRAGON_RING = 51,
    KF_ITEM_GOLD_CROSS = 52,
    KF_ITEM_KEY_OF_THE_DEAD = 53,
    KF_ITEM_NONE = 0xff
};

/*
 * Serialized floor-item placement record (12 bytes) from the map resource
 * stream, and the runtime floor-item entry (24 bytes) the loader expands it
 * into. The tile bytes index map_floor_height_grid[tile_z][tile_x]; the world
 * position is tile*2000 plus the signed local offset, and the height byte times
 * -100 sinks the item onto the floor.
 */
typedef struct KfFloorItemPlacement {
    u16 item_id;
    u8 facing_and_frame_count;
    u8 unknown_03;
    u8 tile_z;
    u8 tile_x;
    s16 local_z;
    s16 local_x;
    s16 local_y;
} KfFloorItemPlacement;

typedef struct KfFloorItem {
    u16 item_id;
    u8 facing_and_frame_count;
    u8 unknown_03;
    s32 position_x;
    s32 position_y;
    s32 position_z;
    u8 unknown_10[4];
    u8 animation_frame;
    u8 unknown_15[3];
} KfFloorItem;

/* GAME: player quantities, followed by two shop-stock/availability banks. */
extern u8 item_stock[3][80];

/* GAME globals; OPEN places these fields in KfFloorItemStateOpen. */
extern u16 floor_item_count;
extern KfFloorItem floor_items[KF_FLOOR_ITEM_CAPACITY];

extern void item_load_floor_placements(KfFloorItemPlacement *placements);

#endif
