#include <kf/address.h>
#include <kf/item.h>
#include <kf/map_data.h>
#include <kf/psyq_libc.h>

DATA(0x8006da3a, 0x2)
u16 floor_item_count;

DATA(0x8006da40, 0x600)
KfFloorItem floor_items[64];

ADDRESS(0x800197e4, 0x1b0)
void item_load_floor_placements(KfFloorItemPlacement *placements)
{
    KfFloorItemPlacement *placement;
    KfFloorItem *item;
    u16 *count = &floor_item_count;

    placement = placements;
    *count = 0;
    if (placement->item_id != 0xffff) {
        placement++;
        do {
            (*count)++;
        } while (placement++->item_id != 0xffff);
    }

    item = floor_items;
    placement = placements;
    if (placement->item_id != 0xffff) {
        do {
            s32 height;

            item->item_id = placement->item_id;
            item->facing_and_frame_count = placement->facing_and_frame_count;
            item->unknown_03 = placement->unknown_03;
            item->position_x = placement->tile_x * 2000 + placement->local_x;
            item->position_z = placement->tile_z * 2000 + placement->local_z;
            height =
                map_floor_height_grid[placement->tile_z][placement->tile_x] * 100;
            item->position_y = placement->local_y - height;
            item->animation_frame =
                (rand() * item->facing_and_frame_count) >> 15;
            item++;
            placement++;
        } while (placement->item_id != 0xffff);
    }
}
