#pragma once
#include <kf/lib/types.h>

namespace kf {
inline constexpr s32 random_max = 32767;
struct RandomStream { u32 state = 1; };
s32 random_next(RandomStream &stream);
s32 random_next();
}
