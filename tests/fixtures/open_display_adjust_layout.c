#include <kf/open_render.h>
#include <LIBETC.H>

#ifndef EXPECTED_DFE_OFFSET
#define EXPECTED_DFE_OFFSET 23
#endif

#define OFFSET(type, field) ((unsigned long)&((type *)0)->field)
#define CHECK_OFFSET(type, field, value) \
    typedef char type##_##field##_offset[OFFSET(type, field) == value ? 1 : -1]

typedef char rect_size[sizeof(RECT) == 8 ? 1 : -1];
CHECK_OFFSET(RECT, x, 0);
CHECK_OFFSET(RECT, y, 2);
CHECK_OFFSET(RECT, w, 4);
CHECK_OFFSET(RECT, h, 6);
typedef struct RectAlignment {
    char before;
    RECT rectangle;
} RectAlignment;
CHECK_OFFSET(RectAlignment, rectangle, 2);
typedef char drawenv_size[sizeof(DRAWENV) == 92 ? 1 : -1];
CHECK_OFFSET(DRAWENV, dtd, 22);
CHECK_OFFSET(DRAWENV, dfe, EXPECTED_DFE_OFFSET);
CHECK_OFFSET(DRAWENV, isbg, 24);
typedef char dispenv_size[sizeof(DISPENV) == 20 ? 1 : -1];
CHECK_OFFSET(DISPENV, disp, 0);
CHECK_OFFSET(KfDisplayStateOpen, buffer_index, 0);
typedef char buffer_index_size[
    sizeof(((KfDisplayStateOpen *)0)->buffer_index) == 1 ? 1 : -1];
typedef char pad_h_mask[PADh == 0x800 ? 1 : -1];
typedef char pad_right_mask[PADLright == 0x2000 ? 1 : -1];
typedef char pad_left_mask[PADLleft == 0x8000 ? 1 : -1];
typedef char pad_up_mask[PADLup == 0x1000 ? 1 : -1];
typedef char pad_down_mask[PADLdown == 0x4000 ? 1 : -1];
