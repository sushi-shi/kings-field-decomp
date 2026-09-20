#include <kf/lib/random.hpp>

namespace kf {
// One stream spans opening/game/ending, as the original BIOS-owned state did.
// The portable application starts with a deterministic seed of one.
static u32 random_state = 1;

s32 random_next()
{
    random_state = random_state * 1103515245u + 12345u;
    return static_cast<s32>((random_state >> 16) & random_max);
}
}
