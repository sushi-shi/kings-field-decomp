#include <kf/platform/prelude.h>
#include <kf/lib/resources.h>
#include <kf/lib/byte_reader.h>
#include <kf/platform/host.h>
#include <kf/renderer/renderer.h>

#include <cstring>

void map_grids_load(KfResourceChunk chunk, KfMapAttributeGrid &attributes,
    KfMapGrid &heights, KfMapOrientationGrid &orientations,
    KfMapGrid &flags, KfMapCollisionGrid &collision)
{
    const auto size = sizeof attributes + sizeof heights + sizeof orientations
        + sizeof flags + sizeof collision;
    if (chunk.size < size)
        kf::host_fail("Truncated map grids");
    const u8 *source = chunk.data;
    auto copy = [&](auto &grid) {
        std::memcpy(&grid, source, sizeof grid);
        source += sizeof grid;
    };
    copy(attributes);
    copy(heights);
    copy(orientations);
    copy(flags);
    copy(collision);
    for (const auto value : orientations.linear) {
        const auto orientation = kf_enum_encode<u8>(value);
        if (orientation < kf_enum_encode<u8>(KF_MAP_ORIENT_UNROTATED) ||
            orientation > kf_enum_encode<u8>(KF_MAP_ORIENT_THREE_QUARTER_TURN))
            kf::host_fail("Invalid map cell orientation");
    }
}

void cell_windows_load(KfResourceChunk chunk,
    std::span<KfCellWindow, KF_CELL_WINDOW_YAW_COUNT> windows)
{
    using namespace kf::codec;
    if (decode([&] {
        Reader input({chunk.data, chunk.size});
        for (auto &window : windows) {
            window.width = input.u16_le();
            window.height = input.u16_le();
            window.origin_x = input.u16_le();
            window.origin_z = input.u16_le();
            require(window.width != 0 && window.height != 0 &&
                std::size_t(window.width) * window.height <= KF_CELL_WINDOW_CELL_CAPACITY,
                "invalid cell-window dimensions");
            require(window.origin_x < window.width && window.origin_z < window.height,
                "invalid cell-window origin");
            for (auto &cell : window.cells) {
                const auto value = input.byte();
                require(value <= kf_enum_encode<u8>(KF_CELL_WINDOW_NEAR),
                    "invalid cell visibility");
                cell = kf_enum_decode<KfCellVisibility>(value);
            }
        }
    }) != KF_CODEC_OK)
        kf::host_fail("Invalid cell-window resource");
}

void tim_upload_images(const u8 *tim_data, std::size_t size)
{
    if (!kf::texture_store_upload_tim(&kf::host_renderer()->textures, tim_data, size))
        kf::host_fail("Invalid TIM resource or texture allocation failure.");
}
