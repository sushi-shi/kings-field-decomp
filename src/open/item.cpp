#include <kf/item.h>
#include <kf/map_data.h>
#include <kf/open_render.h>
#include <psyq/libc.h>

void item_load_floor_placements(KfFloorItemPlacement *placements)
{
    KfFloorItemPlacement *first_placement;
    KfFloorItem *item;

    open_graphics_runtime.floor_item_state.count = 0;
    first_placement = placements;
    while (placements++->base_sprite_index != KF_FLOOR_ITEM_END) {
        open_graphics_runtime.floor_item_state.count++;
    }

    item = open_graphics_runtime.floor_item_state.items;
    placements = first_placement;
    if (placements->base_sprite_index != KF_FLOOR_ITEM_END) {
        do {
            s32 height;

            item->base_sprite_index = placements->base_sprite_index;
            item->facing_and_frame_count = placements->facing_and_frame_count;
            item->unknown_03 = placements->unknown_03;
            item->position_x = map_placement_axis_position(placements->tile_x, placements->local_x);
            item->position_z = map_placement_axis_position(placements->tile_z, placements->local_z);
            height =
                map_floor_height_grid.cells[placements->tile_z][placements->tile_x] * KF_MAP_HEIGHT_STEP;
            item->position_y = placements->local_y - height;
            item->animation_frame =
                (rand() * kf_enum_encode<u8>(item->facing_and_frame_count)) >> KF_FLOOR_ITEM_INITIAL_FRAME_RANDOM_BITS;
            item++;
            placements++;
        } while (placements->base_sprite_index != KF_FLOOR_ITEM_END);
    }
}
