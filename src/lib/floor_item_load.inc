#include <kf/lib/random.hpp>

static u16 floor_item_read_u16(const u8 *data)
{
    return static_cast<u16>(data[0] | (static_cast<u16>(data[1]) << 8));
}

void item_load_floor_placements(const u8 *data, std::size_t size)
{
    constexpr std::size_t placement_bytes = 12;
    KF_FLOOR_ITEM_COUNT = 0;
    for (std::size_t offset = 0; offset <= size && size - offset >= 2; offset += placement_bytes) {
        const u8 *placement = data + offset;
        const auto base = floor_item_read_u16(placement);
        if (base == kf_enum_encode<u16>(KF_FLOOR_ITEM_END))
            return;
        if (size - offset < placement_bytes || KF_FLOOR_ITEM_COUNT == KF_FLOOR_ITEM_CAPACITY)
            kf::host_fail("Truncated or oversized floor-item placement list");
        const u8 frame_count = placement[2] & 0x0f;
        const u8 cell_z = placement[4], cell_x = placement[5];
        if (base >= KF_FLOOR_ITEM_SPRITE_COUNT || frame_count > KF_FLOOR_ITEM_SPRITE_COUNT - base
                || cell_z >= KF_MAP_ROWS || cell_x >= KF_MAP_COLUMNS)
            kf::host_fail("Invalid floor-item sprite range or position");

        auto &item = KF_FLOOR_ITEMS[KF_FLOOR_ITEM_COUNT++];
        item.base_sprite_index = static_cast<KfFloorItemSpriteId>(base);
        item.facing = static_cast<KfFloorItemFacing>(placement[2] & 0xf0);
        item.frame_count = frame_count;
        const auto local_z = static_cast<s16>(floor_item_read_u16(placement + 6));
        const auto local_x = static_cast<s16>(floor_item_read_u16(placement + 8));
        const auto local_y = static_cast<s16>(floor_item_read_u16(placement + 10));
        item.position_x = map_placement_axis_position(cell_x, local_x);
        item.position_z = map_placement_axis_position(cell_z, local_z);
        const s32 height = map_floor_height_grid.cells[cell_z][cell_x] * KF_MAP_HEIGHT_STEP;
        item.position_y = local_y - height;
        item.animation_frame = (kf::random_next() * item.frame_count) >> KF_FLOOR_ITEM_INITIAL_FRAME_RANDOM_BITS;
    }
    kf::host_fail("Floor-item placement list has no terminator");
}
