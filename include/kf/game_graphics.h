#ifndef KF_GAME_GRAPHICS_H
#define KF_GAME_GRAPHICS_H

#include <kf/game_render.h>
#include <kf/game_asset.h>
#include <kf/notify.h>

/* One startup-cleared graphics region; the original declaration is WIP.
 * Registry and projection/morph subobject extents remain unresolved. Their
 * typed source views use the decoded starts without asserting capacities. */
typedef struct KfGraphicsRuntimeGame {
    KfDisplayState display_state;
    DRAWENV display_draw_environments[KF_DISPLAY_BUFFER_COUNT];
    DISPENV display_disp_environments[KF_DISPLAY_BUFFER_COUNT];
    u8 unknown_20108[8];
    KfTmdState tmd_state;
    u8 unknown_registry_20134[0xf0];
    SVECTOR *current_tmd_vertices;
    KfPoolRecord pool_records[KF_ANIMATION_CACHE_CAPACITY];
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
    KfNotificationId notification_message_ids[KF_NOTIFICATION_CAPACITY];
    KfNotificationState notification_state;
    u16 floor_item_clut;
    u16 floor_item_tpage;
    u16 floor_item_count;
    u8 unknown_241fa[6];
    KfFloorItem floor_items[KF_FLOOR_ITEM_CAPACITY];
    u32 DAT_80095698;
    u32 DAT_8009569c;
    KfRenderState render_state;
    MATRIX light_quadrant_matrices[KF_VIEW_QUADRANT_COUNT];
    const KfCellWindow *active_cell_window;
} KfGraphicsRuntimeGame;

extern KfGraphicsRuntimeGame game_graphics_runtime;

#define KF_GAME_GRAPHICS_OFFSET_CHECK(member, offset) \
    typedef char check_game_graphics_##member[ \
        ((unsigned long)&((KfGraphicsRuntimeGame *)0)->member == (offset)) ? 1 : -1]
KF_GAME_GRAPHICS_OFFSET_CHECK(display_state, 0x0);
KF_GAME_GRAPHICS_OFFSET_CHECK(display_draw_environments, 0x20028);
KF_GAME_GRAPHICS_OFFSET_CHECK(display_disp_environments, 0x200e0);
KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_20108, 0x20108);
KF_GAME_GRAPHICS_OFFSET_CHECK(tmd_state, 0x20110);
KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_registry_20134, 0x20134);
KF_GAME_GRAPHICS_OFFSET_CHECK(current_tmd_vertices, 0x20224);
KF_GAME_GRAPHICS_OFFSET_CHECK(pool_records, 0x20228);
KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_projection_morph_20318, 0x20318);
KF_GAME_GRAPHICS_OFFSET_CHECK(effect5_texture_pages, 0x241a0);
KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_241a6, 0x241a6);
KF_GAME_GRAPHICS_OFFSET_CHECK(effect5_texture_cluts, 0x241b0);
KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_241b6, 0x241b6);
KF_GAME_GRAPHICS_OFFSET_CHECK(active_render_clut, 0x241c0);
KF_GAME_GRAPHICS_OFFSET_CHECK(active_render_tpage, 0x241c2);
KF_GAME_GRAPHICS_OFFSET_CHECK(active_render_red, 0x241c4);
KF_GAME_GRAPHICS_OFFSET_CHECK(active_render_green, 0x241c5);
KF_GAME_GRAPHICS_OFFSET_CHECK(active_render_blue, 0x241c6);
KF_GAME_GRAPHICS_OFFSET_CHECK(active_render_code, 0x241c7);
KF_GAME_GRAPHICS_OFFSET_CHECK(hud_clut, 0x241c8);
KF_GAME_GRAPHICS_OFFSET_CHECK(hud_tpage, 0x241ca);
KF_GAME_GRAPHICS_OFFSET_CHECK(hud_brightness, 0x241cc);
KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_241cd, 0x241cd);
KF_GAME_GRAPHICS_OFFSET_CHECK(notification_text_clut, 0x241ce);
KF_GAME_GRAPHICS_OFFSET_CHECK(notification_text_tpage, 0x241d0);
KF_GAME_GRAPHICS_OFFSET_CHECK(notification_digit_clut, 0x241d2);
KF_GAME_GRAPHICS_OFFSET_CHECK(notification_digit_tpage, 0x241d4);
KF_GAME_GRAPHICS_OFFSET_CHECK(notification_message_ids, 0x241d6);
KF_GAME_GRAPHICS_OFFSET_CHECK(notification_state, 0x241de);
KF_GAME_GRAPHICS_OFFSET_CHECK(floor_item_clut, 0x241f4);
KF_GAME_GRAPHICS_OFFSET_CHECK(floor_item_tpage, 0x241f6);
KF_GAME_GRAPHICS_OFFSET_CHECK(floor_item_count, 0x241f8);
KF_GAME_GRAPHICS_OFFSET_CHECK(unknown_241fa, 0x241fa);
KF_GAME_GRAPHICS_OFFSET_CHECK(floor_items, 0x24200);
KF_GAME_GRAPHICS_OFFSET_CHECK(DAT_80095698, 0x24800);
KF_GAME_GRAPHICS_OFFSET_CHECK(DAT_8009569c, 0x24804);
KF_GAME_GRAPHICS_OFFSET_CHECK(render_state, 0x24808);
KF_GAME_GRAPHICS_OFFSET_CHECK(light_quadrant_matrices, 0x24948);
KF_GAME_GRAPHICS_OFFSET_CHECK(active_cell_window, 0x249c8);
#undef KF_GAME_GRAPHICS_OFFSET_CHECK
typedef char check_game_graphics_size[sizeof(KfGraphicsRuntimeGame) == 0x249cc ? 1 : -1];

#endif
