#ifndef KF_GAME_GRAPHICS_H
#define KF_GAME_GRAPHICS_H

#include <kf/game/render.h>
#include <kf/game/asset.h>
#include <kf/game/notify.h>

enum {
    KF_FLOOR5_ACTOR_TEXTURE_COUNT = 3,

    KF_MORPH_SCRATCH_CAPACITY = KF_PROJECTED_VERTEX_CAPACITY
};

typedef struct KfGraphicsRuntimeGame {
    KfDisplayState display_state;
    u8 unknown_20108[8];
    KfTmdState tmd_state;
    KfAssetHeader *asset_registry_entries[KF_ASSET_REGISTRY_KNOWN_ENTRIES];
    KfTmdResource asset_registry_tmds[KF_ASSET_REGISTRY_KNOWN_ENTRIES];
    u8 unknown_201f4[0x30];
    SVECTOR *current_tmd_vertices;
    KfPoolRecord pool_records[KF_ANIMATION_CACHE_CAPACITY];
    KfScreenVertex tmd_projected_vertices[KF_PROJECTED_VERTEX_CAPACITY];
    SVECTOR morph_scratch[KF_MORPH_SCRATCH_CAPACITY];
    kf::FaceMaterial effect5_materials[KF_FLOOR5_ACTOR_TEXTURE_COUNT];
    kf::FaceMaterial active_render_material;
    CVECTOR active_render_color;
    kf::FaceMaterial hud_material;
    u8 hud_brightness;
    u8 unknown_241cd;
    kf::FaceMaterial notification_text_material;
    kf::FaceMaterial notification_digit_material;
    KfNotificationId notification_message_ids[KF_NOTIFICATION_CAPACITY];
    KfNotificationState notification_state;
    kf::FaceMaterial floor_item_material;
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
