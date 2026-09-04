#include <kf/open_render.h>

#ifndef EXPECTED_GT4_SIZE
#define EXPECTED_GT4_SIZE 52
#endif

#define OFFSET(type, field) ((unsigned long)&((type *)0)->field)
#define CHECK_OFFSET(type, field, value) \
    typedef char type##_##field##_offset[OFFSET(type, field) == value ? 1 : -1]

typedef char gt3_size[sizeof(POLY_GT3) == 40 ? 1 : -1];
typedef char gt4_size[sizeof(POLY_GT4) == EXPECTED_GT4_SIZE ? 1 : -1];
CHECK_OFFSET(POLY_GT3, code, 7);
CHECK_OFFSET(POLY_GT3, clut, 14);
CHECK_OFFSET(POLY_GT3, tpage, 26);
CHECK_OFFSET(POLY_GT3, r0, 4);
CHECK_OFFSET(POLY_GT3, r1, 16);
CHECK_OFFSET(POLY_GT3, r2, 28);
CHECK_OFFSET(POLY_GT3, x0, 8);
CHECK_OFFSET(POLY_GT3, x1, 20);
CHECK_OFFSET(POLY_GT3, x2, 32);
CHECK_OFFSET(POLY_GT3, u0, 12);
CHECK_OFFSET(POLY_GT3, u1, 24);
CHECK_OFFSET(POLY_GT3, u2, 36);
CHECK_OFFSET(POLY_GT4, code, 7);
CHECK_OFFSET(POLY_GT4, clut, 14);
CHECK_OFFSET(POLY_GT4, tpage, 26);
CHECK_OFFSET(POLY_GT4, r0, 4);
CHECK_OFFSET(POLY_GT4, r1, 16);
CHECK_OFFSET(POLY_GT4, r2, 28);
CHECK_OFFSET(POLY_GT4, r3, 40);
CHECK_OFFSET(POLY_GT4, x0, 8);
CHECK_OFFSET(POLY_GT4, x1, 20);
CHECK_OFFSET(POLY_GT4, x2, 32);
CHECK_OFFSET(POLY_GT4, x3, 44);
CHECK_OFFSET(POLY_GT4, u0, 12);
CHECK_OFFSET(POLY_GT4, u1, 24);
CHECK_OFFSET(POLY_GT4, u2, 36);
CHECK_OFFSET(POLY_GT4, u3, 48);

typedef char ft3_size[sizeof(KfTmdFt3) == 20 ? 1 : -1];
typedef char ft4_size[sizeof(KfTmdFt4) == 28 ? 1 : -1];
CHECK_OFFSET(KfTmdFt3, tu0, 0);
CHECK_OFFSET(KfTmdFt3, cba, 2);
CHECK_OFFSET(KfTmdFt3, tu1, 4);
CHECK_OFFSET(KfTmdFt3, tsb, 6);
CHECK_OFFSET(KfTmdFt3, tu2, 8);
CHECK_OFFSET(KfTmdFt3, n0, 12);
CHECK_OFFSET(KfTmdFt3, v0, 14);
CHECK_OFFSET(KfTmdFt3, v1, 16);
CHECK_OFFSET(KfTmdFt3, v2, 18);
CHECK_OFFSET(KfTmdFt4, tu0, 0);
CHECK_OFFSET(KfTmdFt4, cba, 2);
CHECK_OFFSET(KfTmdFt4, tu1, 4);
CHECK_OFFSET(KfTmdFt4, tsb, 6);
CHECK_OFFSET(KfTmdFt4, tu2, 8);
CHECK_OFFSET(KfTmdFt4, tu3, 12);
CHECK_OFFSET(KfTmdFt4, n0, 16);
CHECK_OFFSET(KfTmdFt4, v0, 18);
CHECK_OFFSET(KfTmdFt4, v1, 20);
CHECK_OFFSET(KfTmdFt4, v2, 22);
CHECK_OFFSET(KfTmdFt4, v3, 24);

typedef char screen_vertex_size[sizeof(KfScreenVertex) == 8 ? 1 : -1];
typedef char screen_word_size[sizeof(((KfScreenVertex *)0)->sxy) == 4 ? 1 : -1];
CHECK_OFFSET(KfScreenVertex, sxy, 0);
CHECK_OFFSET(KfScreenVertex, sz, 4);
CHECK_OFFSET(KfScreenVertex, p2, 6);
typedef char colour_size[sizeof(CVECTOR) == 4 ? 1 : -1];
CHECK_OFFSET(CVECTOR, r, 0);
CHECK_OFFSET(CVECTOR, g, 1);
CHECK_OFFSET(CVECTOR, b, 2);
CHECK_OFFSET(CVECTOR, cd, 3);
CHECK_OFFSET(KfTmdObject, vertex_count, 4);
CHECK_OFFSET(KfTmdObject, normal_offset, 8);
CHECK_OFFSET(KfTmdObject, primitive_offset, 16);
CHECK_OFFSET(KfTmdObject, primitive_count, 20);
typedef char vertex_count_size[
    sizeof(((KfTmdObject *)0)->vertex_count) == 4 ? 1 : -1];
typedef char primitive_count_size[
    sizeof(((KfTmdObject *)0)->primitive_count) == 4 ? 1 : -1];
