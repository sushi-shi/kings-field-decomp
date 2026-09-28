ADDRESS_AT("OPEN", 0x80017458, 0xa4)
void tmd_project_vertices_perspective_right(s32 count)
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
        projected->p2 = perspective >> KF_GRAPHICS_RUNTIME.tmd_projection_shift;
        ReadSZ2(&depth, &unused_depth);
        projected->sz = depth;
        projected++;
        vertex++;
    }
}
