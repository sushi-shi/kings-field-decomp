void tmd_project_vertices_perspective_right(s32 count)
{
    KfScreenVertex *out;
    SVECTOR *vertex;
    long perspective;
    long flag;
    long depth;
    long unused_depth;

    out = KF_GRAPHICS_RUNTIME.tmd_projected_vertices;
    vertex = KF_GRAPHICS_RUNTIME.current_tmd_vertices;
    for (count--; count != -1; count--) {
        RotTransPers(vertex, &out->sxy.word, &perspective, &flag);
        out->p2 = perspective >> KF_GRAPHICS_RUNTIME.tmd_projection_shift;
        ReadSZ2(&depth, &unused_depth);
        out->sz = depth;
        out++;
        vertex++;
    }
}
