#include <kf/lib/fixed_math.hpp>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>

namespace kf {
static constexpr s16 quarter_sine[1024] = {
#include "sine_table.inc"
};

enum class AngleQuadrant : u32 { First, Second, Third, Fourth };

static u32 angle_magnitude(s32 angle)
{
    const u32 magnitude = angle < 0 ? 0u - static_cast<u32>(angle) : static_cast<u32>(angle);
    return magnitude & 4095;
}

s32 angle_sine(s32 angle)
{
    const u32 magnitude = angle_magnitude(angle);
    const u32 offset = magnitude & 1023;
    s32 value;
    // The mirrored endpoints and signed-angle folding are intentional. Simply
    // wrapping a negative angle, or shifting cosine by a quarter turn, differs.
    switch (static_cast<AngleQuadrant>(magnitude >> 10)) {
    case AngleQuadrant::First: value = quarter_sine[offset]; break;
    case AngleQuadrant::Second: value = quarter_sine[1023 - offset]; break;
    case AngleQuadrant::Third: value = -quarter_sine[offset]; break;
    case AngleQuadrant::Fourth: value = -quarter_sine[1023 - offset]; break;
    }
    return angle < 0 ? -value : value;
}

s32 angle_cosine(s32 angle)
{
    const u32 magnitude = angle_magnitude(angle);
    const u32 offset = magnitude & 1023;
    switch (static_cast<AngleQuadrant>(magnitude >> 10)) {
    case AngleQuadrant::First: return quarter_sine[1023 - offset];
    case AngleQuadrant::Second: return -quarter_sine[offset];
    case AngleQuadrant::Third: return -quarter_sine[1023 - offset];
    case AngleQuadrant::Fourth: return quarter_sine[offset];
    }
    return 0;
}

s32 fixed_arctangent(s32 ratio_q12)
{
    static constexpr s32 angles[] = {511, 302, 159, 81, 41, 20, 10, 5, 3, 1, 0, 0};
    s32 x = 4096, y = ratio_q12, angle = 0;
    for (unsigned step = 0; step < 12; ++step) {
        const u32 dx = static_cast<u32>(x >> step), dy = static_cast<u32>(y >> step);
        if (y < 0) {
            x = static_cast<s32>(static_cast<u32>(x) - dy);
            y = static_cast<s32>(static_cast<u32>(y) + dx);
            angle -= angles[step];
        } else {
            x = static_cast<s32>(static_cast<u32>(x) + dy);
            y = static_cast<s32>(static_cast<u32>(y) - dx);
            angle += angles[step];
        }
    }
    return angle;
}

s32 length_square_root(u32 squared_length)
{
    static constexpr u16 roots[192] = {
#include "sqrt_table.inc"
    };
    if (squared_length == 0)
        return 0;
    // Preserve the original coarse mantissa lookup, not host sqrt rounding.
    const unsigned leading = std::countl_zero(squared_length) & ~1u;
    const u32 mantissa = leading >= 24 ? squared_length << (leading - 24)
        : squared_length >> (24 - leading);
    const unsigned exponent = (31 - leading) / 2;
    return static_cast<s32>((u32(roots[mantissa - 64]) << exponent) >> 12);
}

void matrix_multiply_rotation(const MATRIX &left, const MATRIX &right, MATRIX &output)
{
    s16 result[3][3];
    for (unsigned row = 0; row < 3; ++row) {
        for (unsigned column = 0; column < 3; ++column) {
            const std::int64_t dot = std::int64_t(left.m[row][0]) * right.m[0][column]
                + std::int64_t(left.m[row][1]) * right.m[1][column]
                + std::int64_t(left.m[row][2]) * right.m[2][column];
            result[row][column] = static_cast<s16>(
                std::clamp<std::int64_t>(dot >> 12, -32768, 32767));
        }
    }
    std::memcpy(output.m, result, sizeof result);
}

void matrix_scale_axes(MATRIX &matrix, const VECTOR &scale)
{
    const s32 axes[] = {scale.vx, scale.vy, scale.vz};
    for (unsigned row = 0; row < 3; ++row) {
        for (unsigned column = 0; column < 3; ++column) {
            // Original scaling wraps the product to 32 bits before its signed
            // shift, then narrows to 16 bits; composition instead saturates.
            const u32 product = static_cast<u32>(matrix.m[row][column])
                * static_cast<u32>(axes[column]);
            matrix.m[row][column] = static_cast<s16>(static_cast<s32>(product) >> 12);
        }
    }
}

VECTOR matrix_apply_rotation(const MATRIX &matrix, const SVECTOR &vector)
{
    s32 result[3];
    for (unsigned row = 0; row < 3; ++row) {
        const std::int64_t dot = std::int64_t(matrix.m[row][0]) * vector.vx
            + std::int64_t(matrix.m[row][1]) * vector.vy
            + std::int64_t(matrix.m[row][2]) * vector.vz;
        result[row] = static_cast<s32>(dot >> 12);
    }
    return {result[0], result[1], result[2], 0};
}

void matrix_set_rotation_xyz(const SVECTOR &angles, MATRIX &matrix)
{
    const s32 cx = angle_cosine(angles.vx), cy = angle_cosine(angles.vy), cz = angle_cosine(angles.vz);
    const s32 sx = angle_sine(angles.vx), sy = angle_sine(angles.vy), sz = angle_sine(angles.vz);
    // Keep each Q12 rounding point, including negation before shifting.
    // This constructor is distinct from the game's Y-X-Z convention.
    const s32 negative_cz_sy = (-cz * sy) >> 12;
    const s32 negative_sz_sy = (-sz * sy) >> 12;
    matrix.m[0][0] = static_cast<s16>((cz * cy) >> 12);
    matrix.m[0][1] = static_cast<s16>((-sz * cy) >> 12);
    matrix.m[0][2] = static_cast<s16>(sy);
    matrix.m[1][0] = static_cast<s16>(((sz * cx) >> 12) - ((negative_cz_sy * sx) >> 12));
    matrix.m[1][1] = static_cast<s16>(((cz * cx) >> 12) + ((negative_sz_sy * sx) >> 12));
    matrix.m[1][2] = static_cast<s16>((-cy * sx) >> 12);
    matrix.m[2][0] = static_cast<s16>(((sz * sx) >> 12) + ((negative_cz_sy * cx) >> 12));
    matrix.m[2][1] = static_cast<s16>(((cz * sx) >> 12) - ((negative_sz_sy * cx) >> 12));
    matrix.m[2][2] = static_cast<s16>((cy * cx) >> 12);
}
}
