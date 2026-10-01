#pragma once
#include <kf/lib/types.h>
#include <array>

namespace kf::net {
// SHA-256 of a private local credential; safe to include in campaign snapshots.
using Identity = std::array<u8, 32>;
}
