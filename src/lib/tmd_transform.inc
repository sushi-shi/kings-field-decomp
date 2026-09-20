void tmd_project_vertices_shift(s32 count, u8 shift, const MATRIX *model, const kf::Projection &projection)
{
    if (count < 0 || count > KF_PROJECTED_VERTEX_CAPACITY)
        kf::host_fail("Model exceeds projected vertex capacity.");
    KfScreenVertex *projected;
    SVECTOR *vertex;

    projected = KF_GRAPHICS_RUNTIME.tmd_projected_vertices;
    vertex = KF_GRAPHICS_RUNTIME.current_tmd_vertices;
    for (count--; count != -1; count--) {
        const auto point = kf::render_project_point(*model, projection, *vertex);
        projected->sxy.vector = {point.x, point.y};
        projected->p2 = point.fog << KF_TMD_DEFAULT_PERSPECTIVE_SHIFT;
        projected->sz = point.depth >> shift;
        projected++;
        vertex++;
    }
}

void tmd_transform_vertices(s32 count, const MATRIX *model)
{
    if (count < 0 || count > KF_PROJECTED_VERTEX_CAPACITY)
        kf::host_fail("Model exceeds projected vertex capacity.");
    KfScreenVertex *projected;
    SVECTOR *vertex;
    VECTOR transformed;
    s32 remaining;

    projected = KF_GRAPHICS_RUNTIME.tmd_projected_vertices;
    vertex = KF_GRAPHICS_RUNTIME.current_tmd_vertices;
    for (remaining = count - 1; remaining != -1; remaining--) {
        transformed = kf::render_transform_point(*model, *vertex);
        projected->sxy.vector.vx = transformed.vx;
        projected->sxy.vector.vy = transformed.vy;
        projected->p2 = transformed.vz;
        projected->sz = transformed.vz;
        projected++;
        vertex++;
    }
}
