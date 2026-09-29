#ifndef KF_LIB_GRAPHICS_H
#define KF_LIB_GRAPHICS_H

#ifdef KF_OPEN
#include <kf/open/render.h>
typedef KfOpenColorPreset KfActiveColorPreset;
#define KF_GRAPHICS_RUNTIME open_graphics_runtime
#define KF_ACTIVE_ORDERING_TABLE open_graphics_runtime.ordering_table
#define KF_FLOOR_ITEM_COUNT open_graphics_runtime.floor_item_state.count
#define KF_FLOOR_ITEMS open_graphics_runtime.floor_item_state.items
#else
#include <kf/game/graphics.h>
#include <kf/game/render.h>
typedef KfGameColorPreset KfActiveColorPreset;
#define KF_GRAPHICS_RUNTIME game_graphics_runtime
#define KF_ACTIVE_ORDERING_TABLE game_graphics_runtime.display_state.ordering_table
#define KF_FLOOR_ITEM_COUNT game_graphics_runtime.floor_item_count
#define KF_FLOOR_ITEMS game_graphics_runtime.floor_items
#endif

#endif
