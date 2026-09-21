#ifndef KF_LIB_GRAPHICS_H
#define KF_LIB_GRAPHICS_H

#ifdef KF_OPEN
#include <kf/open/render.h>
inline KfGraphicsRuntimeOpen &graphics_runtime() { return open_graphics_runtime; }
#else
#include <kf/game/graphics.h>
#include <kf/game/render.h>
inline KfGraphicsRuntimeGame &graphics_runtime() { return game_graphics_runtime; }
#endif

struct KfFloorItemStorage {
    u16 &count;
    KfFloorItem (&items)[KF_FLOOR_ITEM_CAPACITY];
};

inline KfFloorItemStorage floor_item_storage()
{
#ifdef KF_OPEN
    return {open_graphics_runtime.floor_item_state.count, open_graphics_runtime.floor_item_state.items};
#else
    return {game_graphics_runtime.floor_item_count, game_graphics_runtime.floor_items};
#endif
}

#endif
