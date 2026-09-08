#include <kf/open_render.h>

#ifndef EXPECTED_FT3_SIZE
#define EXPECTED_FT3_SIZE 32
#endif

#define OFFSET(type, field) ((unsigned long)&((type *)0)->field)
#define CHECK_OFFSET(type, field, value) \
    typedef char type##_##field##_offset[OFFSET(type, field) == value ? 1 : -1]

typedef char tmd_flat_size[sizeof(KfTmdF3) == 12 ? 1 : -1];
CHECK_OFFSET(KfTmdF3, r, 0);
CHECK_OFFSET(KfTmdF3, g, 1);
CHECK_OFFSET(KfTmdF3, b, 2);
CHECK_OFFSET(KfTmdF3, v0, 6);
CHECK_OFFSET(KfTmdF3, v1, 8);
CHECK_OFFSET(KfTmdF3, v2, 10);
typedef char tmd_textured_size[sizeof(KfTmdFt3) == 20 ? 1 : -1];
CHECK_OFFSET(KfTmdFt3, tu0, 0);
CHECK_OFFSET(KfTmdFt3, cba, 2);
CHECK_OFFSET(KfTmdFt3, tu1, 4);
CHECK_OFFSET(KfTmdFt3, tsb, 6);
CHECK_OFFSET(KfTmdFt3, tu2, 8);
CHECK_OFFSET(KfTmdFt3, v0, 14);
CHECK_OFFSET(KfTmdFt3, v1, 16);
CHECK_OFFSET(KfTmdFt3, v2, 18);
typedef char screen_vertex_size[sizeof(KfScreenVertex) == 8 ? 1 : -1];
CHECK_OFFSET(KfScreenVertex, sxy, 0);
CHECK_OFFSET(KfScreenVertex, sz, 4);
CHECK_OFFSET(KfScreenVertex, p2, 6);
typedef char packed_xy_size[sizeof(long) == 4 ? 1 : -1];
typedef char screen_halfword_view_size[sizeof(DVECTOR) == 4 ? 1 : -1];
CHECK_OFFSET(DVECTOR, vx, 0);
CHECK_OFFSET(DVECTOR, vy, 2);

typedef struct TargetLongAlignment {
    u8 before;
    long word;
    u8 after;
} TargetLongAlignment;
typedef char target_long_alignment[sizeof(TargetLongAlignment) == 12 ? 1 : -1];
CHECK_OFFSET(TargetLongAlignment, word, 4);
CHECK_OFFSET(TargetLongAlignment, after, 8);

long screen_vertex_read_xy(const KfScreenVertex *vertex)
{
    return vertex->sxy.word;
}

void screen_vertex_write_xy(KfScreenVertex *vertex, long xy)
{
    vertex->sxy.word = xy;
}

typedef char primitive_count_size[
    sizeof(((KfTmdObject *)0)->primitive_count) == 4 ? 1 : -1];
CHECK_OFFSET(KfTmdObject, primitive_count, 20);
typedef char gpu_flat_size[sizeof(POLY_F3) == 20 ? 1 : -1];
CHECK_OFFSET(POLY_F3, r0, 4);
CHECK_OFFSET(POLY_F3, x0, 8);
CHECK_OFFSET(POLY_F3, x1, 12);
CHECK_OFFSET(POLY_F3, x2, 16);
typedef char gpu_textured_size[sizeof(POLY_FT3) == EXPECTED_FT3_SIZE ? 1 : -1];
CHECK_OFFSET(POLY_FT3, r0, 4);
CHECK_OFFSET(POLY_FT3, x0, 8);
CHECK_OFFSET(POLY_FT3, u0, 12);
CHECK_OFFSET(POLY_FT3, clut, 14);
CHECK_OFFSET(POLY_FT3, x1, 16);
CHECK_OFFSET(POLY_FT3, u1, 20);
CHECK_OFFSET(POLY_FT3, tpage, 22);
CHECK_OFFSET(POLY_FT3, x2, 24);
CHECK_OFFSET(POLY_FT3, u2, 28);
