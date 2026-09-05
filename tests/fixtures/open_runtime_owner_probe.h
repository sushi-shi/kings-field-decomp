#ifndef KF_OPEN_RUNTIME_OWNER_PROBE_H
#define KF_OPEN_RUNTIME_OWNER_PROBE_H
#include <kf/open_render.h>
/* Temporary coherent candidate: cleared OPEN RAM 80049a48..8006e1d0.
 * Unknown arrays represent real uncovered intervals, not extra codegen padding.
 * No curated ownership change or original-source claim is made by this probe. */
typedef struct KfGraphicsRuntimeOpenProbe {
    KfDisplayStateOpen display_state;
    u32 *ordering_table;
    DRAWENV display_draw_environments[2];
    DISPENV display_disp_environments[2];
    u8 unknown_20108[8];
    KfTmdStateOpen tmd_state;
    u8 unknown_2011c[4];
    SVECTOR *current_tmd_vertices;
    u8 unknown_20124[0x14];
    KfScreenVertex tmd_projected_vertices[1000];
    u8 unknown_22078[0x1f68];
    KfFloorItemStateOpen floor_item_state;
    u32 DAT_8006e040;
    u32 DAT_8006e044;
    KfRenderStateOpen render_state;
    MATRIX light_quadrant_matrices[4];
    const KfCellWindow *active_cell_window;
    s16 tmd_projection_shift;
    u8 unknown_24786[2];
} KfGraphicsRuntimeOpenProbe;
extern KfGraphicsRuntimeOpenProbe open_graphics_runtime;
typedef char check_display_state[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->display_state == 0x0) ? 1 : -1];
typedef char check_ordering_table[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->ordering_table == 0x20024) ? 1 : -1];
typedef char check_display_draw_environments[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->display_draw_environments == 0x20028) ? 1 : -1];
typedef char check_display_disp_environments[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->display_disp_environments == 0x200e0) ? 1 : -1];
typedef char check_unknown_20108[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->unknown_20108 == 0x20108) ? 1 : -1];
typedef char check_tmd_state[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->tmd_state == 0x20110) ? 1 : -1];
typedef char check_unknown_2011c[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->unknown_2011c == 0x2011c) ? 1 : -1];
typedef char check_current_tmd_vertices[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->current_tmd_vertices == 0x20120) ? 1 : -1];
typedef char check_unknown_20124[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->unknown_20124 == 0x20124) ? 1 : -1];
typedef char check_tmd_projected_vertices[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->tmd_projected_vertices == 0x20138) ? 1 : -1];
typedef char check_unknown_22078[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->unknown_22078 == 0x22078) ? 1 : -1];
typedef char check_floor_item_state[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->floor_item_state == 0x23fe0) ? 1 : -1];
typedef char check_DAT_8006e040[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->DAT_8006e040 == 0x245f8) ? 1 : -1];
typedef char check_DAT_8006e044[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->DAT_8006e044 == 0x245fc) ? 1 : -1];
typedef char check_render_state[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->render_state == 0x24600) ? 1 : -1];
typedef char check_light_quadrant_matrices[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->light_quadrant_matrices == 0x24700) ? 1 : -1];
typedef char check_active_cell_window[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->active_cell_window == 0x24780) ? 1 : -1];
typedef char check_tmd_projection_shift[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->tmd_projection_shift == 0x24784) ? 1 : -1];
typedef char check_unknown_24786[((unsigned long)&((KfGraphicsRuntimeOpenProbe *)0)->unknown_24786 == 0x24786) ? 1 : -1];
typedef char check_runtime_size[sizeof(KfGraphicsRuntimeOpenProbe) == 0x24788 ? 1 : -1];
#endif
