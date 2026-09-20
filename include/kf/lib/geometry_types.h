#pragma once
#include <kf/lib/types.h>

// Fixed-point value layouts also used by the original resource formats. These
// types carry values only; portable rendering has no implicit register state.
struct MATRIX { s16 m[3][3]; s32 t[3]; };
struct VECTOR { s32 vx, vy, vz, pad; };
struct SVECTOR { s16 vx, vy, vz, pad; };
struct CVECTOR { u8 r, g, b, cd; };
struct DVECTOR { s16 vx, vy; };
static_assert(sizeof(SVECTOR) == 8 && sizeof(VECTOR) == 16 && sizeof(MATRIX) == 32);

// XYZ operations leave padding untouched. Copy/add retain sequential component
// reads and writes, including when source and destination alias.
constexpr void vector_set_xyz(VECTOR &vector, s32 x, s32 y, s32 z)
{
    vector.vx = x;
    vector.vy = y;
    vector.vz = z;
}

constexpr void vector_set_xyz(SVECTOR &vector, s32 x, s32 y, s32 z)
{
    vector.vx = x;
    vector.vy = y;
    vector.vz = z;
}

constexpr void vector_copy_xyz(VECTOR &destination, const SVECTOR &source)
{
    destination.vx = source.vx;
    destination.vy = source.vy;
    destination.vz = source.vz;
}

constexpr void vector_copy_xyz(SVECTOR &destination, const SVECTOR &source)
{
    destination.vx = source.vx;
    destination.vy = source.vy;
    destination.vz = source.vz;
}

constexpr void vector_copy_xyz(SVECTOR &destination, const VECTOR &source)
{
    destination.vx = source.vx;
    destination.vy = source.vy;
    destination.vz = source.vz;
}

constexpr void vector_add_xyz(VECTOR &destination, const VECTOR &source)
{
    destination.vx += source.vx;
    destination.vy += source.vy;
    destination.vz += source.vz;
}

constexpr void vector_add_xyz(VECTOR &destination, const SVECTOR &source)
{
    destination.vx += source.vx;
    destination.vy += source.vy;
    destination.vz += source.vz;
}

constexpr void vector_add_xyz(SVECTOR &destination, const SVECTOR &source)
{
    destination.vx += source.vx;
    destination.vy += source.vy;
    destination.vz += source.vz;
}
