#ifndef KF_GAME_RENDER_H
#define KF_GAME_RENDER_H

/*
 * Rendering / TMD / matrix / lighting / display prototypes.
 *
 * Generated during extern-crutch removal: declarations that were duplicated
 * as `extern` across src/game/*.c now live here once. DAT_/func_ spellings
 * remain unresolved WIP identities. Byte-neutral: a declaration never changes
 * codegen.
 */

#include <kf/semantic_types.h>

/* Texture rectangle followed by its screen-space rectangle. */
typedef struct KfSpriteQuad {
    u8 u;
    u8 v;
    u8 u_span;
    u8 v_span;
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} KfSpriteQuad;

typedef char KfSpriteQuad_size_is_12[(sizeof(KfSpriteQuad) == 0x0c) ? 1 : -1];

extern void display_begin_frame(void);
extern void display_flip_buffer_index(void);
extern void display_initialize(void);
extern void display_play_transition(void);
extern void display_present_frame(void);
extern void fog_interpolate_near(s32 start, s32 end, s32 ratio);
extern void fog_set_near(s32 distance);
extern void lighting_apply_color_preset6(void);
extern void lighting_apply_timed_player_effect(void);
extern void lighting_apply_weapon9_environment(void);
extern void lighting_set_active_color_matrix(s32 index);
extern void lighting_set_color_matrix(
    const MATRIX *from, const MATRIX *to, s32 blend);
extern void matrix_interpolate(
    const MATRIX *from, const MATRIX *to, MATRIX *output, s32 blend);
extern void matrix_set_rotation_x(s16 angle, MATRIX *matrix);
extern void matrix_set_rotation_y(s16 angle, MATRIX *matrix);
extern void matrix_set_rotation_z(s16 angle, MATRIX *matrix);
extern void primitive_buffer_begin_poly_ft4(void);
extern void primitive_buffer_commit_poly_ft4(s32 depth);
extern void render_actor(KfActor *actor);
extern void render_effect_sprites(void);
extern void render_enqueue_model(u16 object_index, s16 depth_bias);
extern void render_enqueue_sprite(KfSpriteQuad *sprite, s16 screen_scale, s32 flag);
extern void render_entities(void);
extern void render_hud_gauges(u8 *table);
extern void render_initialize(void);
extern void render_map_cell(s32 col, s32 row, char cell);
extern void render_map_cells(void);
extern void render_map_event(KfMapEvent *event);
extern void render_map_object(KfMapObject *object);
extern void render_screen_sprite(KfSpriteQuad *sprite);
extern void render_set_view_transform(VECTOR *position, SVECTOR *rotation);
extern void render_weapon(void);
extern void screen_show_image_until_input(const char *path);
extern void sprite_add_ft4(u16 *position, u8 *texcoords, u16 tpage, u16 clut, u8 *color, u16 ot_index);
extern void tmd_prepare_primitive_indices(void);
extern void tmd_project_vertices(s32 count);
extern void tmd_project_vertices_shift(s32 count, u8 shift);
extern void tmd_register(u16 index, void *asset);
extern void tmd_select(u16 index);
extern void tmd_select_object_vertices(u16 index);
extern void tmd_set_current_vertices(SVECTOR *vertices);
extern void tmd_transform_vertices(s32 count);

#endif
