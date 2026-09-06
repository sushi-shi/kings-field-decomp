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
 *
 * WIP: the sprite descriptor tables in the 0x80055afc load-data blob retain
 * byte views until their enclosing owner is reconstructed.
 */

/*
 * Sprite-descriptor blob; ed90 indexes the 12-byte records four bytes in and
 * eedc's billboard path indexes them 0x58 bytes in.
 */

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
    u8 facing;
    u32 next_frame;
    u32 frame_count;
    s16 screen_scale;

    SetRotMatrix(&render_state.view_matrix);
    SetTransMatrix(&render_state.view_matrix);
    screen.vx = (u16)item->position_x - (u16)render_state.view_position.vx;
    screen.vy = (u16)item->position_y - (u16)render_state.view_position.vy;
    screen.vz = (u16)item->position_z - (u16)render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    facing = item->facing_and_frame_count & 0xf0;
    if (facing != 0) {
        matrix_set_rotation_y((facing - 16) << 6, &model);
        MulMatrix2(&render_state.view_matrix, &model);
        SetRotMatrix(&model);
        screen_scale = 0x96;
    } else {
        SetRotMatrix(&render_state.pitch_matrix);
        screen_scale = 0xc8;
    }
    SetTransMatrix(&model);
    render_enqueue_sprite(
        (KfSpriteQuad *)&DAT_80055afc[
            4 + (item->item_id + item->animation_frame) * 12],
        screen_scale, 1);
    next_frame = item->animation_frame + 1;
    frame_count = item->facing_and_frame_count;
    item->animation_frame = next_frame;
    if ((next_frame & 0xff) >= (frame_count & 0xf)) {
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

    if (sprite->sprite_id == 0xff) {
        return;
    }
    SetRotMatrix((MATRIX *)&render_state.view_matrix);
    SetTransMatrix((MATRIX *)&render_state.view_matrix);
    screen.vx = sprite->position_x - (u16)render_state.view_position.vx;
    screen.vy = sprite->position_y - (u16)render_state.view_position.vy;
    screen.vz = sprite->position_z - (u16)render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_yxz(&sprite->rotation, &model);
    scale.vx = sprite->scale_x;
    scale.vy = sprite->scale_y;
    scale.vz = sprite->scale_z;
    ScaleMatrix(&model, &scale);
    if (sprite->mode == 0xff) {
        MulMatrix2((MATRIX *)&render_state.pitch_matrix, &model);
        SetRotMatrix(&model);
        SetTransMatrix(&model);
        render_enqueue_sprite((KfSpriteQuad *)&DAT_80055afc[0x58 + sprite->sprite_id * 12], 0, 0);
    } else {
        MulMatrix2((MATRIX *)&render_state.view_matrix, &model);
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
        render_enqueue_tmd(0, 100);
    }
}
