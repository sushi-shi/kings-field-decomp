#include <kf/null.h>
#include <kf/game_graphics.h>

#include <kf/game_asset.h>
#include <kf/game_math.h>
#include <kf/game_render.h>
#include <kf/game_state.h>
#include <psyq/sdk.h>

enum { EFFECT_MODEL_DEPTH_BIAS = 100 };

KfSpriteQuad floor_item_sprites[KF_FLOOR_ITEM_SPRITE_COUNT] = {
    {0x90, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xd0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0xb0, 0x0, 0x20, 0x20, 0xfe00, 0xfc40, 0x400, 0x400},
    {0x90, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
    {0xb0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
    {0xd0, 0x20, 0x20, 0x27, 0xfe00, 0xfb40, 0x400, 0x500},
};

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

void render_floor_item(KfFloorItem *item)
{
    SVECTOR screen;
    MATRIX model;
    long flag;
    KfFloorItemFacing facing;
    s16 depth_bias;

    SetRotMatrix(&game_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&game_graphics_runtime.render_state.view_matrix);
    setVector(&screen,
        item->position_x - game_graphics_runtime.render_state.view_position.vx,
        item->position_y - game_graphics_runtime.render_state.view_position.vy,
        item->position_z - game_graphics_runtime.render_state.view_position.vz);
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    facing = floor_item_facing(item->facing_and_frame_count);
    if (kf_enum_encode<u8>(facing) != kf_enum_encode<u8>(KF_FLOOR_ITEM_FACING_BILLBOARD)) {
        matrix_set_rotation_y(
            (kf_enum_encode<u16>(facing) - kf_enum_encode<u8>(KF_FLOOR_ITEM_FACING_ZERO_YAW)) << KF_FLOOR_ITEM_FACING_TO_ANGLE_SHIFT,
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
        &floor_item_sprites[kf_enum_encode<u16>(item->base_sprite_index) + item->animation_frame],
        depth_bias, KF_SPRITE_DEPTH_CUE_BOOSTED);
    floor_item_advance_frame(item);
}

void render_actor_sprite(KfEffectRecord *sprite)
{
    SVECTOR screen;
    VECTOR scale;
    MATRIX model;
    long flag;
    u16 asset;
    KfTmdObject *object;

    if (sprite->render_id.model == KF_EFFECT_MODEL_NONE) {
        return;
    }
    SetRotMatrix(&game_graphics_runtime.render_state.view_matrix);
    SetTransMatrix(&game_graphics_runtime.render_state.view_matrix);
    setVector(&screen,
        sprite->position.vx - game_graphics_runtime.render_state.view_position.vx,
        sprite->position.vy - game_graphics_runtime.render_state.view_position.vy,
        sprite->position.vz - game_graphics_runtime.render_state.view_position.vz);
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_yxz(&sprite->rotation.angles, &model);
    setVector(&scale, (s16)sprite->scale_x, (s16)sprite->scale_y, (s16)sprite->scale_z);
    ScaleMatrix(&model, &scale);
    if (sprite->animation_clip == KF_ANIMATION_CLIP_NONE) {
        MulMatrix2(&game_graphics_runtime.render_state.pitch_matrix, &model);
        SetRotMatrix(&model);
        SetTransMatrix(&model);
        render_enqueue_sprite(&effect_billboard_sprites[kf_enum_encode<u8>(sprite->render_id.billboard)], 0, KF_SPRITE_DEPTH_CUE_NORMAL);
    } else {
        MulMatrix2(&game_graphics_runtime.render_state.view_matrix, &model);
        SetRotMatrix(&model);
        SetTransMatrix(&model);
        asset = kf_enum_encode<u8>(sprite->render_id.model) + KF_ASSET_EFFECT_FIRST;
        asset_registry_select(asset);
        object = tmd_get_object(0);
        if (render_bind_animated_instance(
                &sprite->animation_cache, asset, sprite->animation_clip, sprite->visual.animation_phase,
                object->vertex_count) == NULL) {
            tmd_select_object_vertices(0);
            tmd_project_vertices(tmd_get_object(0)->vertex_count);
        } else {
            tmd_project_vertices(object->vertex_count);
        }
        render_enqueue_tmd(0, EFFECT_MODEL_DEPTH_BIAS);
    }
}
