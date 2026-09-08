#include <kf/game_graphics.h>
#include <kf/address.h>
#include <kf/game_asset.h>
#include <kf/game_math.h>
#include <kf/game_render.h>
#include <kf/game_state.h>
#include <kf/psyq.h>

/*
 * Per-entity billboard/model emitters invoked by the frame renderer's pool
 * sweep (render_entities).  Each transforms one live entry into view space, binds
 * the matching asset, and hands the result to the shared sprite/model draw
 * helpers.
 */

enum { EFFECT_MODEL_DEPTH_BIAS = 100 };

DATA(0x80055b00, 0x54)
KfSpriteQuad floor_item_sprites[KF_FLOOR_ITEM_SPRITE_COUNT] = {
    {0x90, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xd0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
    {0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
    {0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
};

DATA(0x80055b54, 0x108)
KfSpriteQuad effect_billboard_sprites[KF_EFFECT_BILLBOARD_SPRITE_COUNT] = {
    {0x0, 0x0, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    {0x30, 0x0, 0x2f, 0x2f, 0xfde7, 0xfde7, 0x433, 0x433},
    {0x30, 0x0, 0x2f, 0x2f, 0xfd74, 0xfd74, 0x519, 0x519},
    {0x60, 0x0, 0x2f, 0x2f, 0xfd4d, 0xfd4d, 0x566, 0x566},
    {0x60, 0x0, 0x2f, 0x2f, 0xfd3a, 0xfd3a, 0x58c, 0x58c},
    {0x0, 0x30, 0x2f, 0x2f, 0xfe34, 0xfe34, 0x399, 0x399},
    {0x0, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    {0x30, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    {0x60, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    {0x30, 0x30, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    {0x60, 0x60, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    {0xe0, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},
    {0xe0, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},
    {0xe0, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},
    {0xe0, 0x48, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},
    {0xe0, 0x76, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},
    {0xe0, 0xa2, 0x17, 0x5c, 0xfed4, 0xec78, 0x258, 0x1388},
    {0x0, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    {0x30, 0x90, 0x2f, 0x2f, 0xfe80, 0xfe80, 0x300, 0x300},
    {0xc8, 0x48, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},
    {0xc8, 0x76, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},
    {0xc8, 0xa2, 0x17, 0x5c, 0xff38, 0xec78, 0x190, 0x1388},
};

/*
 * Emits one floor decoration.  The record's world position is carried into the
 * view via RotTrans (its result lands directly in the model matrix's t column),
 * then either a facing rotation is composed onto the view matrix or the render
 * pitch matrix is used verbatim.  The sprite frame advances and wraps against
 * the low nibble of the packed facing/frame-count byte.
 */
ADDRESS(0x8001ed90, 0x14c)
void render_floor_item(KfFloorItem *item)
{
    SVECTOR screen;
    MATRIX model;
    long flag;
    u16 facing;
    u32 next_frame;
    u32 frame_count;
    s16 depth_bias;

    SetRotMatrix(&game_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&game_graphics_runtime.render_state.view_matrix);
    screen.vx = (u16)item->position_x - (u16)game_graphics_runtime.render_state.view_position.vx;
    screen.vy = (u16)item->position_y - (u16)game_graphics_runtime.render_state.view_position.vy;
    screen.vz = (u16)item->position_z - (u16)game_graphics_runtime.render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    facing = item->facing_and_frame_count & KF_FLOOR_ITEM_FACING_MASK;
    if ((u8)facing != KF_FLOOR_ITEM_FACING_BILLBOARD) {
        matrix_set_rotation_y(
            (facing - KF_FLOOR_ITEM_FACING_ZERO_YAW) << KF_FLOOR_ITEM_FACING_TO_ANGLE_SHIFT,
            &model);
        MulMatrix2(&game_graphics_runtime.render_state.view_matrix, &model);
        SetRotMatrix(&model);
        depth_bias = KF_FLOOR_ITEM_FIXED_FACING_DEPTH_BIAS;
    } else {
        SetRotMatrix(&game_graphics_runtime.render_state.pitch_matrix);
        depth_bias = KF_FLOOR_ITEM_BILLBOARD_DEPTH_BIAS;
    }
    SetTransMatrix(&model);
    render_enqueue_sprite(
        &floor_item_sprites[item->base_sprite_index + item->animation_frame],
        depth_bias, KF_SPRITE_DEPTH_CUE_BOOSTED);
    next_frame = item->animation_frame + 1;
    frame_count = item->facing_and_frame_count;
    item->animation_frame = next_frame;
    if ((next_frame & 0xff) >= (frame_count & KF_FLOOR_ITEM_FRAME_COUNT_MASK)) {
        item->animation_frame = 0;
    }
}

/*
 * Emits one pooled effect sprite. Empty slots (sprite_id 0xff) are skipped. The record is
 * carried into the view, oriented from its Euler angles and scaled in place.
 * A mode of 0xff draws a fixed billboard sprite through render_enqueue_sprite against
 * the render pitch matrix; otherwise the asset that follows the id by 30 is
 * bound, tested for visibility, and projected against the view matrix.
 */
ADDRESS(0x8001eedc, 0x1e8)
void render_actor_sprite(KfEffectRenderView *sprite)
{
    SVECTOR screen;
    VECTOR scale;
    MATRIX model;
    long flag;
    u16 asset;
    KfTmdObject *object;

    if (sprite->sprite_id == KF_EFFECT_RENDER_NONE) {
        return;
    }
    SetRotMatrix((MATRIX *)&game_graphics_runtime.render_state.view_matrix);
    SetTransMatrix((MATRIX *)&game_graphics_runtime.render_state.view_matrix);
    screen.vx = sprite->position_x - (u16)game_graphics_runtime.render_state.view_position.vx;
    screen.vy = sprite->position_y - (u16)game_graphics_runtime.render_state.view_position.vy;
    screen.vz = sprite->position_z - (u16)game_graphics_runtime.render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_yxz(&sprite->rotation, &model);
    scale.vx = sprite->scale_x;
    scale.vy = sprite->scale_y;
    scale.vz = sprite->scale_z;
    ScaleMatrix(&model, &scale);
    if (sprite->mode == KF_EFFECT_ANIMATION_BILLBOARD) {
        MulMatrix2((MATRIX *)&game_graphics_runtime.render_state.pitch_matrix, &model);
        SetRotMatrix(&model);
        SetTransMatrix(&model);
        render_enqueue_sprite(&effect_billboard_sprites[sprite->sprite_id], 0, KF_SPRITE_DEPTH_CUE_NORMAL);
    } else {
        MulMatrix2((MATRIX *)&game_graphics_runtime.render_state.view_matrix, &model);
        SetRotMatrix(&model);
        SetTransMatrix(&model);
        asset = sprite->sprite_id + KF_ASSET_EFFECT_FIRST;
        asset_registry_select(asset);
        object = tmd_get_object(0);
        if (render_bind_animated_instance(
                &sprite->animation_cache, asset, sprite->mode, sprite->asset_variant,
                object->vertex_count) == 0) {
            tmd_select_object_vertices(0);
            tmd_project_vertices(tmd_get_object(0)->vertex_count);
        } else {
            tmd_project_vertices(object->vertex_count);
        }
        render_enqueue_tmd(0, EFFECT_MODEL_DEPTH_BIAS);
    }
}
