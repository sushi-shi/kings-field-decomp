#ifndef KF_GAME_GRAPHICS_STANDALONE_CONTROL_H
#define KF_GAME_GRAPHICS_STANDALONE_CONTROL_H

/* Rejected separate declarations for compiler controls only. */
#include <kf/game_graphics.h>
#define INITIAL_GRAPHICS_CLEAR_BYTES 0x249cc

extern KfDisplayState display_state;
extern DRAWENV display_draw_environments[2];
extern DISPENV display_disp_environments[2];
extern KfTmdState tmd_state;
extern KfPackedSVector *current_tmd_vertices;
extern KfPoolRecord pool_records[12];
extern u16 effect5_texture_pages[3];
extern u16 effect5_texture_cluts[3];
extern u16 active_render_clut;
extern u16 active_render_tpage;
extern u8 active_render_red;
extern u8 active_render_green;
extern u8 active_render_blue;
extern u8 active_render_code;
extern u16 hud_clut;
extern u16 hud_tpage;
extern u8 hud_brightness;
extern u16 notification_text_clut;
extern u16 notification_text_tpage;
extern u16 notification_digit_clut;
extern u16 notification_digit_tpage;
extern u8 notification_message_ids[8];
extern KfNotificationState notification_state;
extern u16 floor_item_clut;
extern u16 floor_item_tpage;
extern u16 floor_item_count;
extern KfFloorItem floor_items[64];
extern u32 DAT_80095698;
extern u32 DAT_8009569c;
extern KfRenderState render_state;
extern MATRIX light_quadrant_matrices[4];
extern const KfCellWindow *active_cell_window;
extern KfAssetHeader *asset_registry_entries[];
extern KfScreenVertex tmd_projected_vertices[];
extern KfPackedSVector tmd_morph_scratch[];

#endif
