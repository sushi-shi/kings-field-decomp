#ifndef KF_OPEN_RENDER_H
#define KF_OPEN_RENDER_H

/* OPEN.EXE display, render, and TMD state shared across render units. */

#include <kf/semantic_types.h>

extern KfDisplayStateOpen display_state;
extern KfRenderStateOpen render_state;
extern KfTmdStateOpen tmd_state;
extern DRAWENV display_draw_environments[2];
extern DISPENV display_disp_environments[2];
extern u32 *ordering_table;
extern SVECTOR *current_tmd_vertices;
extern MATRIX light_quadrant_matrices[4];
extern MATRIX color_matrix_table[5];

extern u32 DAT_800439d8;
extern u16 DAT_8006da36;
extern u16 DAT_8006da38;
extern u16 DAT_8006da3a;
extern u32 DAT_8006e040;
extern u32 DAT_8006e044;
extern u32 DAT_80075928;

extern void lighting_set_active_color_matrix(s32 index);
extern void render_initialize(void);
extern void display_begin_frame(void);
extern void display_present_frame(void);
extern void tmd_select(u16 index);
extern KfTmdObject *tmd_get_object(u16 index);
extern void tmd_set_current_vertices(SVECTOR *vertices);
extern void tmd_select_object_vertices(u16 index);
extern void render_set_view_transform(
    const VECTOR *position, const SVECTOR *rotation);
extern void tmd_prepare_primitive_indices(void);
extern void tmd_register(u16 slot, u8 *tmd);
extern void tmd_release_last_allocation(s32 slot);

#endif
