#ifndef KF_ITEM_H
#define KF_ITEM_H

/* Floor-item resource loading shared by GAME and OPEN. */

#include <kf/game_types.h>
#include <kf/enum.h>

/* Database IDs exclude the no-item sentinel; stock adds the player bank. */
enum {
    KF_ITEM_COUNT = 80,
    KF_ITEM_SHOP_COUNT = 2,
    KF_ITEM_STOCK_BANK_COUNT = 3
};

/* Stock rows distinguish owned quantities from the two shop lists. */
enum {
    KF_ITEM_STOCK_PLAYER = 0,
    KF_ITEM_STOCK_FIRST_SHOP = 1,
    KF_ITEM_STOCK_SECOND_SHOP = 2
};

/* Shop identity shares the stock-row encoding; zero is the player's bank. */
KF_ENUM_BEGIN(KfShopId, u8)
    KF_SHOP_FIRST = KF_ITEM_STOCK_FIRST_SHOP,
    KF_SHOP_SECOND = KF_ITEM_STOCK_SECOND_SHOP
KF_ENUM_END(KfShopId)

enum {
    KF_FLOOR_ITEM_CAPACITY = 64,
    KF_FLOOR_ITEM_END = 0xffff,
    KF_ITEM_STACK_CAPACITY = 99
};

KF_ENUM_BEGIN(KfItemId, u8)
    KF_ITEM_SHORT_SWORD = 0,
    KF_ITEM_BATTLE_AXE = 1,
    KF_ITEM_KNIGHT_SWORD = 2,
    KF_ITEM_COLICHEMARDE = 3,
    KF_ITEM_MORNING_STAR = 4,
    KF_ITEM_BASTARD_SWORD = 5,
    KF_ITEM_CRESCENT_AXE = 6,
    KF_ITEM_TRIPLE_FANG = 7,
    KF_ITEM_FLAME_SWORD = 8,
    KF_ITEM_SHADOW_BLADE = 9,
    KF_ITEM_DRAGON_SWORD = 10,
    KF_ITEM_MOONLIGHT_SWORD = 11,
    KF_ITEM_IRON_MASK = 13,
    KF_ITEM_KNIGHT_HELM = 14,
    KF_ITEM_GREAT_HELM = 15,
    KF_ITEM_BLACK_MASK = 16,
    KF_ITEM_MIST_CROWN = 17,
    KF_ITEM_BREASTPLATE = 19,
    KF_ITEM_KNIGHT_PLATE = 20,
    KF_ITEM_FULL_PLATE = 21,
    KF_ITEM_FIRE_MAIL = 22,
    KF_ITEM_SKULL_ARMOR = 23,
    KF_ITEM_FOREST_ARMOR = 24,
    KF_ITEM_SMALL_SHIELD = 26,
    KF_ITEM_KNIGHT_SHIELD = 27,
    KF_ITEM_TOWER_SHIELD = 28,
    KF_ITEM_BLACK_DRAGON = 29,
    KF_ITEM_WIND_GUARD = 30,
    KF_ITEM_GAUNTLET = 32,
    KF_ITEM_KNIGHT_GLOVE = 33,
    KF_ITEM_STONE_HAND = 34,
    KF_ITEM_IRON_BOOTS = 35,
    KF_ITEM_KNIGHT_BOOTS = 36,
    KF_ITEM_LEG_GUARDS = 37,
    KF_ITEM_FEATHER_BOOTS = 38,
    KF_ITEM_GOLD_COIN = 39,
    KF_ITEM_BLOODSTONE = 40,
    KF_ITEM_MOONSTONE = 41,
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
    KF_ITEM_RAITO_FAMILY_KEY = 54,
    KF_ITEM_WATCHMAN_MAP = 55,
    KF_ITEM_DRAGON_CHALICE = 56,
    KF_ITEM_ILLUSION_STAFF = 57,
    KF_ITEM_GREEN_DRAGON_STAFF = 58,
    KF_ITEM_MIRROR_OF_TRUTH = 59,
    KF_ITEM_DUNGEON_KEY = 60,
    KF_ITEM_DRAGON_CREST = 61,
    KF_ITEM_HARP = 62,
    KF_ITEM_WATER_SEAL_STONE = 63,
    KF_ITEM_EARTH_SEAL_STONE = 64,
    KF_ITEM_SEAL_CANDLESTICK = 67,
    KF_ITEM_FIRE_SEAL_STONE = 68,
    KF_ITEM_WIND_SEAL_STONE = 69,
    KF_ITEM_HOLY_SEAL_STONE = 70,
    KF_ITEM_SORCERER_MAP = 73,
    KF_ITEM_SORCERER_KEY = 74,
    KF_ITEM_DEMON_KING_HAND = 75,
    KF_ITEM_SALAMANDER_STATUE = 76,
    KF_ITEM_NONE = 0xff
KF_ENUM_END(KfItemId)

/* High nibble: zero billboard or biased quarter-turn facing; low: frame count. */
enum {
    KF_FLOOR_ITEM_FACING_MASK = 0xf0,
    KF_FLOOR_ITEM_FACING_BILLBOARD = 0,
    KF_FLOOR_ITEM_FACING_ZERO_YAW = 0x10,
    KF_FLOOR_ITEM_FACING_TO_ANGLE_SHIFT = 6,
    KF_FLOOR_ITEM_FRAME_COUNT_MASK = 0x0f,
    KF_FLOOR_ITEM_INITIAL_FRAME_RANDOM_BITS = 15,
    KF_FLOOR_ITEM_FIXED_FACING_DEPTH_BIAS = 150,
    KF_FLOOR_ITEM_BILLBOARD_DEPTH_BIAS = 200
};

/*
 * Serialized floor-item placement record (12 bytes) from the map resource
 * stream, and the runtime floor-item entry (24 bytes) the loader expands it
 * into. base_sprite_index selects the first sprite descriptor of the animation.
 * The tile bytes index map_floor_height_grid[tile_z][tile_x]; the world
 * position is tile*2000 plus the signed local offset, and the height byte times
 * -100 sinks the item onto the floor.
 */
typedef struct KfFloorItemPlacement {
    u16 base_sprite_index;
    u8 facing_and_frame_count;
    u8 unknown_03;
    u8 tile_z;
    u8 tile_x;
    s16 local_z;
    s16 local_x;
    s16 local_y;
} KfFloorItemPlacement;

typedef struct KfFloorItem {
    u16 base_sprite_index;
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
extern u8 item_stock[KF_ITEM_STOCK_BANK_COUNT][KF_ITEM_COUNT];

/* GAME globals; OPEN places these fields in KfFloorItemStateOpen. */
extern u16 floor_item_count;
extern KfFloorItem floor_items[KF_FLOOR_ITEM_CAPACITY];

extern void item_load_floor_placements(KfFloorItemPlacement *placements);

#endif
