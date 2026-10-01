#include <kf/platform/prelude.h>
#include <kf/cutscene/render.h>
#include <kf/lib/debug.h>
#include <kf/lib/map_data.h>
#include <kf/lib/memory.h>
#include <kf/lib/null.h>
#include <kf/lib/tmd.h>

void cutscene_tmd_project_vertices(s32 count, const MATRIX *model, const kf::Projection &projection)
{
    KfScreenVertex *projected;
    const SVECTOR *vertex;

    projected = open_graphics_runtime.tmd_projected_vertices.data();
    vertex = tmd_vertices(cutscene_tmd_context(), count).data();
    for (count--; count != -1; count--) {
        const auto point = kf::render_project_point(*model, projection, *vertex);
        projected->position = {point.x, point.y};
        projected->p2 = point.fog << open_graphics_runtime.tmd_projection_shift;
        projected->sz = point.depth;
        projected++;
        vertex++;
    }
}

void tmd_project_vertices_perspective_right(s32 count, const MATRIX *model, const kf::Projection &projection)
{
    KfScreenVertex *out;
    const SVECTOR *vertex;

    out = open_graphics_runtime.tmd_projected_vertices.data();
    vertex = tmd_vertices(cutscene_tmd_context(), count).data();
    for (count--; count != -1; count--) {
        const auto point = kf::render_project_point(*model, projection, *vertex);
        out->position = {point.x, point.y};
        out->p2 = point.fog >> open_graphics_runtime.tmd_projection_shift;
        out->sz = point.depth;
        out++;
        vertex++;
    }
}
