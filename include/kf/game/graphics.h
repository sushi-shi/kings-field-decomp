#ifndef KF_GAME_GRAPHICS_H
#define KF_GAME_GRAPHICS_H

#include <kf/game/render.h>
#include <kf/game/asset.h>
#include <kf/game/notify.h>

enum {
    KF_FLOOR5_ACTOR_TEXTURE_COUNT = 3,

    KF_MORPH_SCRATCH_CAPACITY = 1001
};

typedef struct KfGraphicsRuntimeGame {
    KfDisplayState display_state;
    DRAWENV display_draw_environments[KF_DISPLAY_BUFFER_COUNT];
    DISPENV display_disp_environments[KF_DISPLAY_BUFFER_COUNT];
    u8 unknown_20108[8];
    KfTmdState tmd_state;
    KfAssetHeader *asset_registry_entries[KF_ASSET_REGISTRY_KNOWN_ENTRIES];
    u8 unknown_201f4[0x30];
    SVECTOR *current_tmd_vertices;
    KfPoolRecord pool_records[KF_ANIMATION_CACHE_CAPACITY];
    KfScreenVertex tmd_projected_vertices[KF_PROJECTED_VERTEX_CAPACITY];
    SVECTOR morph_scratch[KF_MORPH_SCRATCH_CAPACITY];
    u16 effect5_texture_pages[KF_FLOOR5_ACTOR_TEXTURE_COUNT];
    u8 unknown_241a6[10];
    u16 effect5_texture_cluts[KF_FLOOR5_ACTOR_TEXTURE_COUNT];
    u8 unknown_241b6[10];
    u16 active_render_clut;
    u16 active_render_tpage;
    CVECTOR active_render_color;
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

#endif
