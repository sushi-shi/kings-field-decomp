#include <kf/renderer/projection.hpp>
#include <kf/lib/fixed_math.hpp>
#include <algorithm>
#include <array>
#include <bit>

namespace kf {
static u32 projection_scale_q16(u16 distance, u16 depth)
{
    constexpr u32 maximum_scale = 0x1ffff;
    if (u32(distance) >= u32(depth) * 2)
        return maximum_scale;

    // The original reciprocal uses an 8-bit seed and two integer refinement
    // steps. Host division, including an exact rational quotient, differs.
    static constexpr auto reciprocal_seeds = [] {
        std::array<u8, 257> seeds{};
        for (u32 index = 0; index < seeds.size(); ++index) {
            const s32 seed = static_cast<s32>((0x40000u / (index + 0x100u) + 1) / 2)
                - 0x101;
            seeds[index] = static_cast<u8>(std::max(seed, 0));
        }
        return seeds;
    }();

    const unsigned shift = std::countl_zero(depth);
    const u32 normalized_depth = u32(depth) << shift;
    const u32 seed = reciprocal_seeds[(normalized_depth - 0x7fc0) >> 7] + 0x101;
    const u32 correction = (0x2000080 - normalized_depth * seed) >> 8;
    const u32 reciprocal = (0x80 + correction * seed) >> 8;
    const std::uint64_t scaled = std::uint64_t(u32(distance) << shift) * reciprocal;
    return static_cast<u32>(std::min<std::uint64_t>((scaled + 0x8000) >> 16, maximum_scale));
}

VECTOR render_transform_point(const MATRIX &model, const SVECTOR &point)
{
    VECTOR result = matrix_apply_rotation(model, point);
    result.vx = static_cast<s32>(static_cast<u32>(result.vx) + static_cast<u32>(model.t[0]));
    result.vy = static_cast<s32>(static_cast<u32>(result.vy) + static_cast<u32>(model.t[1]));
    result.vz = static_cast<s32>(static_cast<u32>(result.vz) + static_cast<u32>(model.t[2]));
    return result;
}

void render_place_model(MATRIX &model, const MATRIX &view, const SVECTOR &position)
{
    const VECTOR transformed = render_transform_point(view, position);
    model.t[0] = transformed.vx;
    model.t[1] = transformed.vy;
    model.t[2] = transformed.vz;
}

ProjectedPoint render_project_point(const MATRIX &model, const Projection &projection,
    const SVECTOR &point)
{
    const VECTOR camera = render_transform_point(model, point);
    const u16 depth = static_cast<u16>(std::clamp(camera.vz, 0, 65535));
    const u32 scale = projection_scale_q16(projection.distance, depth);
    const auto screen = [scale](s32 value, s32 center) {
        const s32 offset_q16 = static_cast<s32>(static_cast<u32>(center) << 16);
        const std::int64_t position_q16 =
            std::int64_t(std::clamp(value, -32768, 32767)) * scale + offset_q16;
        return static_cast<s16>(std::clamp<std::int64_t>(position_q16 >> 16, -1024, 1023));
    };
    // Preserve the original wrapped numerator, signed division and 16-bit
    // coefficient before applying the same Q16 scale used for screen X/Y.
    const s32 fog_numerator = static_cast<s32>(static_cast<u32>(projection.fog_near) * (0u - 320u));
    const s16 fog_slope = static_cast<s16>(fog_numerator / Projection::fog_reference_distance);
    const std::int64_t fog_q12 = (std::int64_t(scale) * fog_slope + 0x1400000) >> 12;
    return {screen(camera.vx, projection.center_x), screen(camera.vy, projection.center_y),
        depth, static_cast<s32>(std::clamp<std::int64_t>(fog_q12, 0, 4096))};
}
}
