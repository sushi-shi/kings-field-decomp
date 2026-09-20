#include <kf/lib/null.h>

#include <kf/lib/map_data.h>
#include <kf/lib/memory.h>
#include <kf/lib/debug.h>
#include <kf/open/render.h>
#include <kf/lib/tmd.h>
#include <kf/lib/graphics.h>

#include "../lib/display_frame.inc"

#include "../lib/tmd.inc"

void tmd_project_vertices(s32 count, const MATRIX *model, const kf::Projection &projection)
{
    KfScreenVertex *out;
    SVECTOR *vertex;

    if (count < 0 || count > KF_PROJECTED_VERTEX_CAPACITY)
        kf::host_fail("Model exceeds projected vertex capacity.");
    out = open_graphics_runtime.tmd_projected_vertices;
    vertex = open_graphics_runtime.current_tmd_vertices;
    for (count--; count != -1; count--) {
        const auto point = kf::render_project_point(*model, projection, *vertex);
        out->sxy.vector = {point.x, point.y};
        out->p2 = point.fog << open_graphics_runtime.tmd_projection_shift;
        out->sz = point.depth;
        out++;
        vertex++;
    }
}

#include "../lib/tmd_perspective_right.inc"

#include "../lib/tmd_transform.inc"
