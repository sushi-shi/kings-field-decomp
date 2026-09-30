#ifndef KF_PLATFORM_PRELUDE_H
#define KF_PLATFORM_PRELUDE_H

// Common system and portable interfaces.
#include <cstddef>
#include <cstdint>
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>
#include <kf/lib/types.h>
#include <kf/lib/fixed_math.h>
#include <kf/lib/random.h>
#include <kf/audio/sound.h>
#include <kf/platform/files.h>
#include <kf/platform/saves.h>
#include <kf/platform/host.h>
#include <kf/platform/module_state.h>
#include <kf/platform/input.h>
#include <kf/platform/language_runtime.h>
#include <kf/lib/codec.h>
#include <kf/renderer/renderer.h>
#include <kf/renderer/lighting.h>
#include <kf/renderer/projection.h>
#include <kf/lib/geometry_types.h>

// Shared game-library types and operations have one identity across phases.
#include <kf/lib/math.h>
#include <kf/lib/memory.h>
#include <kf/lib/resources.h>
#include <kf/lib/resource_file.h>
#include <kf/lib/debug.h>
#include <kf/lib/audio.h>
#include <kf/lib/map_types.h>
#include <kf/lib/item_types.h>
#include <kf/lib/camera_path.h>
#include <kf/lib/render_types.h>
#include <kf/lib/tmd.h>
#include <kf/lib/graphics.h>
#include <kf/lib/render_face.h>

#endif // KF_PLATFORM_PRELUDE_H
