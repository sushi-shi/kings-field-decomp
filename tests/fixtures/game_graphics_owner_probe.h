#ifndef KF_GAME_GRAPHICS_OWNER_PROBE_H
#define KF_GAME_GRAPHICS_OWNER_PROBE_H

#include <kf/game_render.h>
#include <kf/notify.h>

/* Candidate view of the complete startup clear, not a production DATA owner.
 * Registry/projection/morph capacities remain unproved; these opaque spans
 * are not used as typed arrays by the pilot consumers. */
typedef struct KfGraphicsOwnerProbe {
    KfDisplayState display_state;
    DRAWENV display_draw_environments[2];
    DISPENV display_disp_environments[2];
    u8 unknown_20108[8];
    KfTmdState tmd_state;
    u8 unknown_registry_20134[0xf0];
    SVECTOR *current_tmd_vertices;
    KfPoolRecord pool_records[12];
    u8 unknown_projection_morph_20318[0x3e88];
    u16 effect5_texture_pages[3];
    u8 unknown_241a6[10];
    u16 effect5_texture_cluts[3];
    u8 unknown_241b6[10];
    u16 active_render_clut;
    u16 active_render_tpage;
    u8 active_render_red;
    u8 active_render_green;
    u8 active_render_blue;
    u8 active_render_code;
    u16 hud_clut;
    u16 hud_tpage;
    u8 hud_brightness;
    u8 unknown_241cd;
    u16 notification_text_clut;
    u16 notification_text_tpage;
    u16 notification_digit_clut;
    u16 notification_digit_tpage;
    u8 notification_message_ids[8];
    KfNotificationState notification_state;
    u16 floor_item_clut;
    u16 floor_item_tpage;
    u16 floor_item_count;
    u8 unknown_241fa[6];
    KfFloorItem floor_items[64];
    u32 DAT_80095698;
    u32 DAT_8009569c;
    KfRenderState render_state;
    MATRIX light_quadrant_matrices[4];
    const KfCellWindow *active_cell_window;
} KfGraphicsOwnerProbe;

extern KfGraphicsOwnerProbe graphics_owner_probe;

#endif
