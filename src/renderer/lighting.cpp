#include <kf/renderer/lighting.hpp>
#include <algorithm>

namespace kf {
static s32 environment_color_q4(s32 value) {
    return static_cast<s32>(static_cast<u32>(value) << 4);
}

static u8 fog_channel(std::int64_t color_q16, s32 far_color, s16 depth_cue) {
    // Quantize and saturate the difference before interpolation. Authored fog
    // boosts may exceed 4096; the signed 16-bit input is not a clamped fraction.
    const auto far_q16 = std::int64_t{environment_color_q4(far_color)} * 4096;
    const auto difference_q4 = std::clamp(
        static_cast<s32>((far_q16 - color_q16) >> 12), -32768, 32767);
    const auto result_q4 = (color_q16 + std::int64_t{difference_q4} * depth_cue) >> 12;
    return static_cast<u8>(std::clamp<std::int64_t>(result_q4 >> 4, 0, 255));
}

CVECTOR render_fog_color(const LightingEnvironment &environment, CVECTOR color, s32 depth_cue) {
    const auto factor = static_cast<s16>(depth_cue);
    return {
        fog_channel(std::int64_t{color.r} * 65536, environment.fog.r, factor),
        fog_channel(std::int64_t{color.g} * 65536, environment.fog.g, factor),
        fog_channel(std::int64_t{color.b} * 65536, environment.fog.b, factor), color.cd};
}

CVECTOR render_light_normal(const LightingEnvironment &environment, const MATRIX &lights,
    const SVECTOR &normal, CVECTOR base, s32 depth_cue) {
    const s16 direction[] = {normal.vx, normal.vy, normal.vz};
    const s32 ambient[] = {environment.ambient.r, environment.ambient.g, environment.ambient.b};
    const s32 far_color[] = {environment.fog.r, environment.fog.g, environment.fog.b};
    const u8 tint[] = {base.r, base.g, base.b};
    s32 illumination[3];
    u8 color[3];
    for (unsigned row = 0; row < 3; ++row) {
        std::int64_t dot = 0;
        for (unsigned axis = 0; axis < 3; ++axis)
            dot += std::int64_t{lights.m[row][axis]} * direction[axis];
        illumination[row] = std::clamp(static_cast<s32>(dot >> 12), 0, 32767);
    }
    for (unsigned channel = 0; channel < 3; ++channel) {
        std::int64_t dot = std::int64_t{environment_color_q4(ambient[channel])} * 4096;
        for (unsigned light = 0; light < 3; ++light)
            dot += std::int64_t{environment.color_matrix.m[channel][light]} * illumination[light];
        // Preserve the shifted 32-bit result before positive 16-bit saturation.
        const auto intensity_q4 = std::clamp(static_cast<s32>(dot >> 12), 0, 32767);
        const auto tinted_q16 = std::int64_t{tint[channel]} * intensity_q4 * 16;
        color[channel] = fog_channel(tinted_q16, far_color[channel], static_cast<s16>(depth_cue));
    }
    return {color[0], color[1], color[2], base.cd};
}
}
