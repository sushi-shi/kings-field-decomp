#include <kf/address.h>
#include <kf/semantic_types.h>
#include <kf/game.h>

/*
 * Per-entity billboard/model emitters invoked by the frame renderer's pool
 * sweep (render_entities).  Each transforms one live entry into view space, binds
 * the matching asset, and hands the result to the shared sprite/model draw
 * helpers.
 *
 * WIP: the pool record layouts and the sprite descriptor tables in the
 * 0x80055afc load-data blob are unresolved, so entity fields and the tables are
 * reached through byte views until their owners are reconstructed.
 *
 * Codegen residue (render_floor_item): retail zero-extends the masked facing nibble
 * with a redundant `andi 0xff` after `andi 0xf0` before its `beqz`/subtract; the
 * value provably fits a byte, so the rebuilt gcc-2.5.7 probe elides the second
 * mask (the inverse of the byte-field cases in docs/patterns/source-shapes-
 * gcc257.md).  The single extra instruction is the only divergence.
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

    SetRotMatrix((MATRIX *)&render_state.view_matrix);
    SetTransMatrix((MATRIX *)&render_state.view_matrix);
    screen.vx = (u16)item->position_x - (u16)render_state.view_position.vx;
    screen.vy = (u16)item->position_y - (u16)render_state.view_position.vy;
    screen.vz = (u16)item->position_z - (u16)render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    facing = item->facing_and_frame_count & 0xf0;
    if (facing != 0) {
        matrix_set_rotation_y((facing - 16) << 6, &model);
        MulMatrix2((MATRIX *)&render_state.view_matrix, &model);
        SetRotMatrix(&model);
        screen_scale = 0x96;
    } else {
        SetRotMatrix((MATRIX *)&render_state.pitch_matrix);
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
void render_actor_sprite(KfEffectRenderView *actor)
{
    SVECTOR screen;
    VECTOR scale;
    MATRIX model;
    long flag;
    u16 asset;
    KfTmdObject *object;

    if (actor->sprite_id == 0xff) {
        return;
    }
    SetRotMatrix((MATRIX *)&render_state.view_matrix);
    SetTransMatrix((MATRIX *)&render_state.view_matrix);
    screen.vx = actor->position_x - (u16)render_state.view_position.vx;
    screen.vy = actor->position_y - (u16)render_state.view_position.vy;
    screen.vz = actor->position_z - (u16)render_state.view_position.vz;
    RotTrans(&screen, (VECTOR *)&model.t, &flag);
    matrix_set_rotation_yxz(&actor->rotation, &model);
    scale.vx = actor->scale_x;
    scale.vy = actor->scale_y;
    scale.vz = actor->scale_z;
    ScaleMatrix(&model, &scale);
    if (actor->mode == 0xff) {
        MulMatrix2((MATRIX *)&render_state.pitch_matrix, &model);
        SetRotMatrix(&model);
        SetTransMatrix(&model);
        render_enqueue_sprite((KfSpriteQuad *)&DAT_80055afc[0x58 + actor->sprite_id * 12], 0, 0);
    } else {
        MulMatrix2((MATRIX *)&render_state.view_matrix, &model);
        SetRotMatrix(&model);
        SetTransMatrix(&model);
        asset = actor->sprite_id + 30;
        asset_registry_select(asset);
        object = tmd_get_object(0);
        if (render_bind_animated_instance(
                &actor->anchor, asset, actor->mode, actor->asset_variant,
                object->vertex_count) == 0) {
            tmd_select_object_vertices(0);
            tmd_project_vertices(tmd_get_object(0)->vertex_count);
        } else {
            tmd_project_vertices(object->vertex_count);
        }
        render_enqueue_tmd(0, 100);
    }
}
