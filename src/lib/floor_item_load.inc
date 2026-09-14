ADDRESS_AT("GAME", 0x80020b4c, 0x1b0)
ADDRESS_AT("OPEN", 0x800197e4, 0x1b0)
void item_load_floor_placements(KfFloorItemPlacement *placements)
{
    KfFloorItemPlacement *first_placement;
    KfFloorItem *item;

    KF_FLOOR_ITEM_COUNT = 0;
    first_placement = placements;
    while (placements++->base_sprite_index != KF_FLOOR_ITEM_END) {
        KF_FLOOR_ITEM_COUNT++;
    }

    item = KF_FLOOR_ITEMS;
    placements = first_placement;
    if (placements->base_sprite_index != KF_FLOOR_ITEM_END) {
        do {
            s32 height;

            item->base_sprite_index = placements->base_sprite_index;
            item->facing_and_frame_count = placements->facing_and_frame_count;
            item->unknown_03 = placements->unknown_03;
            item->position_x = map_placement_axis_position(placements->tile_x, placements->local_x);
            item->position_z = map_placement_axis_position(placements->tile_z, placements->local_z);
            height = map_floor_height_grid.cells[placements->tile_z][placements->tile_x] * KF_MAP_HEIGHT_STEP;
            item->position_y = placements->local_y - height;
            item->animation_frame =
                (rand() * KF_ENUM_ENCODE(u8, item->facing_and_frame_count)) >> KF_FLOOR_ITEM_INITIAL_FRAME_RANDOM_BITS;
            item++;
            placements++;
        } while (placements->base_sprite_index != KF_FLOOR_ITEM_END);
    }
}
