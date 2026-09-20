#pragma once
#include <kf/lib/geometry_types.h>

namespace kf {
struct Projection {
    // Original fog setters always use this reference, even for weapon views.
    static constexpr s32 fog_reference_distance = 200;
    s32 center_x = 160;
    s32 center_y = 120;
    u16 distance = 200;
    s32 fog_near = 11000;
};

struct ProjectedPoint {
    s16 x, y;
    s32 depth;
    s32 fog;
};

VECTOR render_transform_point(const MATRIX &model, const SVECTOR &point);
void render_place_model(MATRIX &model, const MATRIX &view, const SVECTOR &position);
ProjectedPoint render_project_point(const MATRIX &model, const Projection &projection,
    const SVECTOR &point);
}
