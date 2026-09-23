void tmd_project_vertices_shift(s32 count, u8 shift)
{
    KfScreenVertex *projected;
    SVECTOR *vertex;
    long perspective;
    long gte_flags;
    long depth;
    long unused_depth;

    projected = KF_GRAPHICS_RUNTIME.tmd_projected_vertices;
    vertex = KF_GRAPHICS_RUNTIME.current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTransPers(vertex, &projected->sxy.word, &perspective, &gte_flags);
        projected->p2 = perspective << KF_TMD_DEFAULT_PERSPECTIVE_SHIFT;
        ReadSZ2(&depth, &unused_depth);
        projected->sz = depth >> shift;
        projected++;
        vertex++;
    }
}

void tmd_transform_vertices(s32 count)
{
    KfScreenVertex *projected;
    SVECTOR *vertex;
    VECTOR transformed;
    long gte_flags;
    s32 remaining;

    projected = KF_GRAPHICS_RUNTIME.tmd_projected_vertices;
    vertex = KF_GRAPHICS_RUNTIME.current_tmd_vertices;
    for (remaining = count - 1; remaining != -1; remaining--) {
        RotTrans(vertex, &transformed, &gte_flags);
        projected->sxy.vector.vx = transformed.vx;
        projected->sxy.vector.vy = transformed.vy;
        projected->p2 = transformed.vz;
        projected->sz = transformed.vz;
        projected++;
        vertex++;
    }
}
