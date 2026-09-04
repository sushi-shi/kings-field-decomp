#include <kf/open_render.h>

#ifndef EXPECTED_MATERIAL_SIZE
#define EXPECTED_MATERIAL_SIZE 8
#endif

#define FIELD_OFFSET(type, field) ((unsigned long)&((type *)0)->field)
#define CHECK_OFFSET(type, field, offset) \
    typedef char type##_##field##_offset[FIELD_OFFSET(type, field) == offset ? 1 : -1]

typedef char sprite_material_size[
    sizeof(KfSpriteMaterial) == EXPECTED_MATERIAL_SIZE ? 1 : -1];
CHECK_OFFSET(KfSpriteMaterial, clut, 0);
CHECK_OFFSET(KfSpriteMaterial, tpage, 2);
CHECK_OFFSET(KfSpriteMaterial, color, 4);

#ifndef EXPECTED_ITEM_STATE_SIZE
#define EXPECTED_ITEM_STATE_SIZE 0x618
#endif
typedef char floor_item_state_size[
    sizeof(KfFloorItemStateOpen) == EXPECTED_ITEM_STATE_SIZE ? 1 : -1];
CHECK_OFFSET(KfFloorItemStateOpen, material, 0);
CHECK_OFFSET(KfFloorItemStateOpen, unknown_08, 8);
CHECK_OFFSET(KfFloorItemStateOpen, texture_clut, 14);
CHECK_OFFSET(KfFloorItemStateOpen, texture_tpage, 16);
CHECK_OFFSET(KfFloorItemStateOpen, count, 18);
CHECK_OFFSET(KfFloorItemStateOpen, unknown_14, 20);
CHECK_OFFSET(KfFloorItemStateOpen, items, 24);

typedef char sdk_color_size[sizeof(CVECTOR) == 4 ? 1 : -1];
CHECK_OFFSET(CVECTOR, r, 0);
CHECK_OFFSET(CVECTOR, g, 1);
CHECK_OFFSET(CVECTOR, b, 2);
CHECK_OFFSET(CVECTOR, cd, 3);
typedef char sdk_normal_size[sizeof(SVECTOR) == 8 ? 1 : -1];
CHECK_OFFSET(SVECTOR, vx, 0);
CHECK_OFFSET(SVECTOR, vy, 2);
CHECK_OFFSET(SVECTOR, vz, 4);
typedef char sdk_matrix_size[sizeof(MATRIX) == 32 ? 1 : -1];
CHECK_OFFSET(MATRIX, m, 0);
CHECK_OFFSET(MATRIX, t, 20);

typedef char gpu_quad_size[sizeof(POLY_FT4) == 40 ? 1 : -1];
CHECK_OFFSET(POLY_FT4, r0, 4);
CHECK_OFFSET(POLY_FT4, code, 7);
CHECK_OFFSET(POLY_FT4, x0, 8);
CHECK_OFFSET(POLY_FT4, clut, 14);
CHECK_OFFSET(POLY_FT4, x1, 16);
CHECK_OFFSET(POLY_FT4, tpage, 22);
CHECK_OFFSET(POLY_FT4, x2, 24);
CHECK_OFFSET(POLY_FT4, x3, 32);
