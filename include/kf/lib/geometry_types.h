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

// Coordinate operations preserve the original fixed-point expressions.
#define setVector(p,x,y,z) ((p)->vx=(x),(p)->vy=(y),(p)->vz=(z))
#define limitRange(x,l,h) ((x)=((x)<(l)?(l):(x)>(h)?(h):(x)))
#define copyVector(p,q) setVector(p,(q)->vx,(q)->vy,(q)->vz)
#define addVector(p,q) ((p)->vx+=(q)->vx,(p)->vy+=(q)->vy,(p)->vz+=(q)->vz)
