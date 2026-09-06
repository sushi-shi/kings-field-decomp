#include <kf/address.h>
#include <kf/item.h>
#include <kf/map_data.h>
#include <kf/open_render.h>
#include <kf/psyq_libc.h>

ADDRESS(0x800197e4, 0x1b0)
void item_load_floor_placements(KfFloorItemPlacement *placements)
{
    KfFloorItemPlacement *first_placement;
    KfFloorItem *item;

    open_graphics_runtime.floor_item_state.count = 0;
    first_placement = placements;
    while (placements++->item_id != 0xffff) {
        open_graphics_runtime.floor_item_state.count++;
    }

    item = open_graphics_runtime.floor_item_state.items;
    placements = first_placement;
    if (placements->item_id != 0xffff) {
        do {
            s32 height;

            item->item_id = placements->item_id;
            item->facing_and_frame_count = placements->facing_and_frame_count;
            item->unknown_03 = placements->unknown_03;
            item->position_x = placements->tile_x * KF_MAP_TILE_SIZE + placements->local_x;
            item->position_z = placements->tile_z * KF_MAP_TILE_SIZE + placements->local_z;
            height =
                map_floor_height_grid[placements->tile_z][placements->tile_x] * KF_MAP_HEIGHT_STEP;
            item->position_y = placements->local_y - height;
            item->animation_frame =
                (rand() * item->facing_and_frame_count) >> 15;
            item++;
            placements++;
        } while (placements->item_id != 0xffff);
    }
}
