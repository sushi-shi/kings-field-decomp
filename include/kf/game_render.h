#ifndef KF_GAME_RENDER_H
#define KF_GAME_RENDER_H

/* GAME.EXE rendering, display, lighting, and TMD state and operations. */

#include <kf/game_actor.h>
#include <kf/game_effect.h>
#include <kf/game_map.h>
#include <kf/item.h>
#include <kf/pool.h>
#include <kf/render_types.h>
#include <kf/tmd.h>

enum {
    KF_SYSTEM_SCREEN_CD_SEARCH_FAILED = 0,
    KF_SYSTEM_SCREEN_CD_READ_FAILED = 1,
    KF_SYSTEM_SCREEN_NO_MEMORY_CARD = 2,
    KF_SYSTEM_SCREEN_PAUSE = 3
};

enum {
    KF_SYSTEM_SCREEN_LEFT = 32,
    KF_SYSTEM_SCREEN_TOP = 112,
    KF_SYSTEM_SCREEN_RIGHT = 288,
    KF_SYSTEM_SCREEN_BOTTOM = 240,
    KF_SYSTEM_SCREEN_U_SPAN = 255,
    KF_SYSTEM_SCREEN_V_SPAN = 128,
    KF_SYSTEM_SCREEN_TPAGE_X = 960,
    KF_SYSTEM_SCREEN_CLUT_Y = 501
};

enum {
    KF_GAME_COLOR_PRESET_COUNT = 7,
    KF_GAME_COLOR_DEFAULT = 0,
    KF_GAME_COLOR_DAMAGE = 1,
    KF_GAME_COLOR_DEFENSE_EFFECT = 2,
    KF_GAME_COLOR_WHITE = 3,
    KF_GAME_COLOR_BLACK = 4,
    KF_GAME_COLOR_GREEN = 5,
    KF_GAME_COLOR_BLUE = 6,
    KF_HUD_DEFAULT_BRIGHTNESS = 86
};

/* One row of the sentinel-terminated HUD gauge and status-sprite table. */
typedef struct KfHudSprite {
    u8 state;
    u8 unknown_01;
    KfSpriteQuad sprite;
} KfHudSprite;

/* Animated screen-facing model entry; state 0xff terminates the list. */
typedef struct KfEffectSprite {
    u8 state;
    u8 visibility_tag;
    u16 asset_variant;
    u16 scale;
    s16 translation_x;
    s16 translation_y;
    s16 translation_z;
    u8 unknown_0c[2];
    SVECTOR rotation;
    u8 unknown_16[2];
    KfPoolRecord *animation_cache;
} KfEffectSprite;

/* Double-buffered GAME.EXE display state. */
typedef struct KfDisplayState {
    u8 buffer_index;
    u8 unknown_01[3];
    void *asset_load_buffer;
    KfPrimitiveBuffer primitive_buffers[KF_DISPLAY_BUFFER_COUNT];
    KfPrimitiveBuffer *primitive_buffer;
    KfOrderingTable ordering_tables[KF_DISPLAY_BUFFER_COUNT];
    u32 *ordering_table;
} KfDisplayState;

/* Eight registered TMD slots and the selected asset. */
typedef struct KfTmdState {
    u8 *slots[8];
    void *current_asset;
} KfTmdState;

/* GAME.EXE view, lighting, fog, and quadrant matrices. */
typedef struct KfRenderState {
    MATRIX view_matrix;
    MATRIX pitch_matrix;
    MATRIX light_matrix;
    MATRIX light_matrix_copy;
    MATRIX effect_color_matrix;
    s32 fog_near_distance;
    VECTOR view_position;
    SVECTOR view_rotation;
    struct KfVecXZs view_cell;
    MATRIX quadrant_matrices[KF_VIEW_QUADRANT_COUNT];
} KfRenderState;

/* Copied vertices begin at element 1; the complete capacity is unresolved. */
extern SVECTOR tmd_morph_scratch[];
extern MATRIX color_matrix_table[KF_GAME_COLOR_PRESET_COUNT];
extern POLY_FT4 *current_poly_ft4;
extern DRAWENV display_draw_environments[KF_DISPLAY_BUFFER_COUNT];
extern DISPENV display_disp_environments[KF_DISPLAY_BUFFER_COUNT];
extern KfDisplayState display_state;
extern KfCellWindow render_cell_windows[KF_CELL_WINDOW_YAW_COUNT];
extern const KfCellWindow *active_cell_window;
extern u16 effect5_texture_pages[3];
extern u16 effect5_texture_cluts[3];
extern KfEffectSprite effect_sprites[2];
extern KfHudSprite hud_sprites[14];
extern MATRIX light_quadrant_matrices[KF_VIEW_QUADRANT_COUNT];
extern MATRIX render_light_matrices[6];
extern KfRenderState render_state;
extern KfTmdState tmd_state;
/* GAME's projected-array capacity remains unresolved. */
extern SVECTOR *current_tmd_vertices;
extern KfScreenVertex tmd_projected_vertices[];
extern u16 active_render_clut;
extern u16 active_render_tpage;
/* Contiguous RGB/command bytes; the enclosing graphics object is unresolved. */
extern u8 active_render_red;
extern u8 active_render_green;
extern u8 active_render_blue;
extern u8 active_render_code;

extern void display_begin_frame(void);
extern void display_flip_buffer_index(void);
extern void display_initialize(void);
extern void display_play_transition(void);
extern void display_present_frame(void);
extern void display_show_error_screen(s32 stage);
extern void effect5_texture_cache_prepare(s32 mode);
extern void fog_interpolate_near(s32 start, s32 end, s32 ratio);
extern void fog_set_near(s32 distance);
extern void lighting_apply_color_preset6(void);
extern void lighting_apply_timed_player_effect(void);
extern void lighting_apply_weapon9_environment(void);
extern void lighting_set_active_color_matrix(s32 index);
extern void lighting_set_color_matrix(
    const MATRIX *from, const MATRIX *to, s32 blend);
extern void menu_render_item_model(void);
extern void primitive_buffer_begin_poly_ft4(void);
extern void primitive_buffer_commit_poly_ft4(s32 depth);
extern void render_actor(KfActor *actor);
extern void render_actor_sprite(KfEffectRenderView *sprite);
extern void render_effect_sprites(void);
extern void render_enqueue_map(u16 object_index);
extern void render_enqueue_model(u16 object_index, s16 depth_bias);
extern void render_enqueue_sprite(KfSpriteQuad *sprite, s16 screen_scale, s32 flag);
extern void render_enqueue_tmd(u16 object_index, s16 depth_bias);
extern void render_entities(void);
extern void render_frame(
    const VECTOR *position_or_null, const SVECTOR *rotation_or_null);
extern void render_hud_gauges(KfHudSprite *table);
extern void render_initialize(void);
extern void render_map_cell(s32 col, s32 row, char cell);
extern void render_map_cells(void);
extern void render_map_event(KfMapEvent *event);
extern void render_map_object(KfMapObject *object);
extern void render_screen_sprite(KfSpriteQuad *sprite);
extern void render_set_view_transform(
    const VECTOR *position_or_null, const SVECTOR *rotation_or_null);
extern void render_weapon(void);
extern void screen_show_image_until_input(const char *path);
extern void sprite_add_ft4(u16 *position, u8 *texcoords, u32 tpage, u32 clut, u8 *color, u16 ot_index);
extern void tmd_project_vertices_shift(s32 count, u8 shift);
extern void tmd_transform_vertices(s32 count);

#endif
